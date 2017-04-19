#ifndef SCENE_H
#define SCENE_H

//#include "scene.h"
#include "glslprogram.h"
#include "vbocylinder.h"
#include "textrs.h"

#include "../../board.h"
#include "ipickable.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm.hpp>
//using glm::mat4;

class Scene// : public Scene
{
private:
   CGLSLProgram prog_board;
   CGLSLProgram prog_piece;
   CGLSLProgram prog_highlight;
   CGLSLProgram prog_board_picking;
   CGLSLProgram prog_text;
   //CGLSLProgram prog_pickable_object;

   // Width & Height of the window (in pixels)
   int width, height;
   // Target aspect ratio (width/height) of what is to be rendered
   float widthOverHeightRatio;

   // VBOs for everything that needs rendering
   GLuint vaoHandleBoard;  // VAO handle for the board (vertex pos, normal, tex coord, tex ID)
   GLuint vaoHandleBoardPicking;  // VAO handle for the board when picking (only need the vertex pos & obj IDs)
   GLuint nVertsBoard;     // Number of vertices of the board

   GLuint vaoHandleHighlight; // VAO handle for the highlight lines around a selected square
   GLuint nVertsHighlight;    // Number of vertices of the highlight lines

   VBOCylinder *piece;     // The VBO for the checkers pieces

   // Texture IDs
   GLuint texIDSquareBlack, texIDSquareWhite, texIDPiece, texIDCrown;

   // Variables for the picking Frame Buffer Object
   //GLuint fboHandlePicking;
   //GLuint texIDPicking;

   int RGBIntBits;
   glm::vec3 IDtoRGBf(const int id);
   int RGBtoIDf(const glm::vec3 rgb);
   int RGBtoIDi(const int r, const int g, const int b);
   // Picking ID colours for text options
   glm::vec3 idColourReset;
   glm::vec3 idColourOpenMenu;
   glm::vec3 idColourCloseMenu;
   glm::vec3 idColourAIPModerate;
   glm::vec3 idColourAIPAggressive;
   glm::vec3 idColourAIPGenerous;
   glm::vec3 idColourAIPStupid;
   glm::vec3 idColourAIPRandom;
   glm::vec3 idColourAISide;
   glm::vec3 idColourFirstTurn;
   glm::vec3 idColourLayout;

   // Storage for all of the pickable objects, where the index is the object ID
   std::vector<IPickable*> pickableObjects;
   // Function that links an object ID with an action on the board
   void ExecuteObject(const int id);

   int mouseX, mouseY;  // Mouse coordinates for picking
   int windowHeight;    // Height of the window, which is not necessarily the height of the rendered area
   bool bClicked;       // set true when a mouse click is registered; set false after Scene has processed the mouse click

   // The checkers board
   CBoard board;

   int aiPersonality;   // The AI's personality defines the metric used in determining a branch's score
   int aiIntelligence;  // The AI's intelligence is the depth of the search tree that it will evaluate
   bool isNewGame;      // Bool to trigger asking for the game initialisation options
   bool singlePlayer;   // bool to signal whether it is a 1-player or 2-player game
   bool aiIsX;          // bool to signal which side the AI is controlling (if 1-player game)
   int aiSideAtReset;   // Which side should the AI be set to upon board reset: 0 = O , 1 = X , 2 = random
   int firstTurnSide;   // Which side has the first turn: 0 = O , 1 = X , 2 = random
   bool boardLayout;    // Layout of the board

   float timeAIStart;   // Counter to wait for 1s before making the AI's move
   bool aiHasDecided;

   glm::vec3 colourX;	// Colour of the X pieces
   glm::vec3 colourO;	// Colour of the O pieces

   // Text Rendering class
   CTextRS text;
   std::string sAIType;
   std::string sAISide;
   std::string sTurn;
   std::string sGameOver;

   // Standard matrices
   glm::mat4 model, view, projection;

   float angle;
   float anglePerSecond;
   float time;
   float fps;

   void setMatrices();
   void compileAndLinkShader();
   void initBuffers();

   // Rendering functions & functions for controlling the switching between them
   void RenderMenu();
   void RenderBoard();

   enum RenderableScenes
   {
      BOARD,
      MENU
   };
   int sceneToRender;   // Should take one of the values of RenderableScenes

public:
   Scene();
   ~Scene();

   void initScene();
   void update( float t );
   void render();
   void resize(int, int);

   // Not sure if these should be private
   void ResetBoard();
   // Rendering functions & functions for controlling the switching between them
   void OpenMenu();
   void CloseMenu();
   void SetPersonality(int _aiPersonality);
   void ToggleFirstTurn();
   void ToggleAISide();
   void ToggleLayout();

   // Functions to process user-input
   void UserInputUp()      { if( !aiHasDecided ) board.MoveSelectSquareUp(); }
   void UserInputDown()    { if( !aiHasDecided ) board.MoveSelectSquareDown(); }
   void UserInputLeft()    { if( !aiHasDecided ) board.MoveSelectSquareLeft(); }
   void UserInputRight()   { if( !aiHasDecided ) board.MoveSelectSquareRight(); }
   void UserInputSelect()  { if( !aiHasDecided ) board.ExecuteSelectedSquare(); }

   void ClickLocation(const int _mouseX, const int _mouseY)
   {
      if( bClicked == false ) // Only do something if we are not currently processing a mouse click
      {
         mouseX = _mouseX;
         mouseY = windowHeight - _mouseY;
         bClicked = true;
      }
   }
};

#endif // SCENE_H
