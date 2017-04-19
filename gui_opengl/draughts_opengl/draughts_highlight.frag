
#version 400 

in vec3 Position;   // Interpolated position of this fragment

layout (location = 0) out vec4 FragColour;

uniform vec4 LineColour;


void main()
{
   FragColour = LineColour;
}

