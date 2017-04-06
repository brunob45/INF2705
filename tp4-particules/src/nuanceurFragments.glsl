#version 410

uniform sampler2D laTexture;
uniform int texnumero;

in Attribs {
   vec4 couleur;
   vec2 texCoord;
} AttribsIn;

out vec4 FragColor;

void main( void )
{   
   vec4 color = AttribsIn.couleur;
   
   if( texnumero > 0)
   {
	   vec2 texCoord = vec2(1.,1.);
	   if(texnumero > 1)
	   {
		   texCoord = AttribsIn.texCoord;
	   }
	   
	   color = texture( laTexture, gl_PointCoord * texCoord );
	   if(color.a < 0.1)
	   {
		   discard;
	   }
   }
   FragColor = vec4(mix( AttribsIn.couleur.rgb, color.rgb, 0.6 ), AttribsIn.couleur.a);
}
