
#version 400 

in vec3 Position;   // Interpolated position of this fragment
in vec3 Normal;     // Interpolated normal of this fragment
in vec2 TexCoord;	// Interpolated texture coordinate
flat in uint TexID; // Specifies which texture to use

layout (location = 0) out vec4 FragColour;

uniform sampler2D BlackSquareTex;  // Texture sampler for the black board squares
uniform sampler2D WhiteSquareTex;  // Texture sampler for the white board squares

// Define structures for storing the ADS light info & material properties
struct LightInfo
{
   vec4 Position;  // light position in eye-coords
   vec3 Intensity; // light intensity
};

struct MaterialInfo
{
   vec3 Ka;         // ambient reflectivity
   vec3 Kd;         // diffuse reflectivity
   vec3 Ks;         // specular reflectivity
   float Shininess; // specular shininess factor
};

// Create light info & material properties structures
uniform LightInfo Light;
uniform MaterialInfo Material;

// Define shader functions

void ads( out vec3 ad, out vec3 spec )
{
   vec3 norm = normalize( Normal );
   // Calculate various vectors needed for the ads calcualtions
   vec3 s = normalize( vec3(Light.Position) - Position ); // light-source vector to surface
   vec3 v = normalize( -Position );                   // camera/eye vector to surface
   vec3 r = reflect( -s, norm );                          // vector of perfect reflection
   
   // Calculate the ambient light intensity
   vec3 ambient = Material.Ka;
   
   // Calculate the diffuse light intensity
   float sDotN = max( dot(s, norm), 0.0 );
   vec3 diffuse = Material.Kd * sDotN;

   // Calculate the specular reflection intensity
   spec = vec3(0.0);
   if( sDotN > 0.0 )
      spec = Light.Intensity * Material.Ks * pow( max( dot(r,v) , 0.0 ) , Material.Shininess );
   
   // Combine A & D into a single light intensity at this point on the surface
   ad = Light.Intensity * ( ambient + diffuse );
}

void main()
{
   vec3 ColourAD;  // Ambient & Diffuse shading colour result
   vec3 ColourS;   // Specular shading colour result
   ads( ColourAD , ColourS );

   vec4 texColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
   if( TexID == 0 )
	   texColour = texture( BlackSquareTex , TexCoord );
   else
	   texColour = texture( WhiteSquareTex , TexCoord );

   //vec4 texColour = texture( WaterTex  , TexCoord );
   
   // Calculate the colour from light-source shading & texture
   //FragColour = vec4( ColourAD , 1.0 ) * vec4( texColour ) + vec4( ColourS , 1.0 );
   FragColour = vec4( texColour );
   //FragColour = vec4( 1.0, 1.0, 1.0 , 1.0 );
}

