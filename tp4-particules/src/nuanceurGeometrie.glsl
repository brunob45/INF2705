#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in Attribs {
   vec4 couleur;
   float tempsDeVieRestant;
   float sens; // du vol
} AttribsIn[];

out Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsOut;

void main()
{
   gl_PointSize = 10.0; // en pixels
   gl_Position = gl_in[0].gl_Position;
   
   AttribsOut.couleur = AttribsIn[0].couleur;
   AttribsOut.texCoord = vec2(sign(AttribsIn[0].sens) * 1./16, -1.);
   EmitVertex();
}
