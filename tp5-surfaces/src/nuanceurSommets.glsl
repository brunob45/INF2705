//#version 410

uniform mat4 matrModel;
uniform vec4 bDim;

/////////////////////////////////////////////////////////////////

layout(location=0) in vec4 Vertex;

void main( void )
{
   // appliquer la transformation de modélisation et
   // passer le sommet au nuanceur de tessellation pour le déplacement
   gl_Position = bDim * matrModel * Vertex;
}
