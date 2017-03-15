#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position;
   vec3 spotDirection;
   float spotExponent;
   float spotCutoff;            // ([0.0,90.0] ou 180.0)
   float constantAttenuation;
   float linearAttenuation;
   float quadraticAttenuation;
} LightSource[1];

// Définition des paramètres des matériaux
layout (std140) uniform MaterialParameters
{
   vec4 emission;
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float shininess;
} FrontMaterial;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

layout (std140) uniform varsUnif
{
   // partie 1: illumination
   int typeIllumination;     // 0:Lambert, 1:Gouraud, 2:Phong
   bool utiliseBlinn;        // indique si on veut utiliser modèle spéculaire de Blinn ou Phong
   bool utiliseDirect;       // indique si on utilise un spot style Direct3D ou OpenGL
   bool afficheNormales;     // indique si on utilise les normales comme couleurs (utile pour le débogage)
   // partie 3: texture
   int texnumero;            // numéro de la texture appliquée
   bool utiliseCouleur;      // doit-on utiliser la couleur de base de l'objet en plus de celle de la texture?
   int afficheTexelNoir;     // un texel noir doit-il être affiché 0:noir, 1:mi-coloré, 2:transparent?
};

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform mat3 matrNormale;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Vertex;
layout(location=2) in vec3 Normal;
layout(location=3) in vec4 Color;
layout(location=8) in vec4 TexCoord;

out Attribs {
   vec4 couleur;
   vec3 normale;
   vec3 lumiDir, obsVec;
} AttribsOut;

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

   // calcul de la composante ambiante
   coul += FrontMaterial.ambient * LightSource[0].ambient;

   // calcul de l'Ã©clairage seulement si le produit scalaire est positif
   float NdotL = max( 0.0, dot( N, L ) );
   if ( NdotL > 0.0 )
   {
      // calcul de la composante diffuse
      coul += FrontMaterial.diffuse * LightSource[0].diffuse * NdotL;
      //coul += Color * LightSource[0].diffuse * NdotL; //(ici, on utilise plutÃ´t la couleur de l'objet)

      // calcul de la composante spÃ©culaire (Blinn ou Phong)
      float NdotHV = max( 0.0, ( utiliseBlinn ) ? dot( normalize( L + O ), N ) : dot( reflect( -L, N ), O ) );
      coul += FrontMaterial.specular * LightSource[0].specular * ( ( NdotHV == 0.0 ) ? 0.0 : pow( NdotHV, FrontMaterial.shininess ) );
   }
	
   return clamp( coul, 0.0, 1.0 );
}

void main( void )
{
	// transformation standard du sommet (P * V * M * sommet)
   gl_Position = matrVisu * matrModel * Vertex;

   // calculer la normale qui sera interpolÃ©e pour le nuanceur de fragment
   vec3 N = matrNormale * Normal;
   AttribsOut.normale = N;

   // calculer la position du sommet (dans le repÃ¨re de la camÃ©ra)
   vec3 pos = vec3( matrVisu * matrModel * Vertex );

   // vecteur de la direction de la lumiÃ¨re (dans le repÃ¨re de la camÃ©ra)
   vec3 L = vec3( ( matrVisu * LightSource[0].position ).xyz - pos );
   AttribsOut.lumiDir = L;

   // vecteur de la direction vers l'observateur
   vec3 O = ( LightModel.localViewer ?
			 normalize(-pos) :        // =(0-pos) un vecteur qui pointe vers le (0,0,0), c'est-Ã -dire vers la camÃ©ra
			 vec3( 0.0, 0.0, 1.0 ) ); // on considÃ¨re que l'observateur (la camÃ©ra) est Ã  l'infini dans la direction (0,0,1)
   AttribsOut.obsVec = O;



   // la couleur du sommet
   
   if(typeIllumination == 1)
   {
      AttribsOut.couleur = calculerReflexion(L, N, O);
   }
   else
   {
      AttribsOut.couleur = Color;
   }
}
