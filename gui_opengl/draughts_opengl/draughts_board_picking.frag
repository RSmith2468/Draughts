
#version 400 

flat in vec3 IDColour; // Fragment colour

layout (location = 0) out vec4 FragColour;


void main()
{
   FragColour = vec4( IDColour, 1.0 );
   //FragColour = vec4( 1.0, 1.0, 1.0, 1.0 );
}

