
#version 400 

in vec2 TexCoord;	// Interpolated texture coordinate

layout (location = 0) out vec4 FragColour;

uniform sampler2D TextTexture;

uniform vec3 foregroundColour;

uniform bool bUseAlpha;
uniform vec3 backGroundColour;	// If bUseAlpha is false, then use this colour for the background (alpha<1) areas


void main()
{
   vec4 texColour = texture( TextTexture , TexCoord );
   
   // Hack to get an alpha from a black/white image
   //FragColour = vec4( colour*texColour.rgb, texColour.r );

   if( bUseAlpha )
      FragColour = vec4( foregroundColour*texColour.rgb, texColour.a );
   else
	   FragColour = vec4( (foregroundColour*texColour.rgb*texColour.a + backGroundColour*(1.0-texColour.a)), 1.0 );
   
   //FragColour = vec4( 1.0, 1.0, 1.0 , 1.0 );
}

