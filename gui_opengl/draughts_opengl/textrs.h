// -------------------------------------------------------
// Class for loading a fixed-width ASCII text texture &
// associated rendering functions
// -------------------------------------------------------

#ifndef _TEXTRS_H_
#define _TEXTRS_H_

#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <glm.hpp>

class CTextRS
{
   public:

      CTextRS();

      int LoadTextImageAndInitialise(const std::string filename,
                                     const GLuint      textureUnit,
                                     const GLuint      columns,
                                     const GLuint      rows,
                                     const char        _firstChar,
                                     const char        _lastChar);

      float RenderString(const std::string str);  // Returns the width of the rendered text

      void RenderPickableString(const std::string str, const glm::vec3 colour);

      void ResetTextPos();

   private:

      // Enum for the indices of vboHandleChar
      enum VBOIndices
      {
         POSITION,
         TEXCOORD,
         PICKINGCOLOUR,
         NUMINDICES
      };

      // Handles to GPU data
      GLuint textureID;
      GLuint vboHandleChar[NUMINDICES];   // Vertex Position & Texture Coordinate VBO handles
      GLuint vaoHandleChar;
      GLuint vaoHandlePicking;

      GLuint nVerticesChar;
      std::vector<float> posChar;         // Local buffer of the vertec positions for a character
      std::vector<float> texCoordChar;    // Local buffer of the texture coordinates for a character
      std::vector<float> pickingColour;   // Local buffer of the picking ID colour for a string/quad

      // Variables describing the layout of the characters within the texture
      char firstChar;
      char lastChar;
      float charWidthTexture;
      float charHeightTexture;

      // Variables describing the layout of the characters in screen space
      float charXOffsetScreen;
      float charWidthScreen;
      float charHeightScreen;

      // Map between ASCII chars & the top-left x & y texture coords
      std::map<char, float> charToXCoord;
      std::map<char, float> charToYCoord;
};



#endif // _TEXTRS_H_
