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

uniform sampler2D laTexture;

/////////////////////////////////////////////////////////////////

in Attribs {
   vec4 couleur;
   vec3 normale;
   vec3 lumiDir, obsVec;
   vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

float calculerSpot( in vec3 spotDir, in vec3 L )
{
	float gamma = dot( L, spotDir );
	float delta = cos(radians(LightSource[0].spotCutoff));
	float c = LightSource[0].spotExponent;
	float facteur = 0.0;
	
	if(gamma >  delta)
	{
		if(utiliseDirect)
		{
			float outer = pow(delta, (1.01+c/2));
			facteur = (gamma - outer)/(delta - outer);
		}
		else
		{
			facteur = pow(gamma,  c);
	    }
	}
	
   return( facteur );
}

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
      //coul += AttribsIn.couleur * LightSource[0].diffuse * NdotL; //(ici, on utilise plutÃ´t la couleur de l'objet)

      // calcul de la composante spÃ©culaire (Blinn ou Phong)
      float NdotHV = max( 0.0, ( utiliseBlinn ) ? dot( normalize( L + O ), N ) : dot( reflect( -L, N ), O ) );
      coul += FrontMaterial.specular * LightSource[0].specular * ( ( NdotHV == 0.0 ) ? 0.0 : pow( NdotHV, FrontMaterial.shininess ) );
   }
   
   //vec4 coul = AttribsIn.couleur * dot( N, L );
	
   return clamp( coul, 0.0, 1.0 );
}

void main( void )
{
   vec3 L = normalize( AttribsIn.lumiDir ); // vecteur vers la source lumineuse
   vec3 N = normalize( AttribsIn.normale ); // vecteur normal
   vec3 O = normalize( AttribsIn.obsVec );  // position de l'observateur
   vec3 D = normalize( -LightSource[0].spotDirection );

   // assigner la couleur finale
   vec4 couleur = AttribsIn.couleur;
   
   if(typeIllumination == 0 || typeIllumination == 2)
   {
	  // Lambert ou Phong
	  couleur = calculerReflexion(L, N, O);
   }   
   
   couleur *= calculerSpot(D, L);
      
   if(texnumero != 0)
   {
	  // calculer la couleur de la texture pour le fragment
	  vec4 texcolor = texture(laTexture, AttribsIn.texCoord);

	  // si la couleur est noir
      if(texcolor.x < 0.1 && texcolor.y < 0.1 && texcolor.z < 0.1)
      {
         switch(afficheTexelNoir)
         {
            case 0: // afficher le noir noir
  	 		   couleur = couleur * texcolor;
  	           break;
  	        case 1: // afficher le noir semi-colore
  	           couleur = 0.5*(texcolor + couleur);
  	 		   break;
  	        case 2: // ne pas afficher le noir (transparent)
  	 		   discard;
         }
     }
   }
   
   FragColor = couleur; 
}

