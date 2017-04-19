#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in uint VertexTexID;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;
flat out uint TexID;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

uniform mat4 MVP;           // ModelView * Projection


// Define shader functions

void getEyeSpace( out vec3 norm , out vec3 position )
{
   norm = normalize( NormalMatrix * VertexNormal );
   position = vec3( ModelViewMatrix * vec4( VertexPosition, 1.0 ) );
}

void main()
{
   // Just pass the texture coordinate on for per-fragment interpolation
   TexCoord = VertexTexCoord;
   TexID = VertexTexID;

   // Convert normal and position to eye coords
   getEyeSpace( Normal , Position );
   
   // Convert position to clip coordinates and pass along
   gl_Position = MVP * vec4(VertexPosition, 1.0);
   //gl_Position = vec4(VertexPosition, 1.0);
}