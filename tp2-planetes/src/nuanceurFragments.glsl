#version 410

in Attribs {
   vec4 couleur;
} AttribsIn;

out vec4 FragColor;

uniform int coulProfondeur;
const float debAttenuation = 20.0;
const float finAttenuation = 40.0;

void main( void )
{
   // la couleur du fragment est la couleur interpolée
   FragColor = AttribsIn.couleur;

   // atténuer selon la profondeur
   if ( coulProfondeur == 1 )
   {
      // On veut la profondeur du sommet dans le repère de la caméra et
      // cette distance aurait pu calculée dans le nuanceur de sommets avec :
      //    out float profondeur = -( matrVisu * matrModel * Vertex ).z;
      // Cette valeur peut toutefois être obtenue dans le nuanceur de fragments avec :
      float profondeur = gl_FragCoord.z / gl_FragCoord.w;
      profondeur = (profondeur-debAttenuation)/(finAttenuation - debAttenuation);
      
      // Convertir la profondeur de [debAttenuation,finAttenuation] en un facteur de [0,1] et le limiter à cet intervalle
      float factProfondeur = 1-clamp(profondeur, 0,1);

      // Multiplier les composantes RGB par ce facteur
      FragColor.rgb *= factProfondeur;

      // pour déboguer et « voir » la profondeur, on peut utiliser:

      //FragColor = vec4( vec3(profondeur-floor(profondeur)), 1.0 );
      //FragColor = vec4( vec3(factProfondeur), 1.0 );
      // pour déboguer et « voir » le comportement de z ou w, on peut utiliser:
      //FragColor = vec4( vec3(gl_FragCoord.z), 1.0 );
      //FragColor = vec4( vec3(gl_FragCoord.w), 1.0 );
   }
}
