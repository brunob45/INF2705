//#version 410

layout(quads) in;

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

// in vec3 gl_TessCoord;
// in int gl_PatchVerticesIn;
// in int gl_PrimitiveID;
// patch in float gl_TessLevelOuter[4];
// patch in float gl_TessLevelInner[2];
// in gl_PerVertex
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_in[gl_MaxPatchVertices];

// out gl_PerVertex {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// };

uniform mat3 matrNormale;

uniform float facteurZ;
uniform sampler2D textureDepl;

out Attribs {
   vec2 texCoord;
   vec3 normale;
} AttribsOut;

float interpole( float v0, float v1, float v2, float v3 )
{
   float v01 = mix( v0, v1, gl_TessCoord.x );
   float v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec2 interpole( vec2 v0, vec2 v1, vec2 v2, vec2 v3 )
{
   vec2 v01 = mix( v0, v1, gl_TessCoord.x );
   vec2 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec3 interpole( vec3 v0, vec3 v1, vec3 v2, vec3 v3 )
{
   vec3 v01 = mix( v0, v1, gl_TessCoord.x );
   vec3 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}
vec4 interpole( vec4 v0, vec4 v1, vec4 v2, vec4 v3 )
{
   vec4 v01 = mix( v0, v1, gl_TessCoord.x );
   vec4 v32 = mix( v3, v2, gl_TessCoord.x );
   return mix( v01, v32, gl_TessCoord.y );
}

// déplacement selon la fonction mathématique
float FctMath( vec2 uv )
{
   float x = uv.x;
   float y = uv.y;
   float z = 0.0;
#if ( INDICEFONCTION == 1 )
   z = 0.5 * ( x*x - y*y );
#elif ( INDICEFONCTION == 2 )
   z = ( x*y * cos(x)*sin(y) );
#elif ( INDICEFONCTION == 3 )
   z = 4.0 * x*y / exp(x*x + y*y);
#elif ( INDICEFONCTION == 4 )
   z = 0.4 * sin(3.0*x)*cos(6.0*y);
#elif ( INDICEFONCTION == 5 )
   float w = x*x+y*y; z = (1.0-5.0*w) * exp(-w);
#elif ( INDICEFONCTION == 6 )
   z = 0.2 * exp(sin(5.0*y)*sin(2.0*x)) * exp(sin(1.0*y)*sin(5.0*x));
#endif
   return z * facteurZ;
}

// déplacement selon la texture
float FctText( vec2 texCoord )
{
   vec4 tex = texture(textureDepl, texCoord);
   return length( tex ) * facteurZ / 10;
}

float FctZ( vec2 uv )
{
#if ( INDICEFONCTION != 0 )
	return FctMath( uv );
#else
	return FctText( uv );
#endif
}

vec3 calcNorm(vec2 uv, float delta)
{
	return vec3 ( FctZ(vec2(uv.x+delta, uv.y)) - FctZ(vec2(uv.x-delta, uv.y)),
	              FctZ(vec2(uv.x, uv.y+delta)) - FctZ(vec2(uv.x, uv.y-delta)),
	              -2*delta) / (2*delta);
}

void main( void )
{
   const float eps = 0.01;

   // interpoler la position selon la tessellation (dans le repère de modélisation)
   vec4 posModel = interpole( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position );

   // générer (en utilisant posModel.xy) les coordonnées de texture plutôt que les interpoler
   AttribsOut.texCoord = posModel.xy;

	// déplacement en z
	posModel.z = FctZ( posModel.xy );
   // calculer la normale
	vec3 N = calcNorm( posModel.xy, eps );

#if ( AFFICHENORMALES == 1 )
   AttribsOut.normale = N;
#else
   AttribsOut.normale = matrNormale * N;
#endif

   gl_Position = posModel;
}
