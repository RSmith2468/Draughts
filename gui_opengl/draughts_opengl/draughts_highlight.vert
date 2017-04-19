#version 400

layout (location = 0) in vec3 VertexPosition;

out vec3 Position;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

uniform mat4 MVP;           // ModelView * Projection


// Define shader functions

void getEyeSpace( out vec3 position )
{
   position = vec3( ModelViewMatrix * vec4( VertexPosition, 1.0 ) );
}

void main()
{
   // Convert normal and position to eye coords
   getEyeSpace( Position );
   
   // Convert position to clip coordinates and pass along
   gl_Position = MVP * vec4(VertexPosition, 1.0);
   //gl_Position = vec4(VertexPosition, 1.0);
}