// Prénoms, noms et matricule des membres de l'équipe:
// - Prénom1 NOM1 (matricule1)
// - Prénom2 NOM2 (matricule2)
#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <iostream>
#include "inf2705.h"
#include "teapot_data.h"

// variables pour l'utilisation des nuanceurs
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeCube *cubeFil = NULL;
FormeSphere *sphere = NULL;

// variables pour définir le point de vue
double thetaCam = 0.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)

// variables d'état
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;     // le modèle est en mouvement automatique ou non
bool afficheAxes = true;      // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

// partie 1:
GLfloat anglePatte = 30.0;    // angle de rotation (en degrés) des pattes
GLfloat angleBestiole = 0.0;  // angle de rotation (en degrés) de la bestiole
GLfloat tailleCorps = 1.0;    // {largeur,hauteur,profondeur} du corps
const GLfloat longPatte = 0.7;      // longueur des pattes
const GLfloat largPatte = 0.2*longPatte; // largeur des pattes
glm::vec3 positionBestiole( 0.0, 0.0, 2.0 ); // position courante de la bestiole
double dimBoite = 10.0;       // la dimension de la boite

// partie 2:
GLuint vao[2] = {0,0};
GLuint vboCube = 0;
GLuint vboTheiere = 0;
GLuint vboConnec = 0;

bool modeLookAt = true;
int modele = 1;                  // le modèle à afficher

GLdouble thetaInit = 270., phiInit = 80., distInit = 20.;

// vérifier que les angles ne débordent pas les valeurs permises
void verifierAngles()
{
   if ( thetaCam > 360.0 )
      thetaCam -= 360.0;
   else if ( thetaCam < 0.0 )
      thetaCam += 360.0;

   const GLdouble MINPHI = 0.01, MAXPHI = 180.0 - 0.01;
   if ( phiCam > MAXPHI )
      phiCam = MAXPHI;
   else if ( phiCam < MINPHI )
      phiCam = MINPHI;

   if ( anglePatte > 90.0 )
      anglePatte = 90.0;
   else if ( anglePatte < 0.0 )
      anglePatte = 0.0;
}

void calculerPhysique( )
{
   if ( enmouvement )
   {
      static int sens[5] = { +1, +1, +1, +1, +1 };
      // mouvement en X
      if ( positionBestiole[0]-0.5*tailleCorps <= -0.5*dimBoite ) sens[0] = +1.0;
      else if ( positionBestiole[0]+0.5*tailleCorps >= 0.5*dimBoite ) sens[0] = -1.0;
      positionBestiole[0] += 0.03 * sens[0];
      // mouvement en Y
      if ( positionBestiole[1]-0.5*tailleCorps <= -0.5*dimBoite ) sens[1] = +1.0;
      else if ( positionBestiole[1]+0.5*tailleCorps >= 0.5*dimBoite ) sens[1] = -1.0;
      positionBestiole[1] += 0.02 * sens[1];
      // mouvement en Z
      if ( positionBestiole[2]-0.5*tailleCorps <= 0.0 ) sens[2] = +1.0;
      else if ( positionBestiole[2]+0.5*tailleCorps >= dimBoite ) sens[2] = -1.0;
      positionBestiole[2] += 0.05 * sens[2];

      // angle des pattes
      if ( anglePatte <= 0.0 ) sens[3] = +1.0;
      else if ( anglePatte >= 90.0 ) sens[3] = -1.0;
      anglePatte += 1.0 * sens[3];

      // taille du corps
      if ( tailleCorps <= 0.5 ) sens[4] = +1.0;
      else if ( tailleCorps >= 2.0 ) sens[4] = -1.0;
      tailleCorps += 0.01 * sens[4];

      // rotation du corps
      if ( angleBestiole > 360.0 ) angleBestiole -= 360.0;
      angleBestiole += 0.5;
   }
}

void chargerNuanceurs()
{
   // charger le nuanceur de base
   {
      // créer le programme
      progBase = glCreateProgram();

      // attacher le nuanceur de sommets
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesSommetsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }
      // attacher le nuanceur de fragments
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 1, &ProgNuanceur::chainesFragmentsMinimal, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progBase, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
      }

      // faire l'édition des liens du programme
      glLinkProgram( progBase );

      ProgNuanceur::afficherLogLink( progBase );
      // demander la "Location" des variables
      if ( ( locVertex = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColor = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }
}

void initialiser()
{
   // positionnement de la caméra: angle et distance de la caméra à la base du bras
   thetaCam = thetaInit;
   phiCam = phiInit;
   distCam = distInit;

   // donner la couleur de fond
   glClearColor( 0.0, 0.0, 0.0, 1.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   // charger les nuanceurs
   chargerNuanceurs();

   // allouer les objets OpenGL
   glGenVertexArrays( 2, vao );

   // initialiser le premier VAO (cube)
   glBindVertexArray( vao[0] );

   /*         +Y                    */
   /*   3+-----------+2             */
   /*    |\          |\             */
   /*    | \         | \            */
   /*    |  \        |  \           */
   /*    |  7+-----------+6         */
   /*    |   |       |   |          */
   /*    |   |       |   |          */
   /*   0+---|-------+1  |          */
   /*     \  |        \  |     +X   */
   /*      \ |         \ |          */
   /*       \|          \|          */
   /*       4+-----------+5         */
   /*             +Z                */

   // initialisation
#define p0 -.5, -.5, -.5
#define p1  .5, -.5, -.5
#define p2  .5,  .5, -.5
#define p3 -.5,  .5, -.5
#define p4 -.5, -.5,  .5
#define p5  .5, -.5,  .5
#define p6  .5,  .5,  .5
#define p7 -.5,  .5,  .5
   GLfloat sommetsCube[] = { p0, p4,  p1, p5,  p2, p6,  p3, p7,
                             p1, p2,  p0, p3,  p4, p7,  p5, p6 };
#undef p0
#undef p1
#undef p2
#undef p3
#undef p4
#undef p5
#undef p6
#undef p7

   glGenBuffers( 1, &vboCube );
   glBindBuffer( GL_ARRAY_BUFFER, vboCube );
   glBufferData( GL_ARRAY_BUFFER, sizeof(sommetsCube), sommetsCube, GL_STATIC_DRAW );
   // faire le lien avec l'attribut du nuanceur de sommets
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertex);

   glBindVertexArray(0);

   // initialiser le second VAO (théière)
   glBindVertexArray( vao[1] );

   // (partie 2) MODIFICATIONS ICI ...
   // créer le VBO pour les sommets
   // ...
   glGenBuffers( 1, &vboTheiere);
   glBindBuffer( GL_ARRAY_BUFFER, vboTheiere);
   glBufferData( GL_ARRAY_BUFFER, sizeof(gTeapotSommets), gTeapotSommets, GL_STATIC_DRAW);
   
   // faire le lien avec l'attribut du nuanceur de sommets
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(locVertex);
   
   // créer le VBO la connectivité
   glGenBuffers( 1, &vboConnec);
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboConnec);
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(gTeapotConnec), gTeapotConnec, GL_STATIC_DRAW);


   glBindVertexArray(0);

   // créer quelques autres formes
   glUseProgram( progBase );
   cubeFil = new FormeCube( 1.0, false );
   sphere = new FormeSphere( 0.25, 8, 8, false );
}

void conclure()
{
   glDeleteBuffers( 1, &vboCube );
   glDeleteBuffers( 1, &vboTheiere );
   glDeleteBuffers( 1, &vboConnec );
   delete cubeFil;
   delete sphere;
}

// (partie 1) Vous devez vous servir de cette fonction (sans la modifier) pour tracer tous les parallélépipèdes.
void afficherCube( )
{
   // affiche un cube d'arête 1
   glBindVertexArray( vao[0] );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 8 );
   glDrawArrays( GL_TRIANGLE_STRIP, 8, 8 );
   glBindVertexArray(0);
}
void afficherSphere( )
{
   // affiche une sphere de rayon 0.25
   sphere->afficher();
}

// (partie 2) Vous modifierez cette fonction pour utiliser les VBOs
void afficherTheiere()
{
   glBindVertexArray( vao[1] );
   // (partie 2) MODIFICATIONS ICI ...
   
   glDrawElements( GL_TRIANGLES, sizeof(gTeapotConnec)/sizeof(GLuint),GL_UNSIGNED_INT,0);
   glBindVertexArray(0);
}

void afficherBestiole()
{
   matrModel.PushMatrix();{ // sauvegarder la tranformation courante

      // (partie 1) MODIFICATIONS ICI ...
      
      // donner la couleur du corps
      glVertexAttrib3f( locColor, 0.0, 1.0, 0.0 ); // équivalent au glColor() de OpenGL 2.x

      // ajouter une ou des transformations afin de tracer le corps à la position courante "positionBestiole[]",
      // avec l'angle de rotation "angleBestiole" et de la taille "tailleCorps"
      matrModel.Translate( positionBestiole.x, positionBestiole.y, positionBestiole.z);
      matrModel.Rotate(angleBestiole, 0, 0, 1);
      matrModel.Scale(tailleCorps, tailleCorps, tailleCorps);

      // ...
      matrModel.PushMatrix();{
         // ...
         // ==> Avant de tracer, on doit informer la carte graphique des changements faits à la matrice de modélisation
         glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

         // afficher le corps à la position courante
         switch ( modele )
         {
         default:
         case 1: // une bestiole (très carrée et plutôt extraterrestre)
            afficherCube();
            matrModel.PushMatrix();{
               // tracer la tête à la bonne position
               matrModel.Translate(0.5, 0.0, 0.5 ); // (bidon) À MODIFIER
               matrModel.Scale(1/tailleCorps, 1/tailleCorps, 1/tailleCorps);
               glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
               // donner la couleur de la tête
               glVertexAttrib3f( locColor, 1.0, 0.0, 1.0 ); // équivalent au glColor() de OpenGL 2.x
               afficherSphere();
            }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            break;
         case 2: // une théière
            matrModel.PushMatrix();{
               matrModel.Scale( 0.25, 0.25, 0.25 );
               matrModel.Rotate( 90, 1, 0, 0 );
               matrModel.Translate( 0, -2, 0 );
               glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
               afficherTheiere();
            }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            break;
         }
      }matrModel.PopMatrix();

      // donner la couleur des pattes
      glVertexAttrib3f( locColor, 0.5, 0.5, 1.0 ); // équivalent au glColor() de OpenGL 2.x

      // ajouter une ou des transformations afin de tracer les pattes de largeur "largPatte" et longueur "longPatte"
      // ...
      
      matrModel.Translate(0, 0, -0.5);
      matrModel.PushMatrix();
      
      matrModel.Translate( 0.5, 0.5, 0); // (bidon) À MODIFIER
      matrModel.Rotate(315, 0,0,-1);
      matrModel.Rotate(anglePatte, 0, -1, 0);
      matrModel.Rotate(315, 0,0,1);
      matrModel.Scale(largPatte, largPatte, longPatte);
      matrModel.Translate(0, 0, -0.5);
      // ==> Avant de tracer, on doit informer la carte graphique des changements faits à la matrice de modélisation
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

      // afficher les quatre pattes
      // afficher la première patte
      afficherCube();
      
      matrModel.PopMatrix();
      matrModel.PushMatrix();
            
      // déplacer le repère, informer la carte graphique, tracer la seconde patte
      matrModel.Translate( -0.5, 0.5, 0); // (bidon) À MODIFIER
      matrModel.Rotate(225, 0,0,-1);
      matrModel.Rotate(anglePatte, 0, -1, 0);
      matrModel.Rotate(225, 0,0,1);
      matrModel.Scale(largPatte, largPatte, longPatte);
      matrModel.Translate(0, 0, -0.5);
      
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      afficherCube();
      
      
      matrModel.PopMatrix();
      matrModel.PushMatrix();
      
      matrModel.Translate( 0.5, -0.5, 0); // (bidon) À MODIFIER
      matrModel.Rotate(45, 0,0,-1);
      matrModel.Rotate(anglePatte, 0, -1, 0);
      matrModel.Rotate(45, 0,0,1);
      matrModel.Scale(largPatte, largPatte, longPatte);
      matrModel.Translate(0, 0, -0.5);
      
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      afficherCube();
      
      matrModel.PopMatrix();
      matrModel.PushMatrix();
      
      matrModel.Translate( -0.5, -0.5, 0); // (bidon) À MODIFIER
      matrModel.Rotate(135, 0,0,-1);
      matrModel.Rotate(anglePatte, 0, -1, 0);
      matrModel.Rotate(135, 0,0,1);
      matrModel.Scale(largPatte, largPatte, longPatte);
      matrModel.Translate(0, 0, -0.5);
      
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      afficherCube();
      // déplacer le repère, informer la carte graphique, tracer ...
      // etc.

   }matrModel.PopMatrix(); // revenir à la transformation sauvegardée
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel ); // informer ...
}

void definirCamera()
{
   if ( modeLookAt )
   {
      matrVisu.LookAt( distCam*cos(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                       distCam*sin(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                       distCam*cos(glm::radians(phiCam)),
                       0., 0., 5.,
                       0., 0., 6. );
   }
   else
   {
      matrVisu.LoadIdentity( );
      
      matrVisu.Translate(0,-1, -distCam);
      matrVisu.Rotate(-phiCam, 1,0,0);
      matrVisu.Rotate(-thetaCam-90, 0, 0, 1);
      matrVisu.Translate(0,0,-2);
   }
}

void FenetreTP::afficherScene()
{
   // effacer l'ecran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   matrProj.Perspective( 45.0, (GLdouble) largeur_ / (GLdouble) hauteur_, 0.1, 300.0 );
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj ); // informer la carte graphique des changements faits à la matrice

   definirCamera();
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu ); // informer la carte graphique des changements faits à la matrice

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel ); // informer la carte graphique des changements faits à la matrice

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes();

   // tracer la boite englobante
   glVertexAttrib3f( locColor, 1.0, 0.5, 0.5 ); // équivalent au glColor() de OpenGL 2.x
   matrModel.PushMatrix();{
      matrModel.Translate( 0, 0, 0.5*dimBoite );
      matrModel.Scale( dimBoite, dimBoite, dimBoite );
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      cubeFil->afficher();
   }matrModel.PopMatrix();

   // tracer la bestiole à pattes
   glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
   afficherBestiole();
}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
   switch ( touche )
   {
   case TP_ECHAP:
   case TP_q: // Quitter l'application
      quit();
      break;

   case TP_x: // Activer/désactiver l'affichage des axes
      afficheAxes = !afficheAxes;
      std::cout << "// Affichage des axes ? " << ( afficheAxes ? "OUI" : "NON" ) << std::endl;
      break;

   case TP_v: // Recharger les fichiers des nuanceurs et recréer le programme
      chargerNuanceurs();
      std::cout << "// Recharger nuanceurs" << std::endl;
      break;

   case TP_i: // Réinitiliaser le point de vue
      phiCam = phiInit; thetaCam = thetaInit; distCam = distInit;
      break;
   case TP_l: // Basculer l'utilisation de LookAt ou de Translate+Rotate pour placer la caméra
      modeLookAt = !modeLookAt;
      std::cout << " modeLookAt=" << modeLookAt << std::endl;
      break;
   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      break;

   case TP_m: // Choisir le modèle affiché: cube, théière
      if ( ++modele > 2 ) modele = 1;
      std::cout << " modele=" << modele << std::endl;
      break;

   case TP_SOULIGNE:
   case TP_MOINS: // Reculer la caméra
      distCam += 0.1;
      break;
   case TP_PLUS: // Avancer la caméra
   case TP_EGAL:
      if ( distCam > 1.0 )
         distCam -= 0.1;
      break;

   case TP_DROITE: // Déplacer la bestiole vers +X
      if ( positionBestiole[0]+0.5*tailleCorps < 0.5*dimBoite ) positionBestiole[0] += 0.1;
      break;
   case TP_GAUCHE: // Déplacer la bestiole vers -X
      if ( positionBestiole[0]-0.5*tailleCorps > -0.5*dimBoite ) positionBestiole[0] -= 0.1;
      break;
   case TP_PAGEPREC: // Déplacer la bestiole vers +Y
      if ( positionBestiole[1]+0.5*tailleCorps < 0.5*dimBoite ) positionBestiole[1] += 0.1;
      break;
   case TP_PAGESUIV: // Déplacer la bestiole vers -Y
      if ( positionBestiole[1]-0.5*tailleCorps > -0.5*dimBoite ) positionBestiole[1] -= 0.1;
      break;
   case TP_BAS: // Déplacer la bestiole vers +Z
      if ( positionBestiole[2]+0.5*tailleCorps < dimBoite ) positionBestiole[2] += 0.1;
      break;
   case TP_HAUT: // Déplacer la bestiole vers -Z
      if ( positionBestiole[2]-0.5*tailleCorps > 0.0 ) positionBestiole[2] -= 0.1;
      break;

   case TP_FIN: // Diminuer la taille du corps
      if ( tailleCorps > 0.5 ) tailleCorps -= 0.1;
      verifierAngles();
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;
   case TP_DEBUT: // Augmenter la taille du corps
      tailleCorps += 0.1;
      verifierAngles();
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;

   case TP_VIRGULE: // Tourner la bestiole dans le sens anti-horaire
      angleBestiole -= 2.0;
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;
   case TP_POINT: // Tourner la bestiole dans le sens horaire
      angleBestiole += 2.0;
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;

   case TP_CROCHETGAUCHE: // Diminuer l'angle des pattes
      anglePatte -= 2.0;
      verifierAngles();
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;
   case TP_CROCHETDROIT: // Augmenter l'angle des pattes
      anglePatte += 2.0;
      verifierAngles();
      std::cout << " tailleCorps=" << tailleCorps << " angleBestiole=" << angleBestiole << " anglePatte=" << anglePatte << std::endl;
      break;

   case TP_b: // Incrémenter la dimension de la boite
      dimBoite += 0.05;
      std::cout << " dimBoite=" << dimBoite << std::endl;
      break;
   case TP_h: // Décrémenter la dimension de la boite
      dimBoite -= 0.05;
      if ( dimBoite < 1.0 ) dimBoite = 1.0;
      std::cout << " dimBoite=" << dimBoite << std::endl;
      break;

   case TP_ESPACE: // Mettre en pause ou reprendre l'animation
      enmouvement = !enmouvement;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }
}

int dernierX, dernierY;
static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   // button est un parmi { TP_BOUTON_GAUCHE, TP_BOUTON_MILIEU, TP_BOUTON_DROIT }
   // state  est un parmi { TP_PRESSE, DL_RELEASED }
   pressed = ( state == TP_PRESSE );
   switch ( button )
   {
   case TP_BOUTON_GAUCHE: // Déplacer (modifier angles) la caméra
      dernierX = x;
      dernierY = y;
      break;
   }
}

void FenetreTP::sourisWheel( int x, int y )
{
   //const int sens = +1;
}

void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      int dx = x - dernierX;
      int dy = y - dernierY;
      thetaCam -= dx / 3.0;
      phiCam   -= dy / 3.0;

      dernierX = x;
      dernierY = y;

      verifierAngles();
   }
}

int main( int argc, char *argv[] )
{
   // créer une fenêtre
   FenetreTP fenetre( "INF2705 TP" );

   // allouer des ressources et définir le contexte OpenGL
   initialiser();

   bool boucler = true;
   while ( boucler )
   {
      // mettre à jour la physique
      calculerPhysique( );

      // affichage
      fenetre.afficherScene();
      fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   conclure();

   return 0;
}
