// Prénoms, noms et matricule des membres de l'équipe:
// - Prénom1 NOM1 (matricule1)
// - Prénom2 NOM2 (matricule2)
#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705.h"
#include <glm/gtx/io.hpp>

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locfacteurRechauffement = -1;
GLint locplanCoupe = -1;
GLint loccoulProfondeur = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeCube *cube = NULL;
FormeSphere *sphere = NULL;
FormeTheiere *theiere = NULL;
FormeTore *toreTerre = NULL;
FormeTore *toreMars = NULL;
FormeTore *toreJupiter = NULL;
GLuint vao = 0;
GLuint vbo[2] = {0,0};

// variables pour définir le point de vue
double thetaCam = 0.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)

// variables d'état
bool modeSelection = false;   // on est en mode sélection?
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;     // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;      // indique si on affiche les axes
bool coulProfondeur = false;  // indique si on veut colorer selon la profondeur
GLenum modePolygone = GL_FILL; // comment afficher les polygones
int dernierX = 0, dernierY = 0;

class CorpsCeleste
{
public:
   float rayon;          // le rayon
   float distance;       // la distance au soleil
   float rotation;       // l'angle actuel de rotation en degrés
   float revolution;     // l'angle actuel de révolution en degrés
   float incrRotation;   // l'incrément à ajouter à chaque appel de la fonction calculerPhysique en degrés
   float incrRevolution; // l'incrément à ajouter à chaque appel de la fonction calculerPhysique en degrés
   glm::vec3 couleur;    // la couleur
   bool estSelectionne = false;
   glm::vec3 couleurSelection;    // la couleur
   CorpsCeleste( float r, float dist, float rot, float rev, float incrRot, float incrRev, float coulr=1, float coulg=1, float coulb=1, float coulsr=1, float coulsg=1, float coulsb=1 ) :
      rayon(r), distance(dist),
      rotation(rot), revolution(rev),
      incrRotation(incrRot), incrRevolution(incrRev),
      couleur(coulr,coulg,coulb),
      couleurSelection(coulsr, coulsg, coulsb)
   { }
};

CorpsCeleste Soleil(   4.0,  0.0,  5.0,  0.0, 0.05, 0.0,   1.0, 1.0, 0.0, 1.00, 1.0, 1.0 );

CorpsCeleste Terre(    0.7,  8.0, 20.0, 45.0, 2.5,  0.03,  0.5, 0.5, 1.0, 0.95, 1.0, 1.0 );
CorpsCeleste Lune(     0.2,  1.0, 20.0, 30.0, 2.5, -0.35,  0.6, 0.6, 0.6, 0.90, 1.0, 1.0 );

CorpsCeleste Mars(     0.5, 12.0, 20.0, 45.0, 2.5,  0.03,  0.5, 1.0, 0.5, 0.85, 1.0, 1.0 );
CorpsCeleste Phobos(   0.2,  0.7,  5.0, 15.0, 3.5,  1.7,   0.4, 0.4, 0.8, 0.80, 1.0, 1.0 );
CorpsCeleste Deimos(   0.2,  1.4, 10.0,  2.0, 4.0,  0.5,   0.5, 0.5, 0.1, 0.75, 1.0, 1.0 );
																		  
CorpsCeleste Jupiter(  1.2, 16.0, 10.0, 30.0, 0.2,  0.02,  1.0, 0.5, 0.5, 0.70, 1.0, 1.0 );
CorpsCeleste Europa(   0.2,  1.5,  5.0, 15.0, 3.5,  3.4,   0.4, 0.4, 0.8, 0.65, 1.0, 1.0 );
CorpsCeleste Callisto( 0.3,  2.0, 10.0,  2.0, 4.0,  0.45,  0.5, 0.5, 0.1, 0.60, 1.0, 1.0 );

int modele = 1; // le modèle à afficher (1-sphère, 2-cube, 3-théière). On change avec ';'.

// partie 1: utiliser un plan de coupe
glm::vec4 planCoupe( 0, 0, 1, 0 ); // équation du plan de coupe
GLfloat angleCoupe = 0.0; // angle (degres) autour de x

// partie 2: utiliser un facteur de réchauffement
float facteurRechauffement = 0.2; // le facteur qui sert à calculer la couleur des pôles (0.0=froid, 1.0=chaud)

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
}

void calculerPhysique( )
{
   if ( enmouvement && !modeSelection)
   {
      // incrémenter rotation[] et revolution[] pour faire tourner les planètes
      if(!Soleil.estSelectionne) {
      Soleil.rotation     += Soleil.incrRotation;
      Soleil.revolution   += Soleil.incrRevolution; }
      if(!Terre.estSelectionne) {
      Terre.rotation      += Terre.incrRotation;
      Terre.revolution    += Terre.incrRevolution; }
      if(!Lune.estSelectionne) {
      Lune.rotation       += Lune.incrRotation;
      Lune.revolution     += Lune.incrRevolution; }
      if(!Mars.estSelectionne) {
      Mars.rotation       += Mars.incrRotation;
      Mars.revolution     += Mars.incrRevolution; }
      if(!Phobos.estSelectionne) {
      Phobos.rotation     += Phobos.incrRotation;
      Phobos.revolution   += Phobos.incrRevolution; }
      if(!Deimos.estSelectionne) {
      Deimos.rotation     += Deimos.incrRotation;
      Deimos.revolution   += Deimos.incrRevolution; }
      if(!Jupiter.estSelectionne) {
      Jupiter.rotation    += Jupiter.incrRotation;
      Jupiter.revolution  += Jupiter.incrRevolution; }
      if(!Europa.estSelectionne) {
      Europa.rotation     += Europa.incrRotation;
      Europa.revolution   += Europa.incrRevolution; }
      if(!Callisto.estSelectionne) {
      Callisto.rotation   += Callisto.incrRotation;
      Callisto.revolution += Callisto.incrRevolution; }
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
      if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }

   {
      // charger le nuanceur de ce TP

      // créer le programme
      prog = glCreateProgram();

      // attacher le nuanceur de sommets
      const GLchar *chainesSommets = ProgNuanceur::lireNuanceur( "nuanceurSommets.glsl" );
      if ( chainesSommets != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesSommets, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesSommets;
      }
      // attacher le nuanceur de fragments
      const GLchar *chainesFragments = ProgNuanceur::lireNuanceur( "nuanceurFragments.glsl" );
      if ( chainesFragments != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesFragments, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesFragments;
      }
      
      // attacher le nuanceur de geometrie
      const GLchar *chainesGeometrie = ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" );
      if ( chainesGeometrie != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesGeometrie, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesGeometrie;
      }/**/

      // faire l'édition des liens du programme
      glLinkProgram( prog );

      ProgNuanceur::afficherLogLink( prog );
      // demander la "Location" des variables
      if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColor = glGetAttribLocation( prog, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( locfacteurRechauffement = glGetUniformLocation( prog, "facteurRechauffement" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de facteurRechauffement" << std::endl;
      if ( ( locplanCoupe = glGetUniformLocation( prog, "planCoupe" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de planCoupe" << std::endl;
      if ( ( loccoulProfondeur = glGetUniformLocation( prog, "coulProfondeur" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de coulProfondeur" << std::endl;
   }
}

void initialiser()
{
   enmouvement = true;

   thetaCam = 60.0;
   phiCam = 75.0;
   distCam = 26.0;

   // donner la couleur de fond
   glClearColor( 0.1, 0.1, 0.1, 1.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   // charger les nuanceurs
   chargerNuanceurs();
   glUseProgram( prog );

   // les valeurs à utiliser pour tracer le quad
   const GLfloat taille = Jupiter.distance + Callisto.distance + Callisto.rayon;
   GLfloat coo[] = { -taille, -taille, 0,
                      taille, -taille, 0,
                     -taille,  taille, 0,
                      taille,  taille, 0 };
   // const GLuint connec[] = { ... };

   // initialiser le VAO (quad)
   // ...
   glBindVertexArray( vao );
   // créer les deux VBO pour les sommets et la connectivité
   // ...

   // partie 1: modifs ici ...
   // ...
   
   // faire le lien avec l'attribut du nuanceur de sommets
   glGenBuffers( 1, &vbo[0] );
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(coo), coo, GL_STATIC_DRAW );
   // faire le lien avec l'attribut du nuanceur de sommets
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertex);

   glBindVertexArray(0);
   
   
   glEnable(GL_CLIP_PLANE0);

   // créer quelques autres formes
   cube = new FormeCube( 3.0 );
   sphere = new FormeSphere( 2.0, 16, 16 );
   theiere = new FormeTheiere( );
   toreTerre = new FormeTore( 0.1, Terre.distance, 4, 64 );
   toreMars = new FormeTore( 0.1, Mars.distance, 4, 64 );
   toreJupiter = new FormeTore( 0.1, Jupiter.distance, 4, 64 );
}

void conclure()
{
   delete cube;
   delete sphere;
   delete theiere;
   delete toreTerre;
   delete toreMars;
   delete toreJupiter;
   glDeleteBuffers( 2, vbo );
   glDeleteVertexArrays( 1, &vao );
}

void definirCamera()
{
   // La souris influence le point de vue
   matrVisu.LookAt( distCam*cos(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    distCam*sin(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    distCam*cos(glm::radians(phiCam)),
                    0, 0, 0,
                    0, 0, 1 );

      // (pour apprentissage supplémentaire): modifs ici ...
      // (pour apprentissage supplémentaire): La caméra est sur la Terre et voir passer les autres objets célestes en utilisant l'inverse de la matrice mm
}

void afficherCorpsCeleste( GLfloat rayon )
{
   matrModel.PushMatrix(); {
      matrModel.Scale( 0.5*rayon, 0.5*rayon, 0.5*rayon );
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      switch ( modele )
      {
      default:
      case 1:
         sphere->afficher();
         break;
      case 2:
         cube->afficher();
         break;
      case 3:
         matrModel.Translate( 0.0, 0.0, -1.0 );
         glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
         theiere->afficher( );
         break;
      }
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
}

void afficherQuad( GLfloat alpha ) // le plan qui ferme les solides
{
   glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, alpha );
   // afficher le plan tourné selon l'angle courant et à la position courante
   // partie 1: modifs ici ...
   
   matrModel.PushMatrix(); {
	   
	  matrModel.Translate(0., 0., -planCoupe[3]);
	  matrModel.Rotate(angleCoupe, 0., 1., 0.);
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
	  
	  glEnable(GL_BLEND);
	  glDepthMask( GL_FALSE );
	  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	  
	  glBindVertexArray( vao );
	  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
	  glBindVertexArray(0);
	  
	  glDepthMask(GL_TRUE);
	  glDisable(GL_BLEND);
   
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
}

void afficherModele()
{
   glVertexAttrib4f( locColor, 1.0, 1.0, 1.0, 1.0 );

   // afficher le système solaire
   matrModel.PushMatrix( ); {

      matrModel.PushMatrix( ); { // Terre
         matrModel.Rotate( Terre.revolution, 0, 0, 1 ); // révolution terre autour soleil
         matrModel.Translate( Terre.distance, 0, 0 );
         matrModel.PushMatrix( ); {
            matrModel.Rotate( Terre.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Terre.couleurSelection : Terre.couleur) );
            afficherCorpsCeleste( Terre.rayon );
         } matrModel.PopMatrix();
         matrModel.PushMatrix( ); { // Lune
            matrModel.Rotate( Lune.revolution, 0, 0, 1 ); // révolution lune autour terre
            matrModel.Translate( Lune.distance, 0, 0 );
            matrModel.Rotate( Lune.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Lune.couleurSelection : Lune.couleur) );
            afficherCorpsCeleste( Lune.rayon );
         } matrModel.PopMatrix();
      } matrModel.PopMatrix();

      matrModel.PushMatrix( ); { // Mars
         matrModel.Rotate( Mars.revolution, 0, 0, 1 ); // révolution mars autour soleil
         matrModel.Translate( Mars.distance, 0, 0 );
         matrModel.PushMatrix( ); {
            matrModel.Rotate( Mars.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Mars.couleurSelection : Mars.couleur) );
            afficherCorpsCeleste( Mars.rayon );
         } matrModel.PopMatrix();
         matrModel.PushMatrix( ); { // Phobos
            matrModel.Rotate( Phobos.revolution, 0, 0, 1 ); // révolution phobos autour mars
            matrModel.Translate( Phobos.distance, 0, 0 );
            matrModel.Rotate( Phobos.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Phobos.couleurSelection : Phobos.couleur) );
            afficherCorpsCeleste( Phobos.rayon );
         } matrModel.PopMatrix();
         matrModel.PushMatrix( ); { // Deimos
            matrModel.Rotate( Deimos.revolution, 0, 0, 1 ); // révolution deimos autour mars
            matrModel.Translate( Deimos.distance, 0, 0 );
            matrModel.Rotate( Deimos.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Deimos.couleurSelection : Deimos.couleur) );
            afficherCorpsCeleste( Deimos.rayon );
         } matrModel.PopMatrix();
      } matrModel.PopMatrix();

      matrModel.PushMatrix( ); { // Jupiter
         matrModel.Rotate( Jupiter.revolution, 0, 0, 1 ); // révolution jupiter autour soleil
         matrModel.Translate( Jupiter.distance, 0, 0 );
         matrModel.PushMatrix( ); {
            matrModel.Rotate( Jupiter.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Jupiter.couleurSelection : Jupiter.couleur) );
            afficherCorpsCeleste( Jupiter.rayon );
         } matrModel.PopMatrix();
         matrModel.PushMatrix( ); { // Europa
            matrModel.Rotate( Europa.revolution, 0, 0, 1 ); // révolution europa autour jupiter
            matrModel.Translate( Europa.distance, 0, 0 );
            matrModel.Rotate( Europa.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Europa.couleurSelection : Europa.couleur) );
            afficherCorpsCeleste( Europa.rayon );
         } matrModel.PopMatrix();
         matrModel.PushMatrix( ); { // Callisto
            matrModel.Rotate( Callisto.revolution, 0, 0, 1 ); // révolution callisto autour jupiter
            matrModel.Translate( Callisto.distance, 0, 0 );
            matrModel.Rotate( Callisto.rotation, 0, 0, 1 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            glVertexAttrib3fv( locColor, glm::value_ptr(modeSelection ? Callisto.couleurSelection : Callisto.couleur) );
            afficherCorpsCeleste( Callisto.rayon );
         } matrModel.PopMatrix();
      } matrModel.PopMatrix();

   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );

   // afficher les deux tores pour identifier les orbites des planetes
   glVertexAttrib3f( locColor, 0.0, 0.0, 1.0 );
   toreTerre->afficher();
   glVertexAttrib3f( locColor, 0.0, 1.0, 0.0 );
   toreMars->afficher();
   glVertexAttrib3f( locColor, 1.0, 0.0, 0.0 );
   toreJupiter->afficher();

   // afficher le soleil, un peu transparent ... et sans réchauffement!
   // partie 1: modifs ici ...
   matrModel.PushMatrix( ); { // Soleil
	   
      matrModel.Rotate( Soleil.rotation, 0, 0, 1 );
      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      if(!modeSelection)
         glVertexAttrib4f( locColor, Soleil.couleur[0], Soleil.couleur[1], 
		    Soleil.couleur[2], 0.6);
	  else
         glVertexAttrib4f( locColor, Soleil.couleurSelection[0], 
            Soleil.couleurSelection[1], Soleil.couleurSelection[2], 0.6);
      
	  glEnable(GL_BLEND);
	  glDepthMask( GL_FALSE );
	  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	  
      afficherCorpsCeleste( Soleil.rayon );
      
	  glDepthMask(GL_TRUE);
	  glDisable(GL_BLEND);
      
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
}

void FenetreTP::afficherScene( )
{
   // effacer l'ecran et le tampon de profondeur et le stencil
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

   glUseProgram( progBase );
   glEnable(GL_DEPTH_TEST);
   // définir le pipeline graphique
   matrProj.Perspective( 70.0/1.5, (GLdouble) largeur_ / (GLdouble) hauteur_*2, 0.1, 100.0 );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   definirCamera( );
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes();

   // dessiner la scène
   glUseProgram( prog );
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   glUniform1f( locfacteurRechauffement, facteurRechauffement );
   glUniform4fv( locplanCoupe, 1, glm::value_ptr(planCoupe) );
   glUniform1i( loccoulProfondeur, coulProfondeur );

   // afficher le modèle et tenir compte du stencil et du plan de coupe
   // partie 1: modifs ici ...
   
   if(modeSelection)
      glUniform1f(locfacteurRechauffement, 1); 
   
   //afficherModele();
   glEnable(GL_STENCIL_TEST);
   glStencilFunc(GL_ALWAYS, 1, 1);
   glStencilOp(GL_INCR, GL_INCR, GL_INCR);
   
   glEnable(GL_CLIP_PLANE0);
   afficherModele();
   glDisable(GL_CLIP_PLANE0);
   
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
   
   glStencilFunc(GL_EQUAL, 1, 1);
   afficherQuad(1.);
   glDisable(GL_STENCIL_TEST);
   
   // en plus, dessiner le plan en transparence pour bien voir son étendue
   if(!modeSelection) 
      afficherQuad( 0.25 );
   else
   {
      //glFlush();
      glFinish();
      
      GLint cloture[4]; glGetIntegerv( GL_VIEWPORT, cloture );
	  GLint posX = dernierX, posY = cloture[3]-dernierY;

      glReadBuffer( GL_BACK );

      glm::vec3 color;
      glReadPixels(posX, posY,1,1, GL_RGB, GL_FLOAT, glm::value_ptr(color));
      
      //Soleil.couleur = color;
      
      Soleil.estSelectionne =    abs(color[0] - Soleil.couleurSelection[0]) < 0.01;
      Terre.estSelectionne =     abs(color[0] - Terre.couleurSelection[0]) < 0.01;
      Lune.estSelectionne =      abs(color[0] - Lune.couleurSelection[0]) < 0.01;
      Mars.estSelectionne =      abs(color[0] - Mars.couleurSelection[0]) < 0.01;
      Phobos.estSelectionne =    abs(color[0] - Phobos.couleurSelection[0]) < 0.01;
      Deimos.estSelectionne =    abs(color[0] - Deimos.couleurSelection[0]) < 0.01;
      Jupiter.estSelectionne =   abs(color[0] - Jupiter.couleurSelection[0]) < 0.01;
      Europa.estSelectionne =    abs(color[0] - Europa.couleurSelection[0]) < 0.01;
      Callisto.estSelectionne =  abs(color[0] - Callisto.couleurSelection[0]) < 0.01;
   }
}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
	GLfloat W = w, H2 = 0.5*h;
    GLfloat v[]  = {
      0, 0,  W, H2,
      0, H2, W, H2,
    };
   glViewportArrayv( 0, 2, v );

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

   case TP_ESPACE: // Mettre en pause ou reprendre l'animation
      enmouvement = !enmouvement;
      break;

   case TP_p: // Atténuer ou non la couleur selon la profondeur
      coulProfondeur = !coulProfondeur;
      std::cout << " coulProfondeur=" << coulProfondeur << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_PLUS: // Incrémenter la distance de la caméra
   case TP_EGAL:
      distCam--;
      std::cout << " distCam=" << distCam << std::endl;
      break;

   case TP_SOULIGNE:
   case TP_MOINS: // Décrémenter la distance de la caméra
      distCam++;
      std::cout << " distCam=" << distCam << std::endl;
      break;

   case TP_CROCHETDROIT: // Augmenter l'angle du plan de coupe
   case TP_SUPERIEUR:
      angleCoupe += 0.5;
      planCoupe[0] = sin(glm::radians(angleCoupe));
	  planCoupe[2] = cos(glm::radians(angleCoupe));
      std::cout << " angleCoupe=" << angleCoupe << std::endl;
      break;
   case TP_CROCHETGAUCHE: // Diminuer l'angle du plan de coupe
   case TP_INFERIEUR:
      angleCoupe -= 0.5;
      planCoupe[0] = sin(glm::radians(angleCoupe));
	  planCoupe[2] = cos(glm::radians(angleCoupe));
      std::cout << " angleCoupe=" << angleCoupe << std::endl;
      break;

   case TP_m: // Choisir le modèle: 1-sphère, 2-cube, 3-théière (déjà implanté)
      if ( ++modele > 3 ) modele = 1;
      std::cout << " modele=" << modele << std::endl;
      break;

   case TP_c: // Augmenter le facteur de réchauffement
      facteurRechauffement += 0.05; if ( facteurRechauffement > 1.0 ) facteurRechauffement = 1.0;
      std::cout << " facteurRechauffement=" << facteurRechauffement << " " << std::endl;
      break;
   case TP_f: // Diminuer le facteur de réchauffement
      facteurRechauffement -= 0.05; if ( facteurRechauffement < 0.0 ) facteurRechauffement = 0.0;
      std::cout << " facteurRechauffement=" << facteurRechauffement << " " << std::endl;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }
}

static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   pressed = ( state == TP_PRESSE );
   if ( pressed )
   {
      switch ( button )
      {
      default:
      case TP_BOUTON_GAUCHE: // Modifier le point de vue
         modeSelection = false;
         break;
      case TP_BOUTON_DROIT: // Sélectionner des objets
         modeSelection = true;
         break;
      }
      dernierX = x;
      dernierY = y;
   }
   else
   {
      modeSelection = false;
   }
}

void FenetreTP::sourisWheel( int x, int y ) // Déplacer le plan de coupe
{
   const int sens = +1;
   planCoupe[3] += 0.02 * sens * y;
   std::cout << " planCoupe[3]=" << planCoupe[3] << std::endl;
}

void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      if ( !modeSelection )
      {
         int dx = x - dernierX;
         int dy = y - dernierY;
         thetaCam -= dx / 3.0;
         phiCam   -= dy / 3.0;
      }

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
      if(!modeSelection)
          fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   conclure();

   return 0;
}
