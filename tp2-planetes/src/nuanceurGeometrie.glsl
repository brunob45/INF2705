#version 410

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

layout(triangles) in;
layout(triangle_strip, max_vertices = 8) out;

// in gl_PerVertex // <-- dÃ©claration implicite
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_in[];
// 
// out gl_PerVertex // <-- dÃ©claration implicite
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// };
// out int gl_Layer;
// out int gl_ViewportIndex; // si GL 4.1 ou ARB_viewport_array.

in Attribs {
   vec4 couleur;
} AttribsIn[];

out Attribs {
   vec4 couleur;
} AttribsOut;

void main()
{
   // Ã©mettre les sommets
   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_ViewportIndex = 0;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;

      gl_ClipDistance[0] = -gl_in[0].gl_ClipDistance[0];
      
      EmitVertex();
   }
   EndPrimitive();

   for ( int i = 0 ; i < gl_in.length() ; ++i )
   {
      gl_ViewportIndex = 1;
      gl_Position = gl_in[i].gl_Position;
      AttribsOut.couleur = AttribsIn[i].couleur;
      gl_ClipDistance[0] = gl_in[0].gl_ClipDistance[0];
      EmitVertex();
   }
   EndPrimitive();
}
