#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;

out vec2 TexCoord;

uniform mat4 MVP;           // ModelView * Projection


void main()
{
   // Just pass the texture coordinate on for per-fragment interpolation
   TexCoord = VertexTexCoord;

   // Convert position to clip coordinates and pass along
   gl_Position = MVP * vec4(VertexPosition, 1.0);
   //gl_Position = vec4(VertexPosition, 1.0);
}