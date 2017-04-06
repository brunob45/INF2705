//#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

// Définition des paramètres des sources de lumière
layout (std140) uniform LightSourceParameters
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   vec4 position[2];
} LightSource;

// Définition des paramètres globaux du modèle de lumière
layout (std140) uniform LightModelParameters
{
   vec4 ambient;       // couleur ambiante
   bool localViewer;   // observateur local ou à l'infini?
   bool twoSide;       // éclairage sur les deux côtés ou un seul?
} LightModel;

//uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;
uniform vec4 bDim;

in Attribs {
   vec2 texCoord;
   vec3 normale;
} AttribsIn[];

out Attribs {
   vec2 texCoord;
   vec3 normale, lumiDir[2], obsVec;
} AttribsOut;

void main()
{
   // émettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      AttribsOut.texCoord = AttribsIn[i].texCoord;
      AttribsOut.normale = AttribsIn[i].normale;

      // initialiser gl_ClipDistance[] pour que le découpage psoit fait par OpenGL
      vec4 planCoupeHaut = vec4( 0.0, 0.0, -1.0, bDim.z );
      vec4 planCoupeBas = vec4( 0.0, 0.0, 1.0, bDim.z );
      gl_ClipDistance[0] = dot( planCoupeHaut, gl_in[i].gl_Position );
      gl_ClipDistance[1] = dot( planCoupeBas, gl_in[i].gl_Position );

      // calculer la position du sommet dans l'espace de la caméra
      vec4 posVisu = matrVisu * gl_in[i].gl_Position;

      // vecteur de la direction vers la lumière
      for ( int j = 0 ; j < 2 ; ++j )
      {
         if ( LightSource.position[j].w != 0.0 ) // lumière positionnelle en (x/w,y/w,z/w)
           AttribsOut.lumiDir[j] = vec3( ( matrVisu * LightSource.position[j] ).xyz - posVisu.xyz) / LightSource.position[j].w;
         else                                    // lumière directionnelle dans la direction (x,y,z)
           AttribsOut.lumiDir[j] = vec3( ( matrVisu * LightSource.position[j] ).xyz - posVisu.xyz) / LightSource.position[j].w;
         //ou mieux encore: AttribsOut.lumiDir[j] = ( LightSource.position[j].w != 0.0 ) ? ... : ...;
      }

      // vecteur de la direction vers l'observateur
      AttribsOut.obsVec = ( LightModel.localViewer ?
			 normalize(-posVisu.xyz) :        // =(0-pos) un vecteur qui pointe vers le (0,0,0), c'est-Ã -dire vers la camÃ©ra
			 vec3( 0.0, 0.0, 1.0 ) ); // on considÃ¨re que l'observateur (la camÃ©ra) est Ã  l'infini dans la direction (0,0,1)


      // terminer la transformation standard du sommet
      gl_Position = matrProj * posVisu;

      EmitVertex();
   }
}
