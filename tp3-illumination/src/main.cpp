// Prénoms, noms et matricule des membres de l'équipe:
// - Bruno Bousquet (1798492)
// - VOID
//#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <stdlib.h>
#include <iostream>
#include "inf2705.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locNormal = -1;
GLint locTexCoord = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locmatrNormale = -1;
GLint loclaTexture = -1;
GLuint indLightSource;
GLuint indFrontMaterial;
GLuint indLightModel;
GLuint indvarsUnif;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[5];
GLuint ubo[4];

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeCube *cube = NULL;
FormeSphere *sphere = NULL, *sphereLumi = NULL;
FormeTheiere *theiere = NULL;
FormeTore *tore = NULL;
FormeCylindre *cylindre = NULL;
FormeCylindre *cone = NULL;

// variables pour définir le point de vue
double thetaCam = 0.0;        // angle de rotation de la caméra (coord. sphériques)
double phiCam = 0.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 0.0;         // distance (coord. sphériques)

// variables d'état
bool enPerspective = false;   // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;     // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;      // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

////////////////////////////////////////
// déclaration des variables globales //
////////////////////////////////////////

// partie 1: illumination
int modele = 1;                  // le modèle à afficher

// partie 3: texture
GLuint textureDE = 0;
GLuint textureECHIQUIER = 0;

// définition des lumières
struct LightSourceParameters
{
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   glm::vec4 position;
   glm::vec3 spotDirection;
   float spotExposant;
   float spotAngle;            // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1] = { { glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                       glm::vec4( 4, 1, 15, 1.0 ),
                       glm::vec3( -5.0, -2.0, -10.0 ),
                       1.0,       // l'exposant du cône
                       15.0,      // l'angle du cône du spot
                       1., 0., 0. } };

// définition du matériau
struct MaterialParameters
{
   glm::vec4 emission;
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   float shininess;
} FrontMaterial = { glm::vec4( 0.0, 0.0, 0.0, 1.0 ),
                    glm::vec4( 0.1, 0.1, 0.1, 1.0 ),
                    glm::vec4( 1.0, 0.1, 1.0, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    100.0 };

struct LightModelParameters
{
   glm::vec4 ambient; // couleur ambiante
   int localViewer;   // doit-on prendre en compte la position de l'observateur? (local ou à l'infini)
   int twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel = { glm::vec4(0,0,0,1), false, false };

struct
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   int utiliseBlinn;         // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   int utiliseDirect;        // indique si on utilise un spot style Direct3D ou OpenGL
   int afficheNormales;      // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   int utiliseCouleur;       // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelNoir;     // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
} varsUnif = { 2, false, false, false,
               0, true, 0 };
// ( En glsl, les types 'bool' et 'int' sont de la même taille, ce qui n'est pas le cas en C++.
// Ci-dessus, on triche donc un peu en déclarant les 'bool' comme des 'int', mais ça facilite la
// copie directe vers le nuanceur où les variables seront bien de type 'bool'. )


void verifierAngles()
{
   if ( thetaCam > 360.0 )
      thetaCam -= 360.0;
   else if ( thetaCam < 0.0 )
      thetaCam += 360.0;

   const GLdouble MINPHI = -90.0, MAXPHI = 90.0;
   if ( phiCam > MAXPHI )
      phiCam = MAXPHI;
   else if ( phiCam < MINPHI )
      phiCam = MINPHI;
}

void calculerPhysique( )
{
   if ( enmouvement )
   {
      static int sensTheta = 1;
      static int sensPhi = 1;
      thetaCam += 0.3 * sensTheta;
      phiCam += 0.5 * sensPhi;
      //if ( thetaCam <= 0. || thetaCam >= 360.0 ) sensTheta = -sensTheta;
      if ( phiCam < -90.0 || phiCam > 90.0 ) sensPhi = -sensPhi;

      static int sensAngle = 1;
      LightSource[0].spotAngle += sensAngle * 0.3;
      if ( LightSource[0].spotAngle < 5.0 ) sensAngle = -sensAngle;
      if ( LightSource[0].spotAngle > 60.0 ) sensAngle = -sensAngle;

#if 0
      static int sensExposant = 1;
      LightSource[0].spotExposant += sensExposant * 0.3;
      if ( LightSource[0].spotExposant < 1.0 ) sensExposant = -sensExposant;
      if ( LightSource[0].spotExposant > 10.0 ) sensExposant = -sensExposant;
#endif

      // De temps à autre, alterner entre le modèle d'illumination: Lambert, Gouraud, Phong
      static float type = 0;
      type += 0.005;
      varsUnif.typeIllumination = fmod(type,3);
   }

   verifierAngles();
}

void chargerTextures()
{
   unsigned char *pixels;
   GLsizei largeur, hauteur;
   if ( ( pixels = ChargerImage( "textures/de.bmp", largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &textureDE );
      glBindTexture( GL_TEXTURE_2D, textureDE );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }
   if ( ( pixels = ChargerImage( "textures/echiquier.bmp", largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &textureECHIQUIER );
      glBindTexture( GL_TEXTURE_2D, textureECHIQUIER );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }

   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
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
      if ( ( locNormal = glGetAttribLocation( prog, "Normal" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Normal (partie 1)" << std::endl;
      if ( ( locTexCoord = glGetAttribLocation( prog, "TexCoord" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TexCoord (partie 3)" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( locmatrNormale = glGetUniformLocation( prog, "matrNormale" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrNormale (partie 1)" << std::endl;
      if ( ( loclaTexture = glGetUniformLocation( prog, "laTexture" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de laTexture (partie 3)" << std::endl;
      if ( ( indLightSource = glGetUniformBlockIndex( prog, "LightSourceParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightSource" << std::endl;
      if ( ( indFrontMaterial = glGetUniformBlockIndex( prog, "MaterialParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de FrontMaterial" << std::endl;
      if ( ( indLightModel = glGetUniformBlockIndex( prog, "LightModelParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightModel" << std::endl;
      if ( ( indvarsUnif = glGetUniformBlockIndex( prog, "varsUnif" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de varsUnif" << std::endl;

      // charger les ubo
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(LightSource), &LightSource, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 0;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[0] );
         glUniformBlockBinding( prog, indLightSource, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(FrontMaterial), &FrontMaterial, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 1;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[1] );
         glUniformBlockBinding( prog, indFrontMaterial, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(LightModel), &LightModel, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 2;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[2] );
         glUniformBlockBinding( prog, indLightModel, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[3] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(varsUnif), &varsUnif, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 3;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[3] );
         glUniformBlockBinding( prog, indvarsUnif, bindingIndex );
      }
   }
}

// initialisation d'openGL
void initialiser()
{
   // donner l'orientation du modèle
   thetaCam = 0.0;
   phiCam = 0.0;
   distCam = 30.0;

   // couleur de l'arrière-plan
   glClearColor( 0.4, 0.2, 0.0, 1.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   // charger les textures
   chargerTextures();

   // allouer les UBO pour les variables uniformes
   glGenBuffers( 4, ubo );

   // charger les nuanceurs
   chargerNuanceurs();
   glUseProgram( prog );

   // (partie 1) créer le cube
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

   GLfloat sommets[3*4*6] =
   {
      -1.0,  1.0, -1.0,    1.0,  1.0, -1.0,  -1.0, -1.0, -1.0,    1.0, -1.0, -1.0,   // P3,P2,P0,P1
       1.0, -1.0,  1.0,   -1.0, -1.0,  1.0,   1.0, -1.0, -1.0,   -1.0, -1.0, -1.0,   // P5,P4,P1,P0
       1.0,  1.0,  1.0,    1.0, -1.0,  1.0,   1.0,  1.0, -1.0,    1.0, -1.0, -1.0,   // P6,P5,P2,P1
      -1.0,  1.0,  1.0,    1.0,  1.0,  1.0,  -1.0,  1.0, -1.0,    1.0,  1.0, -1.0,   // P7,P6,P3,P2
      -1.0, -1.0,  1.0,   -1.0,  1.0,  1.0,  -1.0, -1.0, -1.0,   -1.0,  1.0, -1.0,   // P4,P7,P0,P3
      -1.0, -1.0,  1.0,    1.0, -1.0,  1.0,  -1.0,  1.0,  1.0,    1.0,  1.0,  1.0    // P4,P5,P7,P6
   };
   GLfloat normales[3*4*6] = 
   {
	   0.0,  0.0, -1.0,    0.0,  0.0, -1.0,   0.0,  0.0, -1.0,    0.0,  0.0, -1.0,   // P3,P2,P0,P1
       0.0, -1.0,  0.0,    0.0, -1.0,  0.0,   0.0, -1.0,  0.0,    0.0, -1.0,  0.0,   // P5,P4,P1,P0
      +1.0,  0.0,  0.0,   +1.0,  0.0,  0.0,  +1.0,  0.0,  0.0,   +1.0,  0.0,  0.0,   // P6,P5,P2,P1
       0.0, +1.0,  0.0,    0.0, +1.0,  0.0,   0.0, +1.0,  0.0,    0.0, +1.0,  0.0,   // P7,P6,P3,P2
      -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0,  -1.0,  0.0,  0.0,   -1.0,  0.0,  0.0,   // P4,P7,P0,P3
       0.0,  0.0, +1.0,    0.0,  0.0, +1.0,   0.0,  0.0, +1.0,    0.0,  0.0, +1.0    // P4,P5,P7,P6
   };

   // allouer les objets OpenGL
   glGenVertexArrays( 2, vao );
   glGenBuffers( 5, vbo );
   // initialiser le VAO
   glBindVertexArray( vao[0] );

   // charger le VBO pour les sommets
   glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(sommets), sommets, GL_STATIC_DRAW );
   glVertexAttribPointer( locVertex, 3, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertex);
   
   // (partie 1) charger le VBO pour les normales
   glBindBuffer( GL_ARRAY_BUFFER, vbo[1] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(normales), normales, GL_STATIC_DRAW );
   glVertexAttribPointer( locNormal, 3, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locNormal);
   
   // (partie 3) charger le VBO pour les coordonnées de texture
   // ...

   glBindVertexArray(0);

   // initialiser le VAO pour une ligne (montrant la direction du spot)
   glBindVertexArray( vao[1] );
   GLfloat coords[] = { 0., 0., 0., 0., 0., 1. };
   glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
   glBufferData( GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW );
   glVertexAttribPointer( locVertexBase, 3, GL_FLOAT, GL_FALSE, 0, 0 );
   glEnableVertexAttribArray(locVertexBase);
   glBindVertexArray(0);

   // créer quelques autres formes
   cube = new FormeCube( 2.0 );
   sphere = new FormeSphere( 1.0, 32, 32 );
   sphereLumi = new FormeSphere( 0.5, 10, 10 );
   theiere = new FormeTheiere( );
   tore = new FormeTore( 0.4, 0.8, 32, 32 );
   cylindre = new FormeCylindre( 0.3, 0.3, 3.0, 32, 32 );
   cone = new FormeCylindre( 0.0, 0.5, 3.0, 32, 32 );
}

void conclure()
{
   glUseProgram( 0 );
   glDeleteVertexArrays( 2, vao );
   glDeleteBuffers( 4, vbo );
   glDeleteBuffers( 4, ubo );
   delete cube;
   delete sphere;
   delete sphereLumi;
   delete theiere;
   delete tore;
   delete cylindre;
   delete cone;
}

void afficherModele()
{
   // partie 3: paramètres de texture
   switch ( varsUnif.texnumero )
   {
   default:
      //std::cout << "Sans texture" << std::endl;
      break;
   case 1:
      //std::cout << "Texture DE" << std::endl;
      break;
   case 2:
      //std::cout << "Texture ECHIQUIER" << std::endl;
      break;
   }

   // Dessiner le modèle
   matrModel.PushMatrix(); {

      // tourner le modèle
      matrModel.Rotate( phiCam, -1.0, 0.0, 0.0 );
      matrModel.Rotate( thetaCam, 0.0, -1.0, 0.0 );

      // mise à l'échelle
      matrModel.Scale( 5.0, 5.0, 5.0 );

      glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
      // (partie 1: ne pas oublier de calculer et donner une matrice pour les transformations des normales)
	  glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, // on transpose
	  glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr()*matrModel.getMatr() ) ) ) );

      switch ( modele )
      {
      default:
      case 1:
         // afficher le cube
         glBindVertexArray( vao[0] );
         glDrawArrays( GL_TRIANGLE_STRIP,  0, 4 );
         glDrawArrays( GL_TRIANGLE_STRIP,  4, 4 );
         glDrawArrays( GL_TRIANGLE_STRIP,  8, 4 );
         glDrawArrays( GL_TRIANGLE_STRIP, 12, 4 );
         glDrawArrays( GL_TRIANGLE_STRIP, 16, 4 );
         glDrawArrays( GL_TRIANGLE_STRIP, 20, 4 );
         glBindVertexArray( 0 );
         break;
      case 2:
         tore->afficher(); // définit automatiquement des coordonnées de texture :)
         break;
      case 3:
         sphere->afficher(); // définit automatiquement des coordonnées de texture :)
         break;
      case 4:
         matrModel.Rotate( -90.0, 1.0, 0.0, 0.0 );
         matrModel.Translate( 0.0, 0.0, -0.5 );
         matrModel.Scale( 0.5, 0.5, 0.5 );
         glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
         theiere->afficher( );  // définit automatiquement des coordonnées de texture :)
         break;
      case 5:
         matrModel.Scale( 0.5, 0.5, 0.5 );
         glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
         cube->afficher(); // définit automatiquement des coordonnées de texture :)
         break;
      case 6:
         matrModel.PushMatrix(); {
            matrModel.Translate( 0.0, 0.0, -1.5 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            cylindre->afficher(); // définit automatiquement des coordonnées de texture :)
         } matrModel.PopMatrix();
         break;
      case 7:
         matrModel.PushMatrix(); {
            matrModel.Translate( 0.0, 0.0, -1.5 );
            glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
            cone->afficher(); // définit automatiquement des coordonnées de texture :)
         } matrModel.PopMatrix();
         break;
      }
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
}

void afficherLumiere()
{
   // Dessiner la lumiere

   // tracer une ligne vers la source lumineuse
   const GLfloat fact = 5.;
   GLfloat coords[] =
   {
      LightSource[0].position[0]                                     , LightSource[0].position[1]                                     , LightSource[0].position[2],
      LightSource[0].position[0]+LightSource[0].spotDirection[0]/fact, LightSource[0].position[1]+LightSource[0].spotDirection[1]/fact, LightSource[0].position[2]+LightSource[0].spotDirection[2]/fact
   };
   glLineWidth( 3.0 );
   glVertexAttrib3f( locColorBase, 1.0, 1.0, 0.5 ); // jaune
   glBindVertexArray( vao[1] );
   matrModel.PushMatrix(); {
      glBindBuffer( GL_ARRAY_BUFFER, vbo[4] );
      glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(coords), coords );
      glDrawArrays( GL_LINES, 0, 2 );
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   glBindVertexArray( 0 );
   glLineWidth( 1.0 );

   // tracer la source lumineuse
   matrModel.PushMatrix(); {
      matrModel.Translate( LightSource[0].position[0], LightSource[0].position[1], LightSource[0].position[2] );
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      sphereLumi->afficher();
   } matrModel.PopMatrix(); glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
}

// fonction d'affichage
void FenetreTP::afficherScene()
{
   // effacer l'ecran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   if ( enPerspective )
   {
      matrProj.Perspective( 35.0, (GLdouble)largeur_ / (GLdouble)hauteur_,
                            0.1, 60.0 );
   }
   else
   {
      const GLfloat d = 8.0;
      if ( largeur_ <= hauteur_ )
      {
         matrProj.Ortho( -d, d,
                         -d*(GLdouble)hauteur_ / (GLdouble)largeur_,
                         d*(GLdouble)hauteur_ / (GLdouble)largeur_,
                         0.1, 60.0 );
      }
      else
      {
         matrProj.Ortho( -d*(GLdouble)largeur_ / (GLdouble)hauteur_,
                         d*(GLdouble)largeur_ / (GLdouble)hauteur_,
                         -d, d,
                         0.1, 60.0 );
      }
   }
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   matrVisu.LookAt( 0.0, 0.0, distCam,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0 );
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes( 8.0 );

   // dessiner la scène
   afficherLumiere();

   glUseProgram( prog );

   // mettre à jour les blocs de variables uniformes
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[0] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &LightSource, sizeof(LightSource) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[1] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &FrontMaterial, sizeof(FrontMaterial) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &LightModel, sizeof(LightModel) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }
   {
      glBindBuffer( GL_UNIFORM_BUFFER, ubo[3] );
      GLvoid *p = glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );
      memcpy( p, &varsUnif, sizeof(varsUnif) );
      glUnmapBuffer( GL_UNIFORM_BUFFER );
   }

   // mettre à jour les matrices et autres uniformes
   glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   //glActiveTexture( GL_TEXTURE0 ); // activer la texture '0' (valeur de défaut)
   glUniform1i( loclaTexture, 0 ); // '0' => utilisation de GL_TEXTURE0

   afficherModele();
}

// fonction de redimensionnement de la fenêtre graphique
void FenetreTP::redimensionner( GLsizei w, GLsizei h )
{
   glViewport( 0, 0, w, h );
}

static void echoEtats( )
{
   static std::string illuminationStr[] = { "0:Lambert", "1:Gouraud", "2:Phong" };
   static std::string reflexionStr[] = { "0:Phong", "1:Blinn" };
   static std::string spotStr[] = { "0:OpenGL", "1:Direct3D" };
   std::cout << " modèle d'illumination= " << illuminationStr[varsUnif.typeIllumination]
             << ", refléxion spéculaire= " << reflexionStr[varsUnif.utiliseBlinn]
             << ", spot= " << spotStr[varsUnif.utiliseDirect]
             << std::endl;
}

// fonction de gestion du clavier
void FenetreTP::clavier( TP_touche touche )
{
   // traitement des touches q et echap
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

   case TP_p: // Permuter la projection: perspective ou orthogonale
      enPerspective = !enPerspective;
      break;

   case TP_i: // Alterner entre le modèle d'illumination: Lambert, Gouraud, Phong
      if ( ++varsUnif.typeIllumination > 2 ) varsUnif.typeIllumination = 0;
      echoEtats( );
      break;

   case TP_r: // Alterner entre le modèle de réflexion spéculaire: Phong, Blinn
      varsUnif.utiliseBlinn = !varsUnif.utiliseBlinn;
      echoEtats( );
      break;

   case TP_s: // Alterner entre le modèle de spot: OpenGL, Direct3D
      varsUnif.utiliseDirect = !varsUnif.utiliseDirect;
      echoEtats( );
      break;

   //case TP_l: // Alterner entre une caméra locale à la scène ou distante (localViewer)
   //   LightModel.localViewer = !LightModel.localViewer;
   //   std::cout << " localViewer=" << LightModel.localViewer << std::endl;
   //   break;

   case TP_a: // Incrémenter l'angle du cône du spot
   case TP_EGAL:
   case TP_PLUS:
      LightSource[0].spotAngle += 2.0;
      if ( LightSource[0].spotAngle > 90.0 ) LightSource[0].spotAngle = 90.0;
      std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
      break;
   case TP_z: // Décrémenter l'angle du cône du spot
   case TP_MOINS:
   case TP_SOULIGNE:
      LightSource[0].spotAngle -= 2.0;
      if ( LightSource[0].spotAngle < 0.0 ) LightSource[0].spotAngle = 0.0;
      std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
      break;

   case TP_d: // Incrémenter l'exposant du spot
   case TP_BARREOBLIQUE:
      LightSource[0].spotExposant += 0.3;
      if ( LightSource[0].spotExposant > 89.0 ) LightSource[0].spotExposant = 89.0;
      std::cout <<  " spotExposant=" << LightSource[0].spotExposant << std::endl;
      break;
   case TP_e: // Décrémenter l'exposant du spot
   case TP_POINT:
      LightSource[0].spotExposant -= 0.3;
      if ( LightSource[0].spotExposant < 0.0 ) LightSource[0].spotExposant = 0.0;
      std::cout <<  " spotExposant=" << LightSource[0].spotExposant << std::endl;
      break;

   case TP_j: // Incrémenter le coefficient de brillance
   case TP_CROCHETDROIT:
      FrontMaterial.shininess *= 1.1;
      std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
      break;
   case TP_u: // Décrémenter le coefficient de brillance
   case TP_CROCHETGAUCHE:
      FrontMaterial.shininess /= 1.1; if ( FrontMaterial.shininess < 0.0 ) FrontMaterial.shininess = 0.0;
      std::cout << " FrontMaterial.shininess=" << FrontMaterial.shininess << std::endl;
      break;

   case TP_DROITE:
      LightSource[0].position[0] += 0.3;
      break;
   case TP_GAUCHE:
      LightSource[0].position[0] -= 0.3;
      break;
   case TP_BAS:
      LightSource[0].position[1] += 0.3;
      break;
   case TP_HAUT:
      LightSource[0].position[1] -= 0.3;
      break;

   case TP_FIN:
      LightSource[0].spotDirection[0] += 0.6;
      break;
   case TP_DEBUT:
      LightSource[0].spotDirection[0] -= 0.6;
      break;
   case TP_PAGEPREC:
      LightSource[0].spotDirection[1] += 0.6;
      break;
   case TP_PAGESUIV:
      LightSource[0].spotDirection[1] -= 0.6;
      break;

   case TP_m: // Choisir le modèle affiché: cube, tore, sphère, théière, cube, cylindre, cône
      if ( ++modele > 7 ) modele = 1;
      std::cout << " modele=" << modele << std::endl;
      break;

   case TP_0:
      thetaCam = 0.0; phiCam = 0.0; distCam = 30.0; // placer les choses afin d'avoir une belle vue
      break;

   case TP_t: // Choisir la texture utilisée: aucune, dé, échiquier
      varsUnif.texnumero++;
      if ( varsUnif.texnumero > 2 ) varsUnif.texnumero = 0;
      std::cout << " varsUnif.texnumero=" << varsUnif.texnumero << std::endl;
      break;

   // case TP_c: // Changer l'affichage de l'objet texturé avec couleurs ou sans couleur
   //    varsUnif.utiliseCouleur = !varsUnif.utiliseCouleur;
   //    std::cout << " utiliseCouleur=" << varsUnif.utiliseCouleur << std::endl;
   //    break;

   case TP_o: // Changer l'affichage des texels noirs (noir, mi-coloré, transparent)
      varsUnif.afficheTexelNoir++;
      if ( varsUnif.afficheTexelNoir > 2 ) varsUnif.afficheTexelNoir = 0;
      std::cout << " afficheTexelNoir=" << varsUnif.afficheTexelNoir << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_n: // Utiliser ou non les normales calculées comme couleur (pour le débogage)
      varsUnif.afficheNormales = !varsUnif.afficheNormales;
      break;

   case TP_ESPACE: // Permuter la rotation automatique du modèle
      enmouvement = !enmouvement;
      break;

   default:
      std::cout << " touche inconnue : " << (char) touche << std::endl;
      imprimerTouches();
      break;
   }

}

// fonction callback pour un clic de souris
int dernierX = 0; // la dernière valeur en X de position de la souris
int dernierY = 0; // la derniere valeur en Y de position de la souris
static enum { deplaceCam, deplaceSpotDirection, deplaceSpotPosition } deplace = deplaceCam;
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
      case TP_BOUTON_GAUCHE: // Tourner l'objet
         deplace = deplaceCam;
         break;
      case TP_BOUTON_MILIEU: // Modifier l'orientation du spot
         deplace = deplaceSpotDirection;
         break;
      case TP_BOUTON_DROIT: // Déplacer la lumière
         deplace = deplaceSpotPosition;
         break;
      }
   }
   else
   {
      // on vient de relâcher la souris
   }
}

void FenetreTP::sourisWheel( int x, int y ) // Changer la taille du spot
{
   const int sens = +1;
   LightSource[0].spotAngle += sens*y;
   if ( LightSource[0].spotAngle > 90.0 ) LightSource[0].spotAngle = 90.0;
   if ( LightSource[0].spotAngle < 0.0 ) LightSource[0].spotAngle = 0.0;
   std::cout <<  " spotAngle=" << LightSource[0].spotAngle << std::endl;
}

// fonction de mouvement de la souris
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
      case deplaceSpotDirection:
         LightSource[0].spotDirection[0] += 0.06 * dx;
         LightSource[0].spotDirection[1] -= 0.06 * dy;
         // std::cout << " LightSource[0].spotDirection=" << glm::to_string(LightSource[0].spotDirection) << std::endl;
         break;
      case deplaceSpotPosition:
         LightSource[0].position[0] += 0.03 * dx;
         LightSource[0].position[1] -= 0.03 * dy;
         // std::cout << " LightSource[0].position=" << glm::to_string(LightSource[0].position) << std::endl;
         //glm::vec3 ecranPos( x, hauteur_-y, ecranLumi[2] );
         //LightSource[0].position = glm::vec4(glm::unProject( ecranPos, VM, P, cloture ), 1.0);
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
