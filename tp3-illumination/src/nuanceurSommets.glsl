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
   vec3 lumiDir, obsvec;
} AttribsOut;

vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	// ajout de l’émission et du terme ambiant du modèle d’illumination
	vec4 ret = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;
	
	// calcul de la composante ambiante de la 1e source de lumière
	ret += FrontMaterial.ambient * LightSource[0].ambient;
	
	// produit scalaire pour le calcul de la réflexion diffuse
	float NdotL = max (0.0, dot(N, L));
	// calcul de la composante diffuse de la 1e source de lumière
	ret += FrontMaterial.diffuse * LightSource[0].diffuse * NdotL;
	
	// calcul de la composante spéculaire (selon Phong ou Blinn)
	float NdotHV = max( 0.0, dot( normalize( L + O ), N ) );
	// calcul de la composante spéculaire de la 1e source de lumière
	ret += FrontMaterial.specular * LightSource[0].specular *
        pow( NdotHV, FrontMaterial.shininess );
      
   return ret;
}

void main( void )
{
	
	// calculer la position du sommet dans le repère de la caméra
	vec3 pos = vec3( matrVisu * matrModel * Vertex );
	
	vec4 color;
	vec3 L = (matrVisu * LightSource[0].position).xyz / LightSource[0].position.w - pos;
	vec3 N = matrNormale * Normal;
	vec3 O = normalize(-pos);
	
	
	// transformation standard du sommet (ModelView et Projection)
   gl_Position = matrProj * matrVisu * Vertex;
   // ou gl_Position = matrProj * matrVisu * matrModel * Vertex;

   // normaliser le demi-vecteur (Blinn)
   //vec3 halfV = normalize( vec3(LightSource[0].halfVector) );
   // produit scalaire pour la rÃ©flexion spÃ©culaire
   float NdotHV = max( 0.0, dot( normalize( L + O ), N ) );

   // composante diffuse (gl_LightSource[i].position est dÃ©jÃ  dans le repÃ¨re de la camÃ©ra)
   color = FrontMaterial.diffuse * LightSource[0].diffuse * abs(dot(N,vec3(LightSource[0].position)));
   // composante spÃ©culaire
   color += FrontMaterial.specular * LightSource[0].specular * pow(NdotHV, FrontMaterial.shininess);
   // composante ambiante
   color += FrontMaterial.ambient * LightSource[0].ambient;
   
   AttribsOut.couleur = color;
	
/*	
    // transformation standard du sommet
    //gl_Position = matrProj * matrVisu * matrModel * Vertex;
    gl_Position = matrVisu * matrModel * Vertex;
    // multiplier matrProj apres [...]!

    // calculer la normale
	AttribsOut.normale = matrNormale * Normal;
	
	// calculer la position du sommet dans le repère de la caméra
	vec3 pos = vec3( matrVisu * matrModel * Vertex );
	
	AttribsOut.lumiDir = (matrVisu * LightSource[0].position).xyz / LightSource[0].position.w - pos;
	
	
	AttribsOut.obsvec = normalize(-pos);

	AttribsOut.couleur = Color;*/
}
