#version 410

uniform vec3 bDim, positionPuits;
uniform float temps, dt, tempsVieMax, gravite;

in vec3 position;
in vec3 vitesse;
in vec4 couleur;
in float tempsDeVieRestant;

out vec3 positionMod;
out vec3 vitesseMod;
out vec4 couleurMod;
out float tempsDeVieRestantMod;

uint randhash( uint seed ) // entre  0 et UINT_MAX
{
    uint i=(seed^12345391u)*2654435769u;
    i ^= (i<<6u)^(i>>26u);
    i *= 2654435769u;
    i += (i<<5u)^(i>>12u);
    return i;
}
float myrandom( uint seed ) // entre  0 et 1
{
   const float UINT_MAX = 4294967295.0;
   return float(randhash(seed)) / UINT_MAX;
}


void main( void )
{
   if ( tempsDeVieRestant <= 0.0 )
   {
      // se préparer à produire une valeur un peu aléatoire
      uint seed = uint(temps * 1000.0) + uint(gl_VertexID);
      // faire renaitre la particule au puits
      positionMod = positionPuits;
      vitesseMod = vec3( myrandom(seed++)-0.5,       // entre -0.5 et 0.5
                         myrandom(seed++)-0.5,       // entre -0.5 et 0.5
                         myrandom(seed++)*0.5+0.5 ); // entre  0.5 et 1
      
      tempsDeVieRestantMod = myrandom(seed++) * tempsVieMax; // entre 0 et tempsVieMax secondes
      
      const float COULMIN = 0.2; // valeur minimale d'une composante de couleur lorsque la particule (re)naît
      const float COULMAX = 0.9; // valeur maximale d'une composante de couleur lorsque la particule (re)naît
      
      // interpolation linéaire entre COULMIN et COULMAX
      couleurMod = vec4( myrandom(seed++) * (COULMAX - COULMIN) + COULMIN, 
		                 myrandom(seed++) * (COULMAX - COULMIN) + COULMIN,
		                 myrandom(seed++) * (COULMAX - COULMIN) + COULMIN,
		                 1.0);
  }
   else
   {
	  const float DISTMAX = 1.;  // constante, sinon erreurs a l'execution.
	  // avancer la particule
      // ...
      positionMod = position + vitesse * dt;
	  vitesseMod = vitesse - vec3(0,0,gravite * dt);
      couleurMod = couleur;
      tempsDeVieRestantMod = tempsDeVieRestant - dt;
	  
	   bool collision = false;
	   
	   
      vec3 posSphUnitaire = positionMod / bDim ;
      vec3 vitSphUnitaire = vitesseMod * bDim ;
      
      float dist = length ( posSphUnitaire );
      if (dist >= DISTMAX) // ... la particule est sortie de la bulle par un cote.
      {
		  positionMod = ( 1 + DISTMAX - dist ) * positionMod ;
          vec3 N = posSphUnitaire / dist ; // normaliser N
          vec3 vitReflechieSphUnitaire = reflect ( vitSphUnitaire , N );
          vitesseMod = vitReflechieSphUnitaire / bDim ;
          couleurMod.a *= 0.5;
		  
	  }
      if (positionMod.z < 0) // ... la particule est sortie de la bulle par en dessous.
      {
		  positionMod.z = 0;
		  vec3 N = vec3(0,0,1); ; // normaliser N
          vec3 vitReflechieSphUnitaire = reflect ( vitSphUnitaire , N );
          vitesseMod = vitReflechieSphUnitaire / bDim ;
		  couleurMod.a *= 0.5;
	  }
   }
}
