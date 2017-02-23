#version 410

const float M_PI = 3.14159265358979323846;	// pi
const float M_PI_2 = 1.57079632679489661923;	// pi/2
const vec4 couleurPole = vec4( 1.0, 1.0, 1.0, 1.0 ); // couleur du pôle sans réchauffement

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

uniform float facteurRechauffement; // facteur qui sert à calculer la couleur des pôles (0.0=froid, 1.0=chaud)
uniform vec4 planCoupe; // équation du plan de coupe

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;

out Attribs {
   vec4 couleur;
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrProj * matrVisu * matrModel * Vertex;

   // couleur du sommet
   AttribsOut.couleur = Color;
   if(Color[3] > 0.9)
   {
      AttribsOut.couleur = Color + abs(couleurPole * Vertex[2]*(1-facteurRechauffement))/5;
   }
	
   // clip plane
   vec4 pos = matrModel * Vertex;
   gl_ClipDistance[0] = dot( planCoupe, pos );
}
