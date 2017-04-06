// Prénoms, noms et matricule des membres de l'équipe:
// - Bruno BOUSQUET (1798492)
//#warning "Écrire les prénoms, noms et matricule des membres de l'équipe dans le fichier et commenter cette ligne"

#include <iostream>
#include "inf2705.h"

// variables pour l'utilisation des nuanceurs
GLuint prog;      // votre programme de nuanceurs
GLint locVertex = -1;
GLint locmatrModel = -1;
GLint locmatrVisu = -1;
GLint locmatrProj = -1;
GLint locmatrNormale = -1;
GLint loctextureDepl = -1;
GLint loctextureCoul = -1;
GLint locfacteurZ = -1;
GLint locbDim = -1;
GLint locTessLevelInner = -1;
GLint locTessLevelOuter = -1;
GLuint indLightSource = -1;
GLuint indFrontMaterial = -1;
GLuint indLightModel = -1;
GLuint progBase;  // le programme de nuanceurs de base
GLint locVertexBase = -1;
GLint locColorBase = -1;
GLint locmatrModelBase = -1;
GLint locmatrVisuBase = -1;
GLint locmatrProjBase = -1;

GLuint vao[2];
GLuint vbo[3];
GLuint ubo[3];

// matrices de du pipeline graphique
MatricePipeline matrModel;
MatricePipeline matrVisu;
MatricePipeline matrProj;

// les formes
FormeCube *cubeFil = NULL;
FormeSphere *sphereLumi = NULL;

// variables pour définir le point de vue
double thetaCam = -90.0;       // angle de rotation de la caméra (coord. sphériques)
double phiCam = 80.0;          // angle de rotation de la caméra (coord. sphériques)
double distCam = 10.0;         // distance (coord. sphériques)

// variables d'état
bool enPerspective = false;    // indique si on est en mode Perspective (true) ou Ortho (false)
bool enmouvement = false;      // le modèle est en mouvement/rotation automatique ou non
bool afficheAxes = true;       // indique si on affiche les axes
GLenum modePolygone = GL_FILL; // comment afficher les polygones

GLfloat TessLevelInner = 40;
GLfloat TessLevelOuter = 40;

// définition des lumières
struct LightSourceParameters
{
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   glm::vec4 position[2];
} LightSource = { glm::vec4( 0.2, 0.2, 0.2, 1.0 ),
                  glm::vec4( 0.4, 0.4, 0.4, 1.0 ),
                  glm::vec4( 0.6, 0.6, 0.6, 1.0 ),
                  { glm::vec4( -0.7, -1.0, 1.5, 1.0 ),
                    glm::vec4( 0.7, -1.0, 1.5, 1.0 ) } };

// définition du matériau
struct MaterialParameters
{
   glm::vec4 emission;
   glm::vec4 ambient;
   glm::vec4 diffuse;
   glm::vec4 specular;
   float shininess;
} FrontMaterial = { glm::vec4( 0.0, 0.0, 0.0, 1.0 ),
                    glm::vec4( 0.2, 0.2, 0.2, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    glm::vec4( 1.0, 1.0, 1.0, 1.0 ),
                    500.0 };

struct LightModelParameters
{
   glm::vec4 ambient; // couleur ambiante
   int localViewer;   // doit-on prendre en compte la position de l'observateur? (local ou à l'infini)
   int twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel = { glm::vec4(0,0,0,1), true, false };

int curLumi = 0;                 // la source lumineuse courante (celle qui peut être déplacée)
bool positionnelle = true;       // la lumière est de type positionnelle?
bool afficheNormales = false;    // indique si on utilise les normales comme couleurs (utile pour le débogage)

int affichageStereo = 0;         // type d'affichage: mono, stéréo anaglyphe, stéréo double

GLuint textures[9];              // les textures chargées
int indiceTexture = 0;           // indice de la texture à utiliser pour le déplacement
int indiceDiffuse = 0;           // indice de la texture à utiliser pour la couleur
int indiceFonction = 1;          // indice de la fonction à afficher
float facteurZ = 1.0;            // facteur de déplacement initial en Z
glm::vec4 bDim = glm::vec4( 2.0, 2.0, 2.0, 1.0 );  // les dimensions de la boite en x,y,z

struct VueStereo
{
   GLdouble dip;       // la distance interpupillaire
   GLdouble factzoom;  // le facteur de zoom
   GLdouble zavant;    // la position du plan avant du volume de visualisation
   GLdouble zarriere;  // la position du plan arrière du volume de visualisation
   GLdouble zecran;    // la position du plan de l'écran: les objets affichés en avant de ce plan «sortiront» de l'écran
   GLint modele;       // le modèle à afficher
};
struct VueStereo vue;

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
   if ( enmouvement )
   {
#if 1
      static int sensZ = +1;
      facteurZ += 0.01 * sensZ;
      if ( facteurZ < -1.0 ) sensZ = +1.0;
      else if ( facteurZ > 1.0 ) sensZ = -1.0;
#endif

#if 0
      static int sensPhi = 1;
      thetaCam += 0.4;
      phiCam += 0.7 * sensPhi;
      if ( phiCam < 0.0 || phiCam > 180.0 ) sensPhi = -sensPhi;
      verifierAngles();
#endif

#if 0
      static int sensrotZ = +1;
      deltaCam += 0.1 * sensrotZ;
      if ( deltaCam < -10.0 ) sensrotZ = +1.0;
      else if ( deltaCam > +10.0 ) sensrotZ = -1.0;
#endif
   }
}

void charger1Texture( std::string fichier, GLuint &texture )
{
   unsigned char *pixels;
   GLsizei largeur, hauteur;
   if ( ( pixels = ChargerImage( fichier, largeur, hauteur ) ) != NULL )
   {
      glGenTextures( 1, &texture );
      glBindTexture( GL_TEXTURE_2D, texture );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
      delete[] pixels;
   }
}
void chargerTextures()
{
   charger1Texture( "textures/texture1.bmp", textures[0] );
   charger1Texture( "textures/texture2.bmp", textures[1] );
   charger1Texture( "textures/texture3.bmp", textures[2] );
   charger1Texture( "textures/texture4.bmp", textures[3] );
   charger1Texture( "textures/texture5.bmp", textures[4] );
   charger1Texture( "textures/texture6.bmp", textures[5] );
   charger1Texture( "textures/texture7.bmp", textures[6] );
   charger1Texture( "textures/texture8.bmp", textures[7] );
   charger1Texture( "textures/texture9.bmp", textures[8] );
}

void chargerNuanceurs()
{
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
      std::ostringstream preambule; // ce préambule sera ajouté avant le contenu du fichier du nuanceur
      preambule << "#version 410" << std::endl
                << "#define INDICEFONCTION " << indiceFonction << std::endl
                << "#define INDICETEXTURE " << indiceTexture << std::endl
                << "#define INDICEDIFFUSE " << indiceDiffuse << std::endl
                << "#define AFFICHENORMALES " << afficheNormales << std::endl;
      std::string preambulestr = preambule.str();
      const char *preambulechar = preambulestr.c_str();

      // créer le programme
      prog = glCreateProgram();

      // attacher le nuanceur de sommets
      const GLchar *chainesSommets[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurSommets.glsl" ) };
      if ( chainesSommets[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_VERTEX_SHADER );
         glShaderSource( nuanceurObj, 2, chainesSommets, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesSommets[1];
      }
#if 1
      // À ACTIVER (enlever le #if 0 et le #endif)
      // attacher le nuanceur de controle de la tessellation
      const GLchar *chainesTessCtrl[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurTessCtrl.glsl" ) };
      if ( chainesTessCtrl[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_TESS_CONTROL_SHADER );
         glShaderSource( nuanceurObj, 2, chainesTessCtrl, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesTessCtrl[1];
      }
      // attacher le nuanceur d'évaluation de la tessellation
      const GLchar *chainesTessEval[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurTessEval.glsl" ) };
      if ( chainesTessEval[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_TESS_EVALUATION_SHADER );
         glShaderSource( nuanceurObj, 2, chainesTessEval, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesTessEval[1];
      }
#endif
      // attacher le nuanceur de géometrie
      const GLchar *chainesGeometrie[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurGeometrie.glsl" ) };
      if ( chainesGeometrie[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_GEOMETRY_SHADER );
         glShaderSource( nuanceurObj, 2, chainesGeometrie, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesGeometrie[1];
      }
      // attacher le nuanceur de fragments
      const GLchar *chainesFragments[2] = { preambulechar, ProgNuanceur::lireNuanceur( "nuanceurFragments.glsl" ) };
      if ( chainesFragments[1] != NULL )
      {
         GLuint nuanceurObj = glCreateShader( GL_FRAGMENT_SHADER );
         glShaderSource( nuanceurObj, 2, chainesFragments, NULL );
         glCompileShader( nuanceurObj );
         glAttachShader( prog, nuanceurObj );
         ProgNuanceur::afficherLogCompile( nuanceurObj );
         delete [] chainesFragments[1];
      }
      // faire l'édition des liens du programme
      glLinkProgram( prog );

      ProgNuanceur::afficherLogLink( prog );
      // demander la "Location" des variables
      if ( ( locVertex = glGetAttribLocation( prog, "Vertex" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de Vertex" << std::endl;
      if ( ( locmatrModel = glGetUniformLocation( prog, "matrModel" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrModel" << std::endl;
      if ( ( locmatrVisu = glGetUniformLocation( prog, "matrVisu" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrVisu" << std::endl;
      if ( ( locmatrProj = glGetUniformLocation( prog, "matrProj" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrProj" << std::endl;
      if ( ( locmatrNormale = glGetUniformLocation( prog, "matrNormale" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de matrNormale" << std::endl;
      // if ( ( loctextureDepl = glGetUniformLocation( prog, "textureDepl" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de textureDepl" << std::endl;
      // if ( ( loctextureCoul = glGetUniformLocation( prog, "textureCoul" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de textureCoul" << std::endl;
      loctextureDepl = glGetUniformLocation( prog, "textureDepl" );
      loctextureCoul = glGetUniformLocation( prog, "textureCoul" );
      if ( ( locfacteurZ = glGetUniformLocation( prog, "facteurZ" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de facteurZ" << std::endl;
      if ( ( locbDim = glGetUniformLocation( prog, "bDim" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de bDim" << std::endl;
      if ( ( locTessLevelInner = glGetUniformLocation( prog, "TessLevelInner" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelInner" << std::endl;
      if ( ( locTessLevelOuter = glGetUniformLocation( prog, "TessLevelOuter" ) ) == -1 ) std::cerr << "!!! pas trouvé la \"Location\" de TessLevelOuter" << std::endl;
      if ( ( indLightSource = glGetUniformBlockIndex( prog, "LightSourceParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightSource" << std::endl;
      if ( ( indFrontMaterial = glGetUniformBlockIndex( prog, "MaterialParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de FrontMaterial" << std::endl;
      if ( ( indLightModel = glGetUniformBlockIndex( prog, "LightModelParameters" ) ) == GL_INVALID_INDEX ) std::cerr << "!!! pas trouvé l'\"index\" de LightModel" << std::endl;

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
         const GLuint bindingIndex = 3;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[1] );
         glUniformBlockBinding( prog, indFrontMaterial, bindingIndex );
      }
      {
         glBindBuffer( GL_UNIFORM_BUFFER, ubo[2] );
         glBufferData( GL_UNIFORM_BUFFER, sizeof(LightModel), &LightModel, GL_DYNAMIC_COPY );
         glBindBuffer( GL_UNIFORM_BUFFER, 0 );
         const GLuint bindingIndex = 4;
         glBindBufferBase( GL_UNIFORM_BUFFER, bindingIndex, ubo[2] );
         glUniformBlockBinding( prog, indLightModel, bindingIndex );
      }
   }
}

void initialiser()
{
   // donner la couleur de fond
   glClearColor( 0.2, 0.2, 0.2, 1.0 );

   // activer les etats openGL
   glEnable( GL_DEPTH_TEST );

   vue.dip = 0.80;
   vue.factzoom = 1.0;
   vue.zavant = 4.0;
   vue.zarriere = 20.0;
   vue.zecran = 10.0;
   vue.modele = 1;

   // allouer les UBO pour les variables uniformes
   glGenBuffers( 3, ubo );

   // charger les nuanceurs
   chargerNuanceurs();

   // charger les textures
   chargerTextures();

   // créer le VAO pour conserver les informations
   glGenVertexArrays( 2, vao ); // les VAOs: un pour la surface, l'autre pour les décorations
   glGenBuffers( 3, vbo ); // les VBO pour différents tableaux

   // la surface
   glBindVertexArray( vao[0] );
   {
      // les coordonnées
      const GLfloat sommets[] = { -1.0,  1.0,
                                   1.0,  1.0,
                                   1.0, -1.0,
                                  -1.0, -1.0 };
      const GLuint connec[] = { 0, 1, 2, 2, 3, 0 };
      // créer le VBO pour les sommets
      glBindBuffer( GL_ARRAY_BUFFER, vbo[0] );
      glBufferData( GL_ARRAY_BUFFER, sizeof(sommets), sommets, GL_STATIC_DRAW );
      // faire le lien avec l'attribut du nuanceur de sommets
      glVertexAttribPointer( locVertex, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glEnableVertexAttribArray(locVertex);
      // charger le VBO pour la connectivité
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo[1] );
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(connec), connec, GL_STATIC_DRAW );
   }

   // les décorations
   glBindVertexArray( vao[1] );
   {
      // créer le VBO pour les sommets
      glBindBuffer( GL_ARRAY_BUFFER, vbo[2] );

      GLfloat coords[] = { 1., 0., 0., 0., 0., 0. };
      glBufferData( GL_ARRAY_BUFFER, sizeof(coords), coords, GL_STATIC_DRAW );

      // faire le lien avec l'attribut du nuanceur de sommets
      glVertexAttribPointer( locVertexBase, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
      glEnableVertexAttribArray(locVertexBase);
   }

   glBindVertexArray( 0 );

   // créer quelques autres formes
   glUseProgram( progBase );
   cubeFil = new FormeCube( 2.0, false );
   sphereLumi = new FormeSphere( 0.1, 10, 10 );
}

void conclure()
{
   glDeleteBuffers( 3, vbo );
   glDeleteVertexArrays( 2, vao );
   glDeleteBuffers( 3, ubo );
   delete cubeFil;
   delete sphereLumi;
}

void definirProjection( int OeilMult, int w, int h ) // 0: mono, -1: oeil gauche, +1: oeil droit
{
   // partie 2: utiliser plutôt Frustum() pour le stéréo
   matrProj.Perspective( 35.0, (GLdouble) w / (GLdouble) h, vue.zavant, vue.zarriere );
}

void afficherDecoration()
{
   // remettre le programme de base pour le reste des décorations
   glUseProgram( progBase );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj ); // donner la projection courante

   // dessiner le cube englobant
   glVertexAttrib3f( locColorBase, 1.0, 1.0, 1.0 ); // blanc
   matrModel.PushMatrix();{
      matrModel.Scale( bDim.x, bDim.y, bDim.z );
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      cubeFil->afficher();
   }matrModel.PopMatrix();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );

   for ( int i = 0 ; i < 2 ; ++i )
   {
      // dessiner une ligne vers la source lumineuse
      glVertexAttrib3f( locColorBase, 1.0, 1.0, 0.5 ); // jaune
      if ( !positionnelle )
      {
         matrModel.PushMatrix();{
            glm::mat4 a;
            a[0] = glm::vec4( 2*LightSource.position[i][0],  2*LightSource.position[i][1],  2*LightSource.position[i][2], 0.0 );
            a[1] = glm::vec4( 2*LightSource.position[i][0], -2*LightSource.position[i][1],  2*LightSource.position[i][2], 0.0 );
            a[2] = glm::vec4( 2*LightSource.position[i][0],  2*LightSource.position[i][1], -2*LightSource.position[i][2], 0.0 );
            matrModel.setMatr( matrModel.getMatr() * a );
            glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
            glBindVertexArray( vao[1] );
            glDrawArrays( GL_LINES, 0, 2 );
            glBindVertexArray( 0 );
         }matrModel.PopMatrix();
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
      }

      // dessiner une sphère à la position de la lumière
      matrModel.PushMatrix();{
         matrModel.Translate( LightSource.position[i][0], LightSource.position[i][1], LightSource.position[i][2] );
         glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
         sphereLumi->afficher();
      }matrModel.PopMatrix();
      glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   }
}

void afficherModele()
{
   // afficher d'abord les décorations (en utilisant progBase)
   afficherDecoration();

   // afficher le modèle (en utilisant prog)
   glUseProgram( prog );

   // s'il y a lieu, assigner les textures aux unités de texture
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, indiceTexture ? textures[indiceTexture-1] : 0 );
   glActiveTexture( GL_TEXTURE1 );
   glBindTexture( GL_TEXTURE_2D, indiceDiffuse ? textures[indiceDiffuse-1] : 0 );

   // partie 1: activer les deux glClipPane limitant le z vers le haut et vers le bas
   glEnable( GL_CLIP_PLANE0 );
   glEnable( GL_CLIP_PLANE1 );

   // afficher la surface (plane)
   glBindVertexArray( vao[0] );
   glPatchParameteri( GL_PATCH_VERTICES, 4 );

   // À MODIFIER (utiliser des GL_PATCHES)
   //glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
   glDrawArrays( GL_PATCHES, 0, 4 ); // UTILISER des GL_PATCHES plutôt que des GL_TRIANGLES

   glBindVertexArray( 0 );

   glDisable( GL_CLIP_PLANE0 );
   glDisable( GL_CLIP_PLANE1 );
}

void FenetreTP::afficherScene()
{
   // effacer l'écran et le tampon de profondeur
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glUseProgram( progBase );

   // définir le pipeline graphique
   definirProjection( 0, largeur_, hauteur_ );
   glUniformMatrix4fv( locmatrProjBase, 1, GL_FALSE, matrProj );

   matrVisu.LookAt( distCam*cos(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    distCam*sin(glm::radians(thetaCam))*sin(glm::radians(phiCam)),
                    distCam*cos(glm::radians(phiCam)),
                    0, 0, 0,
                    0, 0, 1 );
   glUniformMatrix4fv( locmatrVisuBase, 1, GL_FALSE, matrVisu );

   matrModel.LoadIdentity();
   glUniformMatrix4fv( locmatrModelBase, 1, GL_FALSE, matrModel );
   glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

   // afficher les axes
   if ( afficheAxes ) FenetreTP::afficherAxes( );

   glUseProgram( prog );
   glUniform1i( loctextureDepl, 0 );    // '0' => utilisation de GL_TEXTURE0
   glUniform1i( loctextureCoul, 1 );    // '1' => utilisation de GL_TEXTURE1
   glUniform1f( locfacteurZ, facteurZ );
   glUniform4fv( locbDim, 1, glm::value_ptr(bDim) );
   glUniform1f( locTessLevelInner, TessLevelInner );
   glUniform1f( locTessLevelOuter, TessLevelOuter );

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

   //glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj ); // inutile car on modifie ensuite la projection
   glUniformMatrix4fv( locmatrVisu, 1, GL_FALSE, matrVisu );
   glUniformMatrix4fv( locmatrModel, 1, GL_FALSE, matrModel );
   // (partie 1: ne pas oublier de calculer et donner une matrice pour les transformations des normales)
   glUniformMatrix3fv( locmatrNormale, 1, GL_TRUE, glm::value_ptr( glm::inverse( glm::mat3( matrVisu.getMatr() * matrModel.getMatr() ) ) ) );

   // partie 2: afficher la surface en mono ou en stéréo

   switch ( affichageStereo )
   {
   case 0: // mono
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;

   case 1: // stéréo anaglyphe
      // partie 2: à modifier pour afficher en anaglyphe
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;

   case 2: // stéréo double
      // partie 2: à modifier pour afficher en stéréo double
      definirProjection( 0, largeur_, hauteur_ );
      glUniformMatrix4fv( locmatrProj, 1, GL_FALSE, matrProj );
      afficherModele();
      break;
   }
}

// fonction de redimensionnement de la fenêtre graphique
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

   case TP_i: // Augmenter le niveau de tessellation interne
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, TessLevelInner );
      break;
   case TP_k: // Diminuer le niveau de tessellation interne
      if ( --TessLevelInner < 1 ) TessLevelInner = 1;
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, TessLevelInner );
      break;

   case TP_o: // Augmenter le niveau de tessellation externe
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, TessLevelOuter );
      break;
   case TP_l: // Diminuer le niveau de tessellation externe
      if ( --TessLevelOuter < 1 ) TessLevelOuter = 1;
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, TessLevelOuter );
      break;

   case TP_u: // Augmenter les deux niveaux de tessellation
      TessLevelInner = TessLevelOuter;
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, TessLevelOuter );
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, TessLevelInner );
      break;
   case TP_j: // Diminuer les deux niveaux de tessellation
      if ( --TessLevelOuter < 1 ) TessLevelOuter = 1;
      TessLevelInner = TessLevelOuter;
      std::cout << " TessLevelInner=" << TessLevelInner << " TessLevelOuter=" << TessLevelOuter << std::endl;
      glPatchParameteri( GL_PATCH_DEFAULT_OUTER_LEVEL, TessLevelOuter );
      glPatchParameteri( GL_PATCH_DEFAULT_INNER_LEVEL, TessLevelInner );
      break;

   case TP_MOINS: // Moduler l'effet du déplacement
      facteurZ -= 0.01;
      std::cout << " facteurZ=" << facteurZ << std::endl;
      break;

   case TP_PLUS: // Moduler l'effet du déplacement
   case TP_EGAL:
      facteurZ += 0.01;
      std::cout << " facteurZ=" << facteurZ << std::endl;
      break;

   case TP_DROITE: // Augmenter la dimension de la boîte en X
      bDim.x += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_GAUCHE: // Diminuer la dimension de la boîte en X
      if ( bDim.x > 0.25 ) bDim.x -= 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_BAS: // Augmenter la dimension de la boîte en Y
      bDim.y += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_HAUT: // Diminuer la dimension de la boîte en Y
      if ( bDim.y > 0.25 ) bDim.y -= 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_PAGEPREC: // Augmenter la dimension de la boîte en Z
      bDim.z += 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;
   case TP_PAGESUIV: // Diminuer la dimension de la boîte en Z
      if ( bDim.z > 0.25 ) bDim.z -= 0.1;
      std::cout << " bDim= " << bDim.x << " x " << bDim.y << " x " << bDim.z << std::endl;
      break;

   case TP_0: // Revenir à la surface de base (ne pas utiliser de textures)
      LightSource.position[0].xyz() = glm::vec3( -0.7, -1.0, 1.5 );
      LightSource.position[1].xyz() = glm::vec3(  0.7, -1.0, 1.5 );
      indiceFonction = indiceTexture = indiceDiffuse = 0;
      std::cout << " indiceFonction=" << indiceFonction << " indiceTexture=" << indiceTexture << " indiceDiffuse=" << indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour le #define
      break;

   case TP_f: // Varier la fonction mathématique utilisée pour le déplacement
      if ( ++indiceFonction > 6 ) indiceFonction = 0;
      indiceTexture = 0;
      std::cout << " indiceFonction=" << indiceFonction << " indiceTexture=" << indiceTexture << " indiceDiffuse=" << indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour le #define
      break;

   case TP_t: // Varier l'indice de la texture utilisée pour le déplacement
      if ( ++indiceTexture > 9 ) indiceTexture = 0;
      indiceFonction = 0;
      std::cout << " indiceFonction=" << indiceFonction << " indiceTexture=" << indiceTexture << " indiceDiffuse=" << indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour le #define
      break;

   case TP_c: // Varier l'indice de la texture utilisée pour la composante diffuse de la couleur
      if ( ++indiceDiffuse > 9 ) indiceDiffuse = 0;
      std::cout << " indiceFonction=" << indiceFonction << " indiceTexture=" << indiceTexture << " indiceDiffuse=" << indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour le #define
      break;

   case TP_e: // Varier l'indice de la texture utilisée pour la couleur ET le déplacement
      if ( ++indiceTexture > 9 ) indiceTexture = 0;
      indiceDiffuse = indiceTexture;
      indiceFonction = 0;
      std::cout << " indiceFonction=" << indiceFonction << " indiceTexture=" << indiceTexture << " indiceDiffuse=" << indiceDiffuse << std::endl;
      chargerNuanceurs(); // pour le #define
      break;

   case TP_p: // Permuter lumière positionnelle ou directionnelle
      positionnelle = !positionnelle;
      LightSource.position[0].w = LightSource.position[1].w = positionnelle ? 1.0 : 0.0;
      std::cout << " positionnelle=" << positionnelle << std::endl;
      break;

   case TP_s: // Varier le type d'affichage stéréo: mono, stéréo anaglyphe, stéréo double
      if ( ++affichageStereo > 2 ) affichageStereo = 0;
      std::cout << " affichageStereo=" << affichageStereo << std::endl;
      break;

   case TP_g: // Permuter l'affichage en fil de fer ou plein
      modePolygone = ( modePolygone == GL_FILL ) ? GL_LINE : GL_FILL;
      glPolygonMode( GL_FRONT_AND_BACK, modePolygone );
      break;

   case TP_n: // Utiliser ou non les normales calculées comme couleur (pour le débogage)
      afficheNormales = !afficheNormales;
      chargerNuanceurs(); // pour le #define
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

// fonction callback pour un clic de souris
int dernierX = 0; // la dernière valeur en X de position de la souris
int dernierY = 0; // la derniere valeur en Y de position de la souris
static enum { deplaceCam, deplaceLumProfondeur, deplaceLum } deplace = deplaceCam;
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
      case TP_BOUTON_GAUCHE: // Déplacer la caméra
         deplace = deplaceCam;
         break;
      case TP_BOUTON_MILIEU: // Déplacer la lumière en profondeur uniquement
         deplace = deplaceLumProfondeur;
         break;
      case TP_BOUTON_DROIT: // Déplacer la lumière à la position de la souris (sans changer la profondeur)
         deplace = deplaceLum;
         break;
      }
      if ( deplace != deplaceCam )
      {
         glm::mat4 VM = matrVisu.getMatr()*matrModel.getMatr();
         glm::mat4 P = matrProj.getMatr();
         glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
         glm::vec2 ecranLumi0 = glm::vec2( glm::project( LightSource.position[0].xyz(), VM, P, cloture ) );
         glm::vec2 ecranLumi1 = glm::vec2( glm::project( LightSource.position[1].xyz(), VM, P, cloture ) );
         glm::vec2 ecranXY( x, hauteur_-y );
         curLumi = ( glm::distance( ecranLumi0, ecranXY ) <
                     glm::distance( ecranLumi1, ecranXY ) ) ? 0 : 1;
      }
   }
   else
   {
      // on vient de relacher la souris
   }
}

void FenetreTP::sourisWheel( int x, int y ) // Changer le facteurZ
{
   const int sens = +1;
   facteurZ += 0.01 * sens * y;
   std::cout << " facteurZ=" << facteurZ << std::endl;
}

// fonction de mouvement de la souris
void FenetreTP::sourisMouvement( int x, int y )
{
   if ( pressed )
   {
      int dx = x - dernierX;
      int dy = y - dernierY;
      glm::mat4 VM = matrVisu.getMatr()*matrModel.getMatr();
      glm::mat4 P = matrProj.getMatr();
      glm::vec4 cloture( 0, 0, largeur_, hauteur_ );
      // obtenir les coordonnées d'écran correspondant à la position de la lumière
      glm::vec3 ecranLumi = glm::project( LightSource.position[curLumi].xyz(), VM, P, cloture );
      switch ( deplace )
      {
      case deplaceCam: // déplacer la caméra par incrément
         thetaCam -= dx / 3.0;
         phiCam   -= dy / 3.0;
         break;
      case deplaceLumProfondeur:
         // modifier seulement la profondeur de la lumière
         ecranLumi[2] -= dy * 0.001;
         LightSource.position[curLumi] = glm::vec4( glm::unProject( ecranLumi, VM, P, cloture ), positionnelle ? 1.0 : 0.0 );
         break;
      case deplaceLum:
         // placer la lumière à la nouvelle position (en utilisant la profondeur actuelle)
         glm::vec3 ecranPos( x, hauteur_-y, ecranLumi[2] );
         LightSource.position[curLumi] = glm::vec4( glm::unProject( ecranPos, VM, P, cloture ), positionnelle ? 1.0 : 0.0 );
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
