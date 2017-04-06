//#version 410

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position[2];
} LightSource;

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

uniform sampler2D textureCoul;

// bool gl_FrontFacing  // variable standard d'OpenGL: c'est la face avant qui est actuellement tracée?

in Attribs {
   vec2 texCoord;
   vec3 normale, lumiDir[2], obsVec;
} AttribsIn;

out vec4 FragColor;

bool utiliseBlinn = true;

/*vec4 calculerReflexion( in vec3 L, in vec3 N, in vec3 O )
{
	vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

   // calcul de la composante ambiante
   coul += FrontMaterial.ambient * LightSource.ambient;

   // calcul de l'Ã©clairage seulement si le produit scalaire est positif
   float NdotL = max( 0.0, dot( N, L ) );
   if ( NdotL > 0.0 )
   {
      // calcul de la composante diffuse
      coul += FrontMaterial.diffuse * LightSource.diffuse * NdotL;
      //coul += AttribsIn.couleur * LightSource[0].diffuse * NdotL; //(ici, on utilise plutÃ´t la couleur de l'objet)

      // calcul de la composante spÃ©culaire (Blinn ou Phong)
      float NdotHV = max( 0.0, ( utiliseBlinn ) ? dot( normalize( L + O ), N ) : dot( reflect( -L, N ), O ) );
      coul += FrontMaterial.specular * LightSource.specular * ( ( NdotHV == 0.0 ) ? 0.0 : pow( NdotHV, FrontMaterial.shininess ) );
   }
   
   //vec4 coul = AttribsIn.couleur * dot( N, L );
	
   return clamp( coul, 0.0, 1.0 );
}*/

void main( void )
{
   vec4 coul = FrontMaterial.emission + FrontMaterial.ambient * LightModel.ambient;

   vec3 N = normalize( gl_FrontFacing ? AttribsIn.normale : -AttribsIn.normale );// vecteur normal
   vec3 O = normalize( AttribsIn.obsVec );  // position de l'observateur

   // calculer la contribution de chaque source lumineuse
   for ( int i = 0 ; i < 2 ; ++i )
   {
	  vec3 L = normalize( AttribsIn.lumiDir[i] ); // vecteur vers la source lumineuse
      // ajouter la contribution de la composante ambiante
      coul += FrontMaterial.ambient * LightSource.ambient;

      float NdotL = max( 0.0, dot( N, L ) );

      // calcul de l'éclairage seulement si le produit scalaire est positif
      if ( NdotL > 0.0 )
      {
         // ajouter la contribution de la composante diffuse
#if ( INDICEDIFFUSE == 0 )
         // la composante diffuse (kd) du matériel est utilisé
         coul += FrontMaterial.diffuse * LightSource.diffuse * NdotL;
#else
         // la composante diffuse (kd) provient de la texture 'textureCoul'
         coul += FrontMaterial.diffuse * LightSource.diffuse * NdotL; // modifier BRUNO
#endif

         // ajouter la contribution de la composante spéculaire
         float NdotHV = max( 0.0, ( utiliseBlinn ) ? dot( normalize( L + O ), N ) : dot( reflect( -L, N ), O ) );
         coul += FrontMaterial.specular * LightSource.specular * ( ( NdotHV == 0.0 ) ? 0.0 : pow( NdotHV, FrontMaterial.shininess ) );
      }
   }

   // assigner la couleur finale
   FragColor = clamp( coul, 0.0, 1.0 );

#if ( AFFICHENORMALES == 1 )
   // pour le débogage
   FragColor = vec4(N,1.0);
#endif
}
