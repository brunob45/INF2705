//#version 410

layout(vertices = 4) out;

// in int gl_PatchVerticesIn;
// in int gl_PrimitiveID;
// in int gl_InvocationID;
// in gl_PerVertex
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_in[gl_MaxPatchVertices];

// patch out float gl_TessLevelOuter[4];
// patch out float gl_TessLevelInner[2];
// out gl_PerVertex
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_out[];

uniform float TessLevelInner;
uniform float TessLevelOuter;

void main()
{
   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

   if ( gl_InvocationID == 0 )
   {
      gl_TessLevelInner[0] =
      gl_TessLevelInner[1] = TessLevelInner;
      gl_TessLevelOuter[0] =
      gl_TessLevelOuter[1] =
      gl_TessLevelOuter[2] =
      gl_TessLevelOuter[3] = TessLevelOuter;
   }
}
