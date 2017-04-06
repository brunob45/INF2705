// Prénoms, noms et matricule des membres de l'équipe:
// - Bruno BOUSQUET (1798492)
//#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint loctempsDeVieRestant = -1;
GLint locvitesse = -1;
GLint locColor = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint loclaTexture = -1;
GLint loctexnumero = -1;
GLuint progRetroaction;  // votre programme de nuanceurs pour la rétroaction
GLint locpositionRetroaction = -1;
GLint locvitesseRetroaction = -1;
GLint loctempsDeVieRestantRetroaction = -1;
GLint loccouleurRetroaction = -1;
GLint loctempsRetroaction = -1;
GLint locdtRetroaction = -1;
GLint locgraviteRetroaction = -1;
GLint loctempsVieMaxRetroaction = -1;
GLint locpositionPuitsRetroaction = -1;
GLint locbDimRetroaction = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[2];
GLuint tfo[1];

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

FormeSphere *demisphere = NULL;
FormeDisque *disque = NULL;

// variables pour définir le point de vue
double thetaCam = 0.0;         // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;           // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;          // distance (coord. sphériques)

// variables d'état
bool enPerspective = false;    // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;      // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;       // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

int texnumero = 1;             // numéro de la texture utilisée: 0-aucune, 1-étincelle, 2-oiseau, 3-leprechaun
GLuint textureETINCELLE = 0;
GLuint textureOISEAU = 0;
GLuint textureLEPRECHAUN = 0;

const unsigned int MAXNPARTICULES = 1000000;
struct Part
{
   GLfloat position[3];          // en unités
   GLfloat vitesse[3];           // en unités/seconde
   GLfloat couleur[4];           // couleur actuelle de la particule
   GLfloat tempsDeVieRestant;    // en secondes
   // (vous pouvez ajouter d'autres éléments, mais il faudra les prévoir dans les varyings)
};
Part part[MAXNPARTICULES];       // le tableau de particules

unsigned int nparticules = 300;  // nombre de particules utilisées (actuellement affichées)
float tempsVieMax = 5.0;         // temps de vie maximal (en secondes)
float temps = 0.0;               // le temps courant dans la simulation (en secondes)
const float dt = 1.0 / 60.0;     // intervalle entre chaque affichage (en secondes)
float gravite = 0.3;             // gravité utilisée dans le calcul de la position de la particule

glm::vec3 positionPuits = glm::vec3( 0.0, 0.0, 0.0 ); // position du puits de particules
glm::vec3 bDim = glm::vec3( 2.0, 1.5, 2.2 );  // les dimensions de la bulle en x,y,z

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

// s'assurer que le puits n'a pas été déplacé en dehors des limites de la demi-sphère
void verifierPositionPuits()
{
   const float deplLimite = 0.9; // on ne veut pas aller trop près de la paroi
   float dist = glm::length( glm::vec3( positionPuits.x/bDim.x, positionPuits.y/bDim.y, positionPuits.z/bDim.z ) );
   if ( dist >= deplLimite ) // on réassigne une nouvelle position
      positionPuits = deplLimite * glm::vec3( positionPuits.x/dist, positionPuits.y/dist, positionPuits.z/dist );
}

// Valeur aléatoire entre 0.0 et 1.0
float myrandom()
{
   return float(rand()) / RAND_MAX;
}

void calculerPhysique( )
{
   if ( enmouvement )
   {
      // À MODIFIER
      // déplacer en utilisant le nuanceur de rétroaction
      // ...
      glUseProgram( progRetroaction );
      
      // uniform vec3 bDim, positionPuits;
	  glUniform3fv( locbDimRetroaction, 1, glm::value_ptr(bDim) );// erreur => BRUNO
	  glUniform3fv( locpositionPuitsRetroaction, 1, glm::value_ptr(positionPuits) ); 
      
      // uniform float temps, dt, tempsVieMax, gravite;
	  glUniform1f( loctempsRetroaction, temps );
	  glUniform1f( locdtRetroaction, enmouvement ? dt : 0.0 );
	  glUniform1f( loctempsVieMaxRetroaction, tempsVieMax );
	  glUniform1f( locgraviteRetroaction, gravite );
	  	  
	  // faire les transformations de retour (retroaction)
	  glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1] );
	  
      glBindVertexArray( vao[1] );
      
      // se preparer
      glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
      glVertexAttribPointer( locpositionRetroaction, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,position) ) );
      glVertexAttribPointer( locvitesseRetroaction, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,vitesse) ) );
      glVertexAttribPointer( loccouleurRetroaction, 4, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,couleur) ) );
      glVertexAttribPointer( loctempsDeVieRestantRetroaction, 1, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,tempsDeVieRestant) ) );
      
      glEnableVertexAttribArray(locpositionRetroaction);
      glEnableVertexAttribArray(locvitesseRetroaction);
      glEnableVertexAttribArray(loccouleurRetroaction);
      glEnableVertexAttribArray(loctempsDeVieRestantRetroaction);
      
      glEnable( GL_RASTERIZER_DISCARD );
      glBeginTransformFeedback( GL_POINTS );
      glDrawArrays( GL_POINTS, 0, nparticules );
      glEndTransformFeedback();
      glDisable( GL_RASTERIZER_DISCARD );

	  glBindVertexArray( 0 );
   
      // échanger les deux VBO
      std::swap( vbo[0], vbo[1] );

      // avancer le temps
      temps += dt;

      FenetreTP::VerifierErreurGL("calculerPhysique");
   }
}

void chargerTextures()
{
   unsigned char *pixels;
   GLsizei largeur, hauteur;
   if ( ( pixels = ChargerImage( "textures/etincelle.bmp", largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &textureETINCELLE );
      glBindTexture( GL_TEXTURE_2D, textureETINCELLE );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }
   if ( ( pixels = ChargerImage( "textures/oiseau.bmp", largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &textureOISEAU );
      glBindTexture( GL_TEXTURE_2D, textureOISEAU );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }
   if ( ( pixels = ChargerImage( "textures/leprechaun.bmp", largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &textureLEPRECHAUN );
      glBindTexture( GL_TEXTURE_2D, textureLEPRECHAUN );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
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
      //if ( ( locVertexBase = glGetAttribLocation( progBase, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColorBase = glGetAttribLocation( progBase, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( locmatrModelBase = glGetUniformLocation( progBase, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisuBase = glGetUniformLocation( progBase, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProjBase = glGetUniformLocation( progBase, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
   }

   // charger le nuanceur de ce TP
   {
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
      const GLchar *chainesGeometrie = ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" );
      if ( chainesGeometrie != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesGeometrie, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesGeometrie;
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

      // faire l'édition des liens du programme
      glLinkProgram( prog );

      ProgNuanceur::afficherLogLink( prog );
      // demander la "Location" des variables
      if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locColor = glGetAttribLocation( prog, "Color" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Color" << std::endl;
      if ( ( loctempsDeVieRestant = glGetAttribLocation( prog, "tempsDeVieRestant" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsDeVieRestant" << std::endl;
      if ( ( locvitesse = glGetAttribLocation( prog, "vitesse" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de vitesse" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( loclaTexture = glGetUniformLocation( prog, "laTexture" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de laTexture" << std::endl;
      if ( ( loctexnumero = glGetUniformLocation( prog, "texnumero" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de texnumero" << std::endl;   }

   // charger le nuanceur de rétroaction
   {
      // créer le programme
      progRetroaction = glCreateProgram();

      // attacher le nuanceur de sommets
      const GLchar *chainesSommets = ProgNuanceur::lireNuanceur( "nuanceurRetroaction.glsl" );
      if ( chainesSommets != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 1, &chainesSommets, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( progRetroaction, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesSommets;
      }

      // À MODIFIER (partie 1)
      const GLchar* vars[] = { "positionMod", "vitesseMod", "couleurMod", "tempsDeVieRestantMod" };
      glTransformFeedbackVaryings( progRetroaction, sizeof(vars)/sizeof(vars[0]), vars, GL_INTERLEAVED_ATTRIBS );

      // faire l'édition des liens du programme
      glLinkProgram( progRetroaction );

      ProgNuanceur::afficherLogLink( progRetroaction );
      
      // demander la "Location" des variables
      // À MODIFIER (partie 1)
      if ( ( locpositionRetroaction = glGetAttribLocation( progRetroaction, "position" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de position" << std::endl;
      if ( ( locvitesseRetroaction = glGetAttribLocation( progRetroaction, "vitesse" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de vitesse" << std::endl;
      if ( ( loccouleurRetroaction = glGetAttribLocation( progRetroaction, "couleur" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de couleur" << std::endl;
      if ( ( loctempsDeVieRestantRetroaction = glGetAttribLocation( progRetroaction, "tempsDeVieRestant" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsDeVieRestant" << std::endl;
      if ( ( loctempsRetroaction = glGetUniformLocation( progRetroaction, "temps" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de temps" << std::endl;
      if ( ( locdtRetroaction = glGetUniformLocation( progRetroaction, "dt" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de dt" << std::endl;
      if ( ( locgraviteRetroaction = glGetUniformLocation( progRetroaction, "gravite" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de gravite" << std::endl;
      if ( ( loctempsVieMaxRetroaction = glGetUniformLocation( progRetroaction, "tempsVieMax" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de tempsVieMax" << std::endl;
      if ( ( locpositionPuitsRetroaction = glGetUniformLocation( progRetroaction, "positionPuits" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de positionPuits" << std::endl;
      if ( ( locbDimRetroaction = glGetUniformLocation( progRetroaction, "bDim" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de bDim" << std::endl;
   }

}

void initialiser()
{
   enmouvement = true;

   // donner la position de la caméra
   thetaCam = 270.0;
   phiCam = 80.0;
   distCam = 5.0;

   // donner la couleur de fond
   glClearColor( 0.3, 0.3, 0.3, 1.0 );

   // activer les états openGL
   glEnable( GL_DEPTH_TEST );
   glEnable( GL_PROGRAM_POINT_SIZE );
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   
   glEnable (GL_POINT_SPRITE);
   
   // Initialisation des particules
   for ( unsigned int i = 0 ; i < MAXNPARTICULES ; i++ )
      part[i].tempsDeVieRestant = 0.0; // la particule sera initialisée par le nuanceur de rétroaction

   // charger les textures
   chargerTextures();

   // charger les nuanceurs
   chargerNuanceurs();

   // Initialiser les formes pour les parois
   glUseProgram( progBase );
   demisphere = new FormeSphere( 1.0, 32, 32, true, false );
   disque = new FormeDisque( 0.0, 1.0, 32, 32 );

   // Initialiser les objets OpenGL
   glGenVertexArrays( 2, vao ); // générer deux VAOs
   glGenBuffers( 2, vbo );      // générer les VBOs
   glGenTransformFeedbacks( 1, tfo );
   
   // Initialiser le vao pour les particules
   // charger le VBO pour les valeurs modifiés
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, tfo[0] );
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(part), NULL, GL_STREAM_DRAW ); // on ne donne rien sinon la taille

   FenetreTP::VerifierErreurGL("1");
   glUseProgram( prog );
   // remplir les VBO et faire le lien avec les attributs du nuanceur de sommets
   glBindVertexArray( vao[0] );
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(part), part, GL_STREAM_DRAW );
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,position) ) );
   glEnableVertexAttribArray(locVertex);
   glVertexAttribPointer( locColor, 4, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,couleur) ) );
   glEnableVertexAttribArray(locColor);
   glVertexAttribPointer( loctempsDeVieRestant, 1, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,tempsDeVieRestant) ) );
   glEnableVertexAttribArray(loctempsDeVieRestant);
   glVertexAttribPointer( locvitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,vitesse) ) );
   glEnableVertexAttribArray(locvitesse);
   FenetreTP::VerifierErreurGL("2");
   glBindVertexArray( 0 );
   FenetreTP::VerifierErreurGL("3");

   // remplir les VBO pour les valeurs modifiées
   glBindVertexArray( vao[1] );
   glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(part), part, GL_STREAM_DRAW ); // déjà fait ci-dessus
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,position) ) );
   glEnableVertexAttribArray(locVertex);
   glVertexAttribPointer( locColor, 4, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,couleur) ) );
   glEnableVertexAttribArray(locColor);
   glVertexAttribPointer( loctempsDeVieRestant, 1, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,tempsDeVieRestant) ) );
   glEnableVertexAttribArray(loctempsDeVieRestant);
   // À MODIFIER (partie 3)
   glVertexAttribPointer( locvitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,vitesse) ) );
   glEnableVertexAttribArray(locvitesse);
   glBindVertexArray( 0 );

   // Défaire tous les liens
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   FenetreTP::VerifierErreurGL("fin de initialiser");
}

void conclure()
{
   glUseProgram( 0 );
   glDeleteVertexArrays( 2, vao );
   glDeleteBuffers( 2, vbo );
   delete demisphere;
   delete disque;
}

void FenetreTP::afficherScene()
{
   // effacer l'écran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   if ( enPerspective )
   {
      matrProj.Perspective( 55.0, (GLdouble) largeur_ / (GLdouble) hauteur_, 0.1, 20.0 );
   }
   else
   {
      GLfloat d = 0.5*distCam;
      if ( largeur_ < hauteur_ )
      {
         GLdouble fact = (GLdouble)hauteur_ / (GLdouble)largeur_;
         matrProj.Ortho( -d, d, -d*fact, d*fact, 0.1, 20.0 );
      }
      else
      {
         GLdouble fact = (GLdouble)largeur_ / (GLdouble)hauteur_;
         matrProj.Ortho( -d*fact, d*fact, -d, d, 0.1, 20.0 );
      }
   }
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   glm::vec3 ptVise = glm::vec3( 0.0, 0.0, 0.5*bDim.z ); // un point au milieu du modèle
   matrVisu.LookAt( ptVise[0] + distCam*cos(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    ptVise[1] + distCam*sin(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    ptVise[2] + distCam*cos(glm::radians(phiCam)),
                    ptVise[0], ptVise[1], ptVise[2],
                    0.0, 0.0, 1.0 );
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes( 0.2 );

   // afficher la boîte (demi-sphère)
   matrModel.PushMatrix();{
      matrModel.Scale( bDim.x, bDim.y, bDim.z );
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      // la base de la boîte
      glVertexAttrib3f( locColorBase, 0.8, 0.8, 1.0 );
      disque->afficher();
      // les faces arrières de la demi-sphère qui sert de boîte
      glEnable( GL_CULL_FACE );
      glCullFace( GL_FRONT ); // on enlève les faces avant pour ne garder que les faces arrières
      glVertexAttrib3f( locColorBase, 0.4, 0.4, 0.5 );
      demisphere->afficher();
      glDisable( GL_CULL_FACE );
   }matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );


   // afficher les particules
   glActiveTexture( GL_TEXTURE0 ); // activer la texture '0' (valeur de défaut)
   glUseProgram( prog );
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   glUniform1i( loclaTexture, 0 ); // '0' => utilisation de GL_TEXTURE0
   glUniform1i( loctexnumero, texnumero );

   glBindVertexArray( vao[0] );
   // refaire le lien avec les attributs du nuanceur de sommets pour le vbo actuellement utilisé
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,position) ) );
   glVertexAttribPointer( loctempsDeVieRestant, 1, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,tempsDeVieRestant) ) );
   // À MODIFIER (partie 3)
   //glVertexAttribPointer( locvitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,vitesse) ) );
   glVertexAttribPointer( locColor, 4, GL_FLOAT, GL_FALSE, sizeof(Part), reinterpret_cast<void*>( offsetof(Part,couleur) ) );

   switch ( texnumero ) // 0-aucune, 1-étincelle, 2-oiseau, 3-leprechaun
   {
   default: glBindTexture( GL_TEXTURE_2D, 0 ); break;
   case 1: glBindTexture( GL_TEXTURE_2D, textureETINCELLE ); break;
   case 2: glBindTexture( GL_TEXTURE_2D, textureOISEAU ); break;
   case 3: glBindTexture( GL_TEXTURE_2D, textureLEPRECHAUN ); break;
   }

   // tracer le résultat de la rétroaction
   //glDrawTransformFeedback( GL_POINTS, tfo[0] );
   glDrawArrays( GL_POINTS, 0, nparticules );

   glBindTexture( GL_TEXTURE_2D, 0 );
   glBindVertexArray( 0 );

   VerifierErreurGL("apres tracer le résultat de la rétroaction");

   VerifierErreurGL("fin de afficherScene");
}

void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   glViewport( 0, 0, w, h );
}

void FenetreTP::clavier( TP_touche touche )
{
   // quelques variables pour n'imprimer qu'une seule fois la liste des touches lorsqu'une touche est invalide
   bool toucheValide = true; // on suppose que la touche est connue
   static bool listerTouchesAFAIRE = true; // si la touche est invalide, on imprimera la liste des touches

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

   case TP_j: // Incrémenter le nombre de particules
   case TP_CROCHETDROIT:
      {
         unsigned int nparticulesPrec = nparticules;
         nparticules *= 1.2;
         if ( nparticules > MAXNPARTICULES ) nparticules = MAXNPARTICULES;
         std::cout << " nparticules=" << nparticules << std::endl;
         // on met les nouvelles particules au puits
         // (glBindBuffer n'est pas très efficace, mais on ne fait pas ça souvent)
         glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
         Part *ptr = (Part*) glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
         for ( unsigned int i = nparticulesPrec ; i < nparticules ; ++i )
            ptr[i].tempsDeVieRestant = 0.0; // la particule sera initialisée par le nuanceur de rétroaction
         glUnmapBuffer( GL_ARRAY_BUFFER );
      }
      break;
   case TP_u: // Décrémenter le nombre de particules
   case TP_CROCHETGAUCHE:
      nparticules /= 1.2;
      if ( nparticules < 5 ) nparticules = 5;
      std::cout << " nparticules=" << nparticules << std::endl;
      break;

   case TP_DROITE: // Augmenter la dimension de la boîte en X
      bDim.x += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_GAUCHE: // Diminuer la dimension de la boîte en X
      if ( bDim.x > 0.25 ) bDim.x -= 0.1;
      verifierPositionPuits();
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_BAS: // Augmenter la dimension de la boîte en Y
      bDim.y += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_HAUT: // Diminuer la dimension de la boîte en Y
      if ( bDim.y > 0.25 ) bDim.y -= 0.1;
      verifierPositionPuits();
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_PAGEPREC: // Augmenter la dimension de la boîte en Z
      bDim.z += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_PAGESUIV: // Diminuer la dimension de la boîte en Z
      if ( bDim.z > 0.25 ) bDim.z -= 0.1;
      verifierPositionPuits();
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;

   case TP_0: // Remettre le puits à la position (0,0,0)
      positionPuits = glm::vec3( 0.0, 0.0, 0.0 );
      break;

   case TP_PLUS: // Avancer la caméra
   case TP_EGAL:
      distCam -= 0.2;
      if ( distCam < 0.4 ) distCam = 0.4;
      std::cout << " distCam=" << distCam << std::endl;
      break;

   case TP_SOULIGNE:
   case TP_MOINS: // Reculer la caméra
      distCam += 0.2;
      if ( distCam > 20.0 - bDim.y ) distCam = 20.0 - bDim.y;
      std::cout << " distCam=" << distCam << std::endl;
      break;

   case TP_b: // Incrémenter la gravité
      gravite += 0.05;
      std::cout << " gravite=" << gravite << std::endl;
      break;
   case TP_h: // Décrémenter la gravité
      gravite -= 0.05;
      if ( gravite < 0.0 ) gravite = 0.0;
      std::cout << " gravite=" << gravite << std::endl;
      break;

   case TP_l: // Incrémenter la durée de vie maximale
      tempsVieMax += 0.2;
      std::cout << " tempsVieMax=" << tempsVieMax << std::endl;
      break;
   case TP_k: // Décrémenter la durée de vie maximale
      tempsVieMax -= 0.2;
      if ( tempsVieMax < 1.0 ) tempsVieMax = 1.0;
      std::cout << " tempsVieMax=" << tempsVieMax << std::endl;
      break;

   case TP_t: // Changer la texture utilisée: 0-aucune, 1-étincelle, 2-oiseau, 3-leprechaun
      if ( ++texnumero > 3 ) texnumero = 0;
      std::cout << " texnumero=" << texnumero << std::endl;
      break;

   case TP_p: // Permuter la projection: perspective ou orthogonale
      enPerspective = !enPerspective;
      std::cout << " enPerspective=" << enPerspective << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_ESPACE: // Mettre en pause ou reprendre l'animation
      enmouvement = !enmouvement;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      toucheValide = false;
      break;
   }

   // n'imprimer qu'une seule fois la liste des touches lorsqu'une touche est invalide
   if ( toucheValide ) // si la touche est valide, ...
   {
      listerTouchesAFAIRE = true; // ... on imprimera la liste des touches à la prochaine touche invalide
   }
   else if ( listerTouchesAFAIRE ) // s'il faut imprimer la liste des touches ...
   {
      listerTouchesAFAIRE = false; // ... se souvenir que ça a été fait
      imprimerTouches();
   }

}

int dernierX = 0; // la dernière valeur en X de position de la souris
int dernierY = 0; // la derniere valeur en Y de position de la souris
static enum { deplaceCam, deplacePuits } deplace = deplaceCam;
static bool pressed = false;
void FenetreTP::sourisClic( int button, int state, int x, int y )
{
   pressed = ( state == TP_PRESSE );
   if ( pressed )
   {
      // on vient de presser la souris
      dernierX = x;
      dernierY = y;
      switch ( button )
      {
      case TP_BOUTON_GAUCHE: // Manipuler la caméra
      case TP_BOUTON_MILIEU:
         deplace = deplaceCam;
         break;
      case TP_BOUTON_DROIT: // Déplacer le puits
         deplace = deplacePuits;
         break;
      }
   }
   else
   {
      // on vient de relâcher la souris
   }
}

void FenetreTP::sourisWheel( int x, int y ) // Changer la distance de la caméra
{
   const int sens = +1;
   distCam -= 0.2 * sens*y;
   if ( distCam < 0.4 ) distCam = 0.4;
   else if ( distCam > 20.0 - bDim.y ) distCam = 20.0 - bDim.y;
}

void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      int dx = x - dernierX;
      int dy = y - dernierY;
      switch ( deplace )
      {
      case deplaceCam:
         thetaCam -= dx / 3.0;
         phiCam   -= dy / 3.0;
         break;
      case deplacePuits:
         positionPuits[0] += 0.01 * dx;
         positionPuits[1] -= 0.01 * dy;
         verifierPositionPuits();
         break;
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
      fenetre.swap();

      // récupérer les événements et appeler la fonction de rappel
      boucler = fenetre.gererEvenement();
   }

   // détruire les ressources OpenGL allouées
   conclure();

   return 0;
}
