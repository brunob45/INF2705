#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

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

in Attribs {
   vec4 couleur;
   vec3 normale;
   vec3 lumiDir, obsVec;
} AttribsIn[];

out Attribs {
   vec4 couleur;
   vec3 normale;
   vec3 lumiDir, obsVec;
} AttribsOut;

void main()
{
	vec3 normaleMoy = normalize((cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, 
							gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz )));
							
	/*if(typeIllumination == 0)
	{
		for ( int i = 0 ; i < gl_in.length() ; ++i )
		{
			normaleMoy += AttribsIn[i].normale;
		}
		normaleMoy /= gl_in.length();
	}*/
	
   // émettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_Position = matrProj * gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      
      if(typeIllumination == 0)
      {
		  AttribsOut.normale = normaleMoy;
	  }
	  else
	  {
		  AttribsOut.normale = AttribsIn[i].normale;
	  }
	  
      AttribsOut.lumiDir = AttribsIn[i].lumiDir;
      AttribsOut.obsVec = AttribsIn[i].obsVec;
      
      EmitVertex();
   }
}
