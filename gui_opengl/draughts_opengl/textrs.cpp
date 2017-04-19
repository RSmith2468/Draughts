// -------------------------------------------------------
// Class for loading a fixed-width ASCII text texture &
// associated rendering functions
// -------------------------------------------------------


#include "textrs.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include "soil.h"

#include <gtc/matrix_transform.hpp>
#include <gtx/transform2.hpp> // For glm::rotate

#include <iostream>



CTextRS::CTextRS()
:  textureID(0),
   vaoHandleChar(0),
   nVerticesChar(4),
   texCoordChar(2*nVerticesChar),
   firstChar(' '),
   lastChar(' '),
   charWidthTexture(0.0f),
   charHeightTexture(0.0f),
   charWidthScreen(0.0f),
   charHeightScreen(0.0f)
{
   for( GLuint index = 0 ; index < NUMINDICES ; index++ )
      vboHandleChar[index] = 0;

   ResetTextPos();
}


// ------------------------------------------------------------------------- //
// Function to load the text image as an OpenGL texture, work out the
// texture coordinates for each character, and initialise the VBO & VAO
// ------------------------------------------------------------------------- //
int
CTextRS::LoadTextImageAndInitialise(const std::string filename,
                                    const GLuint      textureUnit,
                                    const GLuint      columns,
                                    const GLuint      rows,
                                    const char        _firstChar,
                                    const char        _lastChar)
{
   bool bError = false;

   // --------------------------------------------------------------------- //
   // Work out the x,y texture coords for each character
   // --------------------------------------------------------------------- //

   firstChar = _firstChar;
   lastChar = _lastChar;

   // Check that the provided settings are OK
   if( columns == 0 || rows == 0 || firstChar > lastChar )
   {
      std::cerr << "Error (CTextRS::LoadTextImageAndInitialise): Invalid settings" << std::endl;
      bError = true;
   }
   else
   {
      // Work our the top-left coordinate of each character within the texture
      for(char c = firstChar ; c <= lastChar ; c++)
      {
         // First apply the char offset (so that the first char in the texturehas an index of 0)
         // Then get an integer position of the character within its row or column
         // Then normalise this position to get a tecture coordinate between 0 & 1
         float xCoord = ((c - firstChar) % columns ) / float(columns);
         //float yCoord = ((c - firstChar) / columns ) / float(rows);
         // Don't fully understand why, but the y-coords need altering
         float yCoord = 1.0f - ((c - firstChar) / columns ) / float(rows);

         charToXCoord.insert(std::pair<char, float>(c, xCoord));
         charToYCoord.insert(std::pair<char, float>(c, yCoord));

         //std::cout << "char \"" << c << "\" at coord " << xCoord << "," << yCoord << std::endl;
      }

      // Work out what the width & height of a character is in texture coordinates
      charWidthTexture = 1.0f / columns;
      charHeightTexture = 1.0f / rows;

      // Work out what the width & height of a character is in screen coordinates (i.e. normalised)
      charWidthScreen  = (columns <= rows) ? (1.0f) : (float(rows) / columns);
      charHeightScreen = (rows <= columns) ? (1.0f) : (float(columns) / rows);

      //std::cout << "Texture char dims: " << charWidthTexture << "x" << charHeightTexture << std::endl;
      //std::cout << "Screen char dims: " << charWidthScreen << "x" << charHeightScreen << std::endl;
   }

   // --------------------------------------------------------------------- //
   // Use the SOIL library to load the ASCII character image into an OpenGL
   // texture & get a handle to it
   // --------------------------------------------------------------------- //
   glActiveTexture(textureUnit);
   textureID = SOIL_load_OGL_texture
   (
      filename.c_str(),
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
   );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//GL_NEAREST);//
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_NEAREST);//

   if( textureID == 0 )
   {
      std::cerr << "Error (CTextRS::LoadTextImageAndInitialise): Failed to load texture file \"" << filename << std::endl;
      bError = true;
   }

   // --------------------------------------------------------------------- //
   // Initialise the buffers (VBO & VAO) for a single character
   // --------------------------------------------------------------------- //

   // Vertex positions
   posChar = { 0.0f           , 0.0f            , 0.0f,
               charWidthScreen, 0.0f            , 0.0f,
               0.0f           , charHeightScreen, 0.0f,
               charWidthScreen, charHeightScreen, 0.0f};

   // Just set up dummy texture coordinates for now - these will be overwritten/updated every time a character is rendered
   texCoordChar = { 0.0f            , 0.0f,
                    charWidthTexture, 0.0f,
                    0.0f            , charHeightTexture,
                    charWidthTexture, charHeightTexture };

   // Just set up dummy colours for now - these will be overwritten/updated every time a pickable-string's triangle pair is rendered
   pickingColour = { 0.0f , 0.0f , 0.0f ,
                     0.0f , 0.0f , 0.0f ,
                     0.0f , 0.0f , 0.0f ,
                     0.0f , 0.0f , 0.0f };

   // Create and populate the buffer objects
   glGenBuffers(NUMINDICES, &(vboHandleChar[0]));

   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[POSITION]);
   glBufferData(GL_ARRAY_BUFFER, posChar.size() * sizeof(float), &(posChar[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[TEXCOORD]);
   glBufferData(GL_ARRAY_BUFFER, texCoordChar.size() * sizeof(float), &(texCoordChar[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[PICKINGCOLOUR]);
   glBufferData(GL_ARRAY_BUFFER, pickingColour.size() * sizeof(float), &(pickingColour[0]), GL_DYNAMIC_DRAW); // Will be updated with each string/quad that is rendered, so DYNAMIC_DRAW

   // Create the VAO for character rendering
   glGenVertexArrays( 1, &vaoHandleChar );
   glBindVertexArray(vaoHandleChar);

   glEnableVertexAttribArray(POSITION);  // Vertex position
   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[POSITION]);
   glVertexAttribPointer( GLuint(0), 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(TEXCOORD);  // Texture coords
   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[TEXCOORD]);
   glVertexAttribPointer( GLuint(1), 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   // Create the VAO for string picking rendering
   glGenVertexArrays( 1, &vaoHandlePicking );
   glBindVertexArray(vaoHandlePicking);

   glEnableVertexAttribArray(0);  // Vertex position
   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[POSITION]);
   glVertexAttribPointer( GLuint(0), 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(1);  // Picking ID Colour
   glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[PICKINGCOLOUR]);
   glVertexAttribPointer( GLuint(1), 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glBindVertexArray(0);

   return bError;
}


// ------------------------------------------------------------------------- //
// Function to render each character in a given string (2 triangles per
// character) - updates the vertex positions & texture coordinate VBOs every
// character
// ------------------------------------------------------------------------- //
float
CTextRS::RenderString(const std::string str)
{
   // Loop through each character in the string
   for( GLuint index = 0 ; index < str.size() ;  index++ )
   {
      char c = str[index];
      // Check that it is within the range of printable characters
      if( (c >= firstChar) && (c <= lastChar) )
      {
         // Update the vertex positions
         posChar = { charXOffsetScreen+0.0f           , 0.0f            , 0.0f,
                     charXOffsetScreen+charWidthScreen, 0.0f            , 0.0f,
                     charXOffsetScreen+0.0f           , charHeightScreen, 0.0f,
                     charXOffsetScreen+charWidthScreen, charHeightScreen, 0.0f};

         charXOffsetScreen += charWidthScreen;

         glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[POSITION]);
         glBufferData(GL_ARRAY_BUFFER, posChar.size() * sizeof(float), &(posChar[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

         // Update the texture coordinates
         float xLeftTex = charToXCoord[c];
         float yTopTex = charToYCoord[c];

         //texCoordChar = { xLeftTex                 , yTopTex,
         //                 xLeftTex+charWidthTexture, yTopTex,
         //                 xLeftTex                 , yTopTex+charHeightTexture,
         //                 xLeftTex+charWidthTexture, yTopTex+charHeightTexture };
         // Don't fully understand why, but the y-coords need altering (see the corresponding change to the "float yCoord = " line in the LoadTextImageAndInitialise function
         texCoordChar = { xLeftTex                 , yTopTex-charHeightTexture,
                          xLeftTex+charWidthTexture, yTopTex-charHeightTexture,
                          xLeftTex                 , yTopTex,
                          xLeftTex+charWidthTexture, yTopTex };

         glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[TEXCOORD]);
         glBufferData(GL_ARRAY_BUFFER, texCoordChar.size() * sizeof(float), &(texCoordChar[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

         // Render the character
         glBindVertexArray(vaoHandleChar);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, nVerticesChar);
      }
   }

   return charWidthScreen*str.size();
}


// ------------------------------------------------------------------------- //
// Function to render a single quad (2 triangles) to cover the entire render
// area of a given string, and colour the quad with a given ID colour
// ------------------------------------------------------------------------- //
void
CTextRS::RenderPickableString(const std::string str, const glm::vec3 colour)
{
   // Calculate the dimensions of the quad to render based on the size of the string
   float quadWidth = charWidthScreen * str.size();

   // Only render if the string has non-zero length
   if( quadWidth > 0.0f )
   {
      // Update the vertex positions
      posChar = { charXOffsetScreen+0.0f      , 0.0f             , 0.0f,
                  charXOffsetScreen+quadWidth , 0.0f             , 0.0f,
                  charXOffsetScreen+0.0f      , charHeightScreen , 0.0f,
                  charXOffsetScreen+quadWidth , charHeightScreen , 0.0f};

      charXOffsetScreen += quadWidth;

      glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[POSITION]);
      glBufferData(GL_ARRAY_BUFFER, posChar.size() * sizeof(float), &(posChar[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

      // Update the picking colour
      pickingColour = { colour.r , colour.g , colour.b ,
                        colour.r , colour.g , colour.b ,
                        colour.r , colour.g , colour.b ,
                        colour.r , colour.g , colour.b };

      glBindBuffer(GL_ARRAY_BUFFER, vboHandleChar[PICKINGCOLOUR]);
      glBufferData(GL_ARRAY_BUFFER, pickingColour.size() * sizeof(float), &(pickingColour[0]), GL_DYNAMIC_DRAW); // Will be updated with each char that is rendered, so DYNAMIC_DRAW

      // Render the pickable string
      glBindVertexArray(vaoHandlePicking);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, nVerticesChar);
   }
}

void
CTextRS::ResetTextPos()
{
   charXOffsetScreen=0.0f;
}


