#version 410

uniform mat4 matrModel;
uniform mat4 matrVisu;
uniform mat4 matrProj;

layout(location=0) in vec4 Vertex;
layout(location=3) in vec4 Color;
in float tempsDeVieRestant;
in vec3 vitesse;

out Attribs {
   vec4 couleur;
   float tempsDeVieRestant;
   float sens; // du vol
} AttribsOut;

void main( void )
{
   // transformation standard du sommet
   gl_Position = matrProj * matrVisu * matrModel * Vertex;

   AttribsOut.tempsDeVieRestant = tempsDeVieRestant;

   // couleur du sommet
   AttribsOut.couleur = Color;

   // À SUPPRIMER: la ligne suivante sert seulement à forcer le compilateur à conserver cet attribut
   if ( tempsDeVieRestant < 0.0 ) AttribsOut.couleur.a += 0.00001;
   
   AttribsOut.sens = sign( ( matrVisu * matrModel * vec4(vitesse, 0) ).x);
}
