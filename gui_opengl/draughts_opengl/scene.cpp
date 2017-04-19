#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <cmath>

#include <string>
#include <vector>

#include "defines.h"
//#include "glutils.h"

#include "../../randomrs.h"
#include "pickable_draughts_square.h"
#include "pickable_draughts_item.h"

#include "soil.h"

//using glm::vec3;

#include <gtc/matrix_transform.hpp>
//#include <gtx/transform2.hpp>
//#include <gtc/matrix_projection.hpp>
//#include <gtx/matrix_transform_2d.hpp> // For glm::rotate
#include <gtx/transform2.hpp> // For glm::rotate

#include "scene.h"

//#define FPS_COUNTER

Scene::Scene()
: width(600), height(750),
  vaoHandleBoard(0),
  vaoHandleBoardPicking(0),
  nVertsBoard(0),
  vaoHandleHighlight(0),
  nVertsHighlight(0),
  piece(0),
  texIDSquareBlack(0), texIDSquareWhite(0), texIDPiece(0), texIDCrown(0),
  //fboHandlePicking(0), texIDPicking(0),
  RGBIntBits(8),
  idColourReset(),
  mouseX(0), mouseY(0), bClicked(false),
  aiIntelligence(5),
  isNewGame(true),
  singlePlayer(true),
  aiIsX(true),
  aiSideAtReset(2),
  firstTurnSide(2),
  boardLayout(1),
  timeAIStart(0.0f),
  aiHasDecided(false),
  angle(0.0f),
  anglePerSecond(2*PI),
  time(0.0f)
{
   windowHeight = height;
   glViewport(0,0,width,height);

   widthOverHeightRatio = 600.0f/750.0f;  // Set the target width/height ratio here

   sTurn = "'s Turn";
   sGameOver = " Wins!";

   colourX = glm::vec3(1.0f, 0.5f, 0.5f);
   colourO = glm::vec3(0.5f, 0.5f, 1.0f);

   ResetBoard();
}

Scene::~Scene()
{
   // Deallocate all of the pickable objects
   for( unsigned int i = 0 ; i < pickableObjects.size() ; i++ )
   {
      if( pickableObjects[i] != 0 )
      {
         delete pickableObjects[i];
         pickableObjects[i] = 0;
      }
   }

   // De-allocate the draughts piece
   if( piece != 0 )
   {
      delete piece;
      piece = 0;
   }
}

void
Scene::SetPersonality(int _aiPersonality)
{
   aiPersonality = _aiPersonality;
}

void
Scene::ToggleFirstTurn()
{
   firstTurnSide = (firstTurnSide+1)%3;
}

void
Scene::ToggleAISide()
{
   aiSideAtReset = (aiSideAtReset+1)%3;
}

void
Scene::ToggleLayout()
{
   boardLayout = !boardLayout;
   board.SetLayoutAndReset(boardLayout);
}

void Scene::ResetBoard()
{
   // Set the board up with some random parameters
   // random number generator to use later in the program
   CRandomRS rng;
   //rng.UseSeed(1074);

   int aiPersonalityToUse = aiPersonality;
   if( aiPersonality > 3 )
   {
      // Randomly select an AI personality
      aiIntelligence = 5;
      rng.SetRange(0,3);  // will generate a number between 0 and 3 inclusive
      aiPersonalityToUse = rng.GetNumber();
   }
   switch (aiPersonalityToUse)
   {
      case 0:
         board.SetAIModerate();
         sAIType = "Moderate";
         break;
      case 1:
         board.SetAIAggressive();
         sAIType = "Aggressive";
         break;
      //case 2:
      //   board.SetAICautious();
      //   break;
      case 2:
         board.SetAIGenerous();
         sAIType = "Generous";
         break;
      default: //case 3:
         board.SetAIModerate();
         aiIntelligence = 0;
         sAIType = "Stupid";
         break;
   }
   std::cout << sAIType << std::endl;

   // Select which side goes first
   if( firstTurnSide == 0 || firstTurnSide == 1 )
   {
      board.ForceTurn( firstTurnSide );
   }
   else
   {
      // Randomly select which side goes first & which side is the AI
      rng.SetRange(0,1);  // will generate a number between 0 and 1 inclusive
      board.ForceTurn( rng.GetNumber() );// Generate a random number & convert it to a bool for the turn
   }

   // Select which side is the AI
   if( aiSideAtReset == 0 || aiSideAtReset == 1 )
   {
      aiIsX = aiSideAtReset;
   }
   else
   {
      rng.SetRange(0,1);  // will generate a number between 0 and 1 inclusive
      aiIsX = rng.GetNumber();  // Generate a random number & convert it to a bool for which side is the AI
   }

   // Set up some strings that will be printed to the screen
   sAISide = "AI is ";
   //sAISide += (aiIsX?"Red":"Blue");

   // Get the current board layout (so that the rendering of the mini-board in the options menu is correct)
   boardLayout = board.Layout();
}

void Scene::initScene()
{
   compileAndLinkShader();

   //glClearColor(0.0f,0.0f,0.0f,1.0f);

   //glDisable(GL_DEPTH_TEST);
   glEnable(GL_DEPTH_TEST);

   // Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   angle = PI / 3.0;

   // Generate the vertex buffers
   initBuffers();

   // --------------------------------------------------------------------- //
   // Load the textures & set the uniforms for the board shaders
   // --------------------------------------------------------------------- //

   prog_board.Use();
   // Use the SOIL library to load the image into an OpenGL texture & get a handle to it
   std::string texName("../../textures/piece_white.png");

   glActiveTexture(GL_TEXTURE0);
   texIDSquareBlack = SOIL_load_OGL_texture
   (
      texName.c_str(),
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
   );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // Set the Tex1 sampler uniform to refer to texture unit 0, if the texture was created correctly
   if( texIDSquareBlack == 0 )
      std::cerr << "Error (Scene::initScene): Failed to load texture file \"" << texName << "\"\n";
   else
      prog_board.SetUniform("BlackSquareTex", 0);

   // Use the SOIL library to load the image into an OpenGL texture & get a handle to it
   texName = "../../textures/marble_white.png";

   glActiveTexture(GL_TEXTURE1);
   texIDSquareWhite = SOIL_load_OGL_texture
   (
      texName.c_str(),
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
   );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // Set the Tex1 sampler uniform to refer to texture unit 0, if the texture was created correctly
   if( texIDSquareWhite == 0 )
      std::cerr << "Error (Scene::initScene): Failed to load texture file \"" << texName << "\"\n";
   else
      prog_board.SetUniform("WhiteSquareTex", 1);

   // Set the light properties
   prog_board.SetUniform("Light.Position", glm::vec4(0.0f,0.0f,0.0f,1.0f) );
   prog_board.SetUniform("Light.Intensity", glm::vec3(1.0f,1.0f,1.0f) );

   // Set the material reflectivity for the board
   prog_board.SetUniform("Material.Kd", 0.7f, 0.7f, 0.7f);
   prog_board.SetUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
   prog_board.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
   prog_board.SetUniform("Material.Shininess", 180.0f);

   // --------------------------------------------------------------------- //
   // Load the textures & set the uniforms for the piece shaders
   // --------------------------------------------------------------------- //

   prog_piece.Use();
   piece = new VBOCylinder(24);
   // Use the SOIL library to load the image into an OpenGL texture & get a handle to it
   texName = "../../textures/piece_white.png";

   glActiveTexture(GL_TEXTURE2);
   texIDPiece = SOIL_load_OGL_texture
   (
      texName.c_str(),
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
   );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   // Set the Tex1 sampler uniform to refer to texture unit 0, if the texture was created correctly
   if( texIDPiece == 0 )
      std::cerr << "Error (Scene::initScene): Failed to load texture file \"" << texName << "\"\n";
   else
      prog_piece.SetUniform("PieceTex", 2);

   // Use the SOIL library to load the image into an OpenGL texture & get a handle to it
   texName = "../../textures/crown_32.png";

   glActiveTexture(GL_TEXTURE3);
   texIDCrown = SOIL_load_OGL_texture
   (
      texName.c_str(),
      SOIL_LOAD_AUTO,
      SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
   );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);// GL_LINEAR); //
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);// GL_LINEAR); //

   // Set the Tex1 sampler uniform to refer to texture unit 0, if the texture was created correctly
   if( texIDCrown == 0 )
      std::cerr << "Error (Scene::initScene): Failed to load texture file \"" << texName << "\"\n";
   else
      prog_piece.SetUniform("CrownTex", 3);

   // Set the light properties
   prog_piece.SetUniform("Light.Position", glm::vec4(0.0f,0.0f,0.0f,1.0f) );
   prog_piece.SetUniform("Light.Intensity", glm::vec3(1.0f,1.0f,1.0f) );

   // Set the material reflectivity for the board
   //prog_piece.SetUniform("Material.Kd", 0.7f, 0.7f, 0.7f);
   prog_piece.SetUniform("Material.Kd", 1.0f, 0.5f, 0.5f);
   //prog_piece.SetUniform("Material.Kd", 0.5f, 0.5f, 1.0f);
   prog_piece.SetUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
   prog_piece.SetUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
   prog_piece.SetUniform("Material.Shininess", 180.0f);

   // --------------------------------------------------------------------- //
   // Set the line width for the highlight boxes
   // --------------------------------------------------------------------- //
   glLineWidth(3.0f);

   // --------------------------------------------------------------------- //
   // Load the text texture & set up the VBO/VAO for text rendering
   // --------------------------------------------------------------------- //

   bool bTextError = text.LoadTextImageAndInitialise("../../textures/courier_new_512x512_alpha_greyscale_whitebg.png",
                                                     GL_TEXTURE4, // textureUnit,
                                                     16,    // columns,
                                                     8,     // rows,
                                                     ' ',   // _firstChar,
                                                     '~');  // _lastChar

   prog_text.Use();

   if( bTextError )
      std::cerr << "Error (Scene::initScene): Failed to correctly initialise the text" << std::endl;
   else
      prog_text.SetUniform("TextTexture", 4);


   // --------------------------------------------------------------------- //
   // Initialise the picking ID colours and pickable objects
   // --------------------------------------------------------------------- //

   // Add a text pickable object for the reset & menu buttons
   idColourReset = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::RESET));

   idColourOpenMenu = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::OPEN_MENU));

   idColourCloseMenu = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::CLOSE_MENU));

   idColourAIPModerate = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AIP_MODERATE));
   idColourAIPAggressive = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AIP_AGGRESSIVE));
   idColourAIPGenerous = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AIP_GENEROUS));
   idColourAIPStupid = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AIP_STUPID));
   idColourAIPRandom = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AIP_RANDOM));

   idColourAISide = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::AI_SIDE));

   idColourFirstTurn = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::FIRST_TURN));

   idColourLayout = IDtoRGBf(pickableObjects.size());
   pickableObjects.push_back(new NSPickableDraughtsItem::CPickableDraughtsItem(*this, NSPickableDraughtsItem::LAYOUT));


   // --------------------------------------------------------------------- //
   // Set up the FBO & texture for picking
   // --------------------------------------------------------------------- //
/*
   // Generate & bind the frame buffer
   glGenFramebuffers(1, &fboHandlePicking);
   glBindFramebuffer(GL_FRAMEBUFFER, fboHandlePicking);

   // Create the texture object
   glGenTextures(1, &texIDPicking);            // generate texture object
   glActiveTexture(GL_TEXTURE4);                  // set texture unit 0 to be active
   glBindTexture(GL_TEXTURE_2D, texIDPicking); // bind renderTex texture object to a GL_TEXTURE_2D target
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // Allocate memory for a 512x512 texture, but leave the memory uninitialised (NULL)
   // Set the minimisation & maximisation filters to be nearest neighbour (don't want interpolation of the object IDs)
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   //Bind the texture to the FBO
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texIDPicking, 0);
   //                     |               |                     |              |          mip-map level of the texture (1 level = 0)
   //                     |               |                     |              handle to the texture
   //                     |               |                     texture target
   //                     |               attachment point of the FBO
   //                     attach texture to the FBO currently bound to the GL_FRAMEBUFFER target

   // Create the depth buffer (to allow rendering to the FBO with depth testing)
   GLuint depthBuf;
   glGenRenderbuffers(1, &depthBuf);
   glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
   //                                     > internal format for the buffer

   // Bind the depth buffer to the FBO
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

   // Set the targets for the fragment output variables
   GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
   glDrawBuffers(1, drawBuffers);
   //            |  attachment point (of the FBO)
   //            1 fragment shader output (FragColour)

   // Unbind the framebuffer, and revert to default framebuffer
   glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
}


void Scene::initBuffers()
{
   // --------------------------------------------------------------------- //
   // Initialise the buffers (VBO & VAO) for the board
   // --------------------------------------------------------------------- //
   const unsigned int boardWidth = board.Width();
   const unsigned int boardHeight = board.Height();

   // Reserve memory for the vector of pickable objects (as we'll be using push_back on the vector)
   pickableObjects.reserve(boardWidth*boardHeight);
   int numPickableObjects = 0; // Each square of the board is pickable, so increment this when assigning an ID colour in the VBO

   nVertsBoard = boardWidth * boardHeight * 4;   // 8x8 grid, 4 vertices per square

   // Vertices
   std::vector<float> v(3 * nVertsBoard);
   // Normals
   std::vector<float> n(3 * nVertsBoard);
   // Texture coordinates
   std::vector<float> tex(2 * nVertsBoard);
   // Texture ID
   std::vector<GLuint> texID(1 * nVertsBoard);
   // Object ID to be used when picking (3 colours)
   std::vector<float> objID(3 * nVertsBoard);

   unsigned int indexVert = 0;
   unsigned int indexNorm = 0;
   unsigned int indexTex = 0;
   unsigned int indexTexID = 0;
   unsigned int indexObjID = 0;
   // Loop through each square in the grid
   for( unsigned int y = 0 ; y < boardHeight ; y++ )
      for( unsigned int x = 0 ; x < boardWidth ; x++ )
      {
         // Each square has a single ID colour
         glm::vec3 objColour = IDtoRGBf(numPickableObjects++);
         pickableObjects.push_back(new CPickableDraughtsSquare(board, x, y));
         // Loop through each vertex of the square
         for( unsigned int dy = 0 ; dy < 2 ; dy++ )
            //for( unsigned int dx = 0 ; dx < 2 ; dx++ )
            for( unsigned int dx = (dy==0?0:1) ; dx < 2 ; (dy==0?dx++:dx--) )
            {
               v[indexVert++] = float(x + dx)/boardWidth;
               v[indexVert++] = float(y + dy)/boardHeight;
               //v[indexVert++] = float(x + dx)*boardWidth;
               //v[indexVert++] = float(y + dy)*boardHeight;
               v[indexVert++] = 0.0f;

               // All normals are pointing up
               n[indexNorm++] = 0.0f;
               n[indexNorm++] = 0.0f;
               n[indexNorm++] = 1.0f;

               tex[indexTex++] = float(dx);
               tex[indexTex++] = float(dy);

               texID[indexTexID++] = ((x + y) & 1) == 0 ? 0 : 1;
               objID[indexObjID++] = objColour.r;
               objID[indexObjID++] = objColour.g;
               objID[indexObjID++] = objColour.b;
            }
      }

   // Create and populate the buffer objects
   GLuint handle[5];
   glGenBuffers(5, handle);

   glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
   glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), &(v[0]), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
   glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(float), &(n[0]), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
   glBufferData(GL_ARRAY_BUFFER, tex.size() * sizeof(float), &(tex[0]), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, handle[3]);
   glBufferData(GL_ARRAY_BUFFER, texID.size() * sizeof(GLuint), &(texID[0]), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, handle[4]);
   glBufferData(GL_ARRAY_BUFFER, objID.size() * sizeof(float), &(objID[0]), GL_STATIC_DRAW);

   // Create the VAO for the rendering pass of the board
   glGenVertexArrays( 1, &vaoHandleBoard );
   glBindVertexArray(vaoHandleBoard);

   glEnableVertexAttribArray(0);  // Vertex position
   glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
   glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(1);  // Vertex normal
   glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
   glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(2);  // Texture coords
   glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
   glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(3);  // Texture ID
   glBindBuffer(GL_ARRAY_BUFFER, handle[3]);
   glVertexAttribPointer( (GLuint)3, 1, GL_UNSIGNED_INT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   // Create the VAO for the picking pass of the board
   glGenVertexArrays( 1, &vaoHandleBoardPicking );
   glBindVertexArray(vaoHandleBoardPicking);

   glEnableVertexAttribArray(0);  // Vertex position
   glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
   glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

   glEnableVertexAttribArray(1);  // Object ID Colour
   glBindBuffer(GL_ARRAY_BUFFER, handle[4]);
   glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );


   glBindVertexArray(0);


   // --------------------------------------------------------------------- //
   // Initialise the buffers (VBO & VAO) for the highlight lines
   // --------------------------------------------------------------------- //
   nVertsHighlight = 5;  // 4 lines = 5 points

   // Vertex positions
   std::vector<float> posHighlight = { 0.0f, 0.0f, 0.0f,
                                       1.0f, 0.0f, 0.0f,
                                       1.0f, 1.0f, 0.0f,
                                       0.0f, 1.0f, 0.0f,
                                       0.0f, 0.0f, 0.0f};

   // Create and populate the buffer objects
   GLuint handleHighlight;
   glGenBuffers(1, &handleHighlight);

   glBindBuffer(GL_ARRAY_BUFFER, handleHighlight);
   glBufferData(GL_ARRAY_BUFFER, posHighlight.size() * sizeof(float), &(posHighlight[0]), GL_STATIC_DRAW);

   // Create the VAO
   glGenVertexArrays( 1, &vaoHandleHighlight );
   glBindVertexArray(vaoHandleHighlight);

   glEnableVertexAttribArray(0);  // Vertex position
   glBindBuffer(GL_ARRAY_BUFFER, handleHighlight);
   glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)(NULL) );

   glBindVertexArray(0);

}

void Scene::update( float t )
{
   angle += (t - time)*anglePerSecond;
   if( angle > 2*PI ) angle -= 2*PI;
#ifdef FPS_COUNTER
   fps = (t == time) ? 999.0f : 1.0f/(t-time);
   if( fps > 999.0f ) fps = 999.0f;
#endif
   time = t;
   //time += t;
}

void Scene::render()
{
   switch(sceneToRender)
   {
      case BOARD:
      {
         RenderBoard();
         break;
      }
      default: //case MENU:
      {
         RenderMenu();
         break;
      }
   }
}


void
Scene::RenderBoard()
{
   glEnable(GL_DEPTH_TEST);

   // Invoke the AI if necessary
   // First get the AI to work out the best move, then after 1 second make the move
   if( !aiHasDecided && singlePlayer && (board.IsXTurn() == aiIsX) && board.CurrentSideHasMoves() )
   {
      /*aiSuccess = */board.InvokeAI( /*isXTurn ,*/ aiIntelligence );
      timeAIStart = time;
      aiHasDecided = true;
   }
   if( aiHasDecided && (time-timeAIStart > 0.5f) )
   {
      board.ExecuteSelectedSquare( /*isXTurn*/ );
      aiHasDecided = false;
   }

   // Calculate scaling factors
   float scaleX = 1.0f/board.Width();
   float scaleY = 1.0f/board.Height();
   float scaleZ = 1.0f/((board.Width()+board.Height())/2.0f);

   //view = glm::lookAt(glm::vec3(3.0f * cos(angle),1.5f,3.0f * sin(angle)), glm::vec3(0.0f,1.5f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
   view = glm::ortho(0.0f, 1.0f, 0.0f, 1.25f);
   model = glm::mat4(1.0f);

   // Define the position & scale of the pickable reset text
   glm::vec3 resetTextTranslate = glm::vec3(0.75f, 1.0f, 0.1f);
   glm::vec3 resetTextScale = glm::vec3(0.0625, 0.0625, 1.0f);
   std::string sReset("Reset");

   // Define the position & scale of the pickable menu text
   glm::vec3 menuTextTranslate = glm::vec3(0.75f, 1.1f, 0.1f);
   glm::vec3 menuTextScale = glm::vec3(0.0625, 0.0625, 1.0f);
   std::string sOpenMenu("Menu");

   // --------------------------------------------------------------------- //
   // Draw the board for picking
   // --------------------------------------------------------------------- //
   // Get the pixel colour under the mouse position
   if( bClicked == true )
   {
      // Enable the picking framebuffer
      //glBindFramebuffer(GL_FRAMEBUFFER, fboHandlePicking);
      // Set the background colour to pure white (i.e. the max object ID, which is a special ID interpreted as "do nothing")
      glClearColor(1.0f,1.0f,1.0f,1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      prog_board_picking.Use();

      // Draw the board squares, but only if it is the player's turn
      if( (!singlePlayer || (singlePlayer && (board.IsXTurn() != aiIsX))) ) // Only allow picking if it's not the AI's turn
      {
         //model = glm::mat4(1.0f);
         setMatrices();
         glBindVertexArray(vaoHandleBoardPicking);
         //glDrawElements(GL_TRIANGLES, nVerts, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
         //glDrawArrays(GL_TRIANGLE_STRIP, 0, nVerts);
         glDrawArrays(GL_QUADS, 0, nVertsBoard);
      }

      // Draw the reset text (always allow the player to reset the game - even during the AI's turn)
      model = glm::mat4(1.0f);
      model *= glm::translate( resetTextTranslate );  // Bottom-left corner of the text to be rendered
      model *= glm::scale( resetTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderPickableString( sReset , idColourReset );

      // Draw the menu text (always allow the player to modify the options (& therefore reset the game) - even during the AI's turn)
      model = glm::mat4(1.0f);
      model *= glm::translate( menuTextTranslate );  // Bottom-left corner of the text to be rendered
      model *= glm::scale( menuTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderPickableString( sOpenMenu , idColourOpenMenu );

      // Wait until all the pending drawing commands are really done.
      // There are usually a long time between glDrawElements() and
      // all the fragments completely rasterized.
      //glFlush();
      //glFinish();

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      unsigned char data[4];  // The pixel values will be 8-bit integer values
      glReadPixels(mouseX, mouseY,1,1, GL_RGBA, GL_UNSIGNED_BYTE, data);
      int pickedObjectID = RGBtoIDi(data[0], data[1], data[2]);

      ExecuteObject(pickedObjectID);

      bClicked = false;
   }

   // Bind the default framebuffer for rendering
   //glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glClearColor(0.5f,0.5f,0.5,1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // --------------------------------------------------------------------- //
   // Draw the board
   // --------------------------------------------------------------------- //
   prog_board.Use();
   model = glm::mat4(1.0f);
   setMatrices();
   glBindVertexArray(vaoHandleBoard);
   //glDrawElements(GL_TRIANGLES, nVerts, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
   //glDrawArrays(GL_TRIANGLE_STRIP, 0, nVerts);
   glDrawArrays(GL_QUADS, 0, nVertsBoard);

   // --------------------------------------------------------------------- //
   // Draw the pieces
   // --------------------------------------------------------------------- //
   prog_piece.Use();
   for(unsigned int y = 0 ; y < board.Height() ; y++)
      for(unsigned int x = 0 ; x < board.Width() ; x++)
      {
         model = glm::mat4(1.0f);
         if( !(board.SquareIsEmpty(x,y)) )
         {
            model *= glm::translate( glm::vec3(x/float(board.Width()), y/float(board.Height()), 0.0f) );
            model *= glm::scale( glm::vec3(scaleX, scaleY, scaleZ) );
            setMatrices();

            if( board.SquareContainsXPiece(x,y) )
            {
               prog_piece.SetUniform("Material.Kd", colourX);
            }
            else
            {
               prog_piece.SetUniform("Material.Kd", colourO);
            }

            if( board.SquareContainsManPiece(x,y) )
               prog_piece.SetUniform("IsCrownPiece", 0);
            else
               prog_piece.SetUniform("IsCrownPiece", 1);

            piece->render();
         }
      }
   //exit(0);
   //model *= glm::translate( glm::vec3(1/8.0f, 1/8.0f, 0.0f) );
   //model *= glm::scale( glm::vec3(1/8.0f, 1/8.0f, 1/8.0f) );
   ////model *= glm::rotate( 0.2f, glm::vec3(1.0f, 0.0f, 0.0f) );
   //setMatrices();
   //piece->render();

   // --------------------------------------------------------------------- //
   // Highlight the selected & queued squares
   // --------------------------------------------------------------------- //
   prog_highlight.Use();
   //setMatrices();
   glBindVertexArray(vaoHandleHighlight);
   // Get the location of the squares to highlight
   int selectedX = board.SelectedX();
   int selectedY = board.SelectedY();
   int queuedX = board.QueuedX();
   int queuedY = board.QueuedY();

   // Offset the highlight squares just above the board
   float highlightZOffset = 0.1;

   // Draw a green box around the selected square
   if( selectedX >=0 && selectedX < int(board.Width()) && selectedY >=0 && selectedY < int(board.Height()) )
   {
      prog_highlight.SetUniform("LineColour", glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));
      model = glm::mat4(1.0f);
      model *= glm::translate( glm::vec3(selectedX/float(board.Width()), selectedY/float(board.Height()), highlightZOffset) );
      model *= glm::scale( glm::vec3(scaleX, scaleY, scaleZ) );
      setMatrices();
      glDrawArrays(GL_LINE_STRIP, 0, nVertsHighlight);
   }
   // Draw a blue box around the queued square
   if( queuedX >=0 && queuedX < int(board.Width()) && queuedY >=0 && queuedY < int(board.Height()) )
   {
      prog_highlight.SetUniform("LineColour", glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
      model = glm::mat4(1.0f);
      model *= glm::translate( glm::vec3(queuedX/float(board.Width()), queuedY/float(board.Height()), highlightZOffset) );
      model *= glm::scale( glm::vec3(scaleX, scaleY, scaleZ) );
      setMatrices();
      glDrawArrays(GL_LINE_STRIP, 0, nVertsHighlight);
   }
   //glDrawArrays(GL_LINE_STRIP, 0, nVertsHighlight);

   // --------------------------------------------------------------------- //
   // Render Text
   // --------------------------------------------------------------------- //

   // Title
   prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   prog_text.SetUniform("foregroundColour", glm::vec3(0.0f, 0.0f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.0f, 1.125f, 1.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.125, 0.125, 1.0f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString("Draughts");

   // AI Info
   //prog_text.Use();
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.0f, 1.0625f, 1.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.0625, 0.0625, 1.0f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   float aiTextOffset = text.RenderString(sAISide);

   prog_piece.Use();
   if( aiIsX )
      prog_piece.SetUniform("Material.Kd", colourX);
   else
      prog_piece.SetUniform("Material.Kd", colourO);
   prog_piece.SetUniform("IsCrownPiece", 0);
   model *= glm::translate( glm::vec3(aiTextOffset, 0.0f, 0.0f) );  // Offset the piece to the right of the AI text
   setMatrices();
   piece->render();

   // Current Turn Info
   //prog_piece.Use();
   if( board.IsXTurn() )
      prog_piece.SetUniform("Material.Kd", colourX);
   else
      prog_piece.SetUniform("Material.Kd", colourO);
   prog_piece.SetUniform("IsCrownPiece", 0);
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.0f, 1.0f, 1.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.0625, 0.0625, 1.0f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   piece->render();

   prog_text.Use();
   model *= glm::translate( glm::vec3(1.0f, 0.0f, 0.0f) );  // Bottom-left corner of the text to be rendered - to the immediate right of the piece
   setMatrices();
   text.ResetTextPos();
   text.RenderString( sTurn );

   // Winner info
   if( !(board.CurrentSideHasMoves()) )
   {
      prog_piece.Use();
      if( board.IsXTurn() )
         prog_piece.SetUniform("Material.Kd", colourO);
      else
         prog_piece.SetUniform("Material.Kd", colourX);
      prog_piece.SetUniform("IsCrownPiece", 1);

      model = glm::mat4(1.0f);
      model *= glm::translate( glm::vec3(0.0f, 0.5f, 0.75f) );    // Bottom-left corner of the piece
      model *= glm::scale( glm::vec3(0.2f, 0.2f, 0.2f) );         // Piece is 1.0f x 1.0f, so must be scaled down to a suitable size
      model *= glm::translate( glm::vec3(0.5f, 0.5f, 0.5f) );     // Shift the axis of rotation to be at the centre of the piece
      model *= glm::rotate( angle, glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate the piece (about its centre)
      model *= glm::scale( glm::vec3(1.0f, 1.0f, 0.2f) );         // Shrink in the z axis to make the piece look a bit better (whilst still keeping the axis of rotation at the centre of the piece)
      model *= glm::translate( glm::vec3(-0.5f, -0.5f, -0.5f) );  // Undo the axis of rotation shift
      setMatrices();
      piece->render();

      prog_text.Use();
      model = glm::mat4(1.0f);
      model *= glm::translate( glm::vec3(0.0f, 0.5f, 1.0f) );  // Bottom-left corner of the text to be rendered (not taking into account the position of the piece)
      model *= glm::scale( glm::vec3(0.2, 0.2, 1.0f) );        // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      model *= glm::translate( glm::vec3(1.0f, 0.0f, 0.0f) );  // Shift the text to the right of the piece
      setMatrices();
      text.ResetTextPos();
      text.RenderString( sGameOver );
   }

   // Draw the reset text
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 0);
   prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.8f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( resetTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( resetTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( sReset );

   // Draw the menu text
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 0);
   prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.8f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( menuTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( menuTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( sOpenMenu );
#ifdef FPS_COUNTER
   // Draw the FPS text
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   //prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.8f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.8f, 1.175f, 0.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.025f, 0.025f, 0.025f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( std::to_string(int(fps)) );
#endif
}


void
Scene::RenderMenu()
{
   glDisable(GL_DEPTH_TEST);
   //view = glm::lookAt(glm::vec3(3.0f * cos(angle),1.5f,3.0f * sin(angle)), glm::vec3(0.0f,1.5f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
   view = glm::ortho(0.0f, 1.0f, 0.0f, 1.25f);
   model = glm::mat4(1.0f);

   float optionScale = 0.0625;

   // Define the position & scale of the AI Pesonality Title text
   glm::vec3 aiPersonalityTitleTextTranslate = glm::vec3(0.1f, 0.9f, 0.1f);
   glm::vec3 aiPersonalityTitleTextScale = glm::vec3(optionScale, optionScale, 1.0f);
   std::string sAIPersonalityTitle("AI Personality: ");

   // A vector for each property of the pickable objects
   std::vector<glm::vec3> pickableTranslate; pickableTranslate.reserve(6);
   std::vector<glm::vec3> pickableScale;     pickableScale.reserve(6);
   std::vector<std::string> pickableString;  pickableString.reserve(6);
   std::vector<glm::vec3> pickableID;        pickableID.reserve(6);
   std::vector<bool> pickableSelected;       pickableSelected.reserve(6);

   // Define the position & scale of the pickable AI Pesonality Moderate text
   pickableTranslate.push_back( glm::vec3(0.65f, 0.9f+optionScale/4, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale/2, optionScale/2, 1.0f) );
   pickableString.push_back("Moderate");
   pickableID.push_back(idColourAIPModerate);
   pickableSelected.push_back(aiPersonality==0);

   // Define the position & scale of the pickable AI Pesonality Aggressive text
   pickableTranslate.push_back( glm::vec3(0.8f, 0.9f+optionScale/4, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale/2, optionScale/2, 1.0f) );
   pickableString.push_back("Aggressive");
   pickableID.push_back(idColourAIPAggressive);
   pickableSelected.push_back(aiPersonality==1);

   // Define the position & scale of the pickable AI Pesonality Generous text
   pickableTranslate.push_back( glm::vec3(0.65f, 0.85f+optionScale/4, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale/2, optionScale/2, 1.0f) );
   pickableString.push_back("Generous");
   pickableID.push_back(idColourAIPGenerous);
   pickableSelected.push_back(aiPersonality==2);

   // Define the position & scale of the pickable AI Pesonality Stupid text
   pickableTranslate.push_back( glm::vec3(0.8f, 0.85f+optionScale/4, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale/2, optionScale/2, 1.0f) );
   pickableString.push_back("Stupid");
   pickableID.push_back(idColourAIPStupid);
   pickableSelected.push_back(aiPersonality==3);

   // Define the position & scale of the pickable AI Pesonality Random text
   pickableTranslate.push_back( glm::vec3(0.65f, 0.8f+optionScale/4, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale/2, optionScale/2, 1.0f) );
   pickableString.push_back("Random");
   pickableID.push_back(idColourAIPRandom);
   pickableSelected.push_back(aiPersonality==4);

   // Define the position & scale of the pickable close-menu text
   pickableTranslate.push_back( glm::vec3(0.1f, 0.1f, 0.1f) );
   pickableScale.push_back( glm::vec3(optionScale, optionScale, 1.0f) );
   pickableString.push_back("Close Menu");
   pickableID.push_back(idColourCloseMenu);
   pickableSelected.push_back(false);

   // Define the position & scale of the AI Side Title text
   glm::vec3 aiSideTitleTextTranslate( 0.1f, 0.7f, 0.1f );
   glm::vec3 aiSideTitleTextScale( optionScale, optionScale, 1.0f );
   std::string aiSideTitle("AI's side: ");

   // Define the position & scale of the pickable AI Side text
   glm::vec3 pickableAISideTranslate(0.65f, 0.7f, 0.1f);
   glm::vec3 pickableAISideScale(optionScale, optionScale, 1.0f);
   std::string pickableAISideString("  ");

   // Define the position & scale of the First Turn Title text
   glm::vec3 firstTurnTitleTextTranslate( 0.1f, 0.6f, 0.1f );
   glm::vec3 firstTurnTitleTextScale( optionScale, optionScale, 1.0f );
   std::string firstTurnTitle("First turn: ");

   // Define the position & scale of the pickable First Turn text
   glm::vec3 pickableFirstTurnTranslate(0.65f, 0.6f, 0.1f);
   glm::vec3 pickableFirstTurnScale(optionScale, optionScale, 1.0f);
   std::string pickableFirstTurnString("  ");

   // Define the position & scale of the Layout Title text
   glm::vec3 layoutTitleTextTranslate( 0.1f, 0.5f, 0.1f );
   glm::vec3 layoutTitleTextScale( optionScale, optionScale, 1.0f );
   std::string layoutTitle("Layout");
   std::string layoutSubTitle("(click to toggle): ");

   // Define the position & scale of the pickable Layout box
   glm::vec3 pickableLayoutTranslate(0.65f, 0.45f, 0.1f);
   glm::vec3 pickableLayoutScale(optionScale*2, optionScale*2, 1.0f);
   std::string pickableLayoutString("  ");

   // --------------------------------------------------------------------- //
   // Draw the board for picking
   // --------------------------------------------------------------------- //
   // Get the pixel colour under the mouse position
   if( bClicked == true )
   {
      // Enable the picking framebuffer
      //glBindFramebuffer(GL_FRAMEBUFFER, fboHandlePicking);
      // Set the background colour to pure white (i.e. the max object ID, which is a special ID interpreted as "do nothing")
      glClearColor(1.0f,1.0f,1.0f,1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      prog_board_picking.Use();

      // Draw all of the pickable Options
      for( unsigned int i = 0 ; i < pickableTranslate.size() ; i++ )
      {
         model = glm::mat4(1.0f);
         model *= glm::translate( pickableTranslate[i] );  // Bottom-left corner of the text to be rendered
         model *= glm::scale( pickableScale[i] );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
         setMatrices();
         text.ResetTextPos();
         text.RenderPickableString( pickableString[i] , pickableID[i] );
      }

      // Draw the pickable text/box for the AI Side toggle
      model = glm::mat4(1.0f);
      model *= glm::translate( pickableAISideTranslate );   // Bottom-left corner of the text to be rendered
      model *= glm::scale( pickableAISideScale );           // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderPickableString( pickableAISideString , idColourAISide );

      // Draw the pickable text/box for the First Turn toggle
      model = glm::mat4(1.0f);
      model *= glm::translate( pickableFirstTurnTranslate );  // Bottom-left corner of the text to be rendered
      model *= glm::scale( pickableFirstTurnScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderPickableString( pickableFirstTurnString , idColourFirstTurn );

      // Draw the pickable box for the Layout toggle
      model = glm::mat4(1.0f);
      model *= glm::translate( pickableLayoutTranslate );  // Bottom-left corner of the text to be rendered
      model *= glm::scale( pickableLayoutScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderPickableString( pickableLayoutString , idColourLayout );

      // Wait until all the pending drawing commands are really done.
      // There are usually a long time between glDrawElements() and
      // all the fragments completely rasterized.
      //glFlush();
      //glFinish();

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      unsigned char data[4];  // The pixel values will be 8-bit integer values
      glReadPixels(mouseX, mouseY,1,1, GL_RGBA, GL_UNSIGNED_BYTE, data);
      int pickedObjectID = RGBtoIDi(data[0], data[1], data[2]);

      ExecuteObject(pickedObjectID);

      bClicked = false;
   }

   // Bind the default framebuffer for rendering
   //glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glClearColor(0.5f,0.5f,0.5,1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   // --------------------------------------------------------------------- //
   // Render Text
   // --------------------------------------------------------------------- //

   // --------------------------------------------
   // Title
   // --------------------------------------------
   prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   prog_text.SetUniform("foregroundColour", glm::vec3(0.0f, 0.0f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.0f, 1.125f, 1.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.125, 0.125, 1.0f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString("Options Menu");

   // --------------------------------------------
   // Draw the AI Personality Title
   // --------------------------------------------
   //prog_text.Use();
   model = glm::mat4(1.0f);
   model *= glm::translate( aiPersonalityTitleTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( aiPersonalityTitleTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( sAIPersonalityTitle );

   // --------------------------------------------
   // Draw all of the pickable Options
   // --------------------------------------------
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 0);
   for( unsigned int i = 0 ; i < pickableTranslate.size() ; i++ )
   {
      if(pickableSelected[i])
         prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.5f, 0.8f));  // Purple background for currently-selected option
      else
         prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.8f, 0.0f));  // Yellow background for unselected option

      model = glm::mat4(1.0f);
      model *= glm::translate( pickableTranslate[i] );  // Bottom-left corner of the text to be rendered
      model *= glm::scale( pickableScale[i] );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
      setMatrices();
      text.ResetTextPos();
      text.RenderString( pickableString[i] );
   }

   // Draw the text for the AI Side title
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   model = glm::mat4(1.0f);
   model *= glm::translate( aiSideTitleTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( aiSideTitleTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( aiSideTitle );

   // --------------------------------------------
   // Draw the pickable text/box for the AI Side toggle
   // --------------------------------------------
   //prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 0);
   prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.5f, 0.8f));  // Purple background
   // Render "??" if the first turn is for a random side, or "  " if a X or O is selected to go first
   if( aiSideAtReset == 2 )
      pickableAISideString = "??";
   else
      pickableAISideString = "  ";
   // Render the text (which may be a blank box)
   model = glm::mat4(1.0f);
   model *= glm::translate( pickableAISideTranslate );   // Bottom-left corner of the text to be rendered
   model *= glm::scale( pickableAISideScale );           // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( pickableAISideString );
   // Render a piece over the text with the colour of the currently-selected option
   prog_piece.Use();
   prog_piece.SetUniform("IsCrownPiece", 1);
   setMatrices();
   if( aiSideAtReset == 0 )
   {
      prog_piece.SetUniform("Material.Kd", colourO);  // 0 = O
      piece->render();
   }
   else if( aiSideAtReset == 1 )
   {
      prog_piece.SetUniform("Material.Kd", colourX);  // 1 = X
      piece->render();
   }

   // --------------------------------------------
   // Draw the text for the First Turn title
   // --------------------------------------------
   prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   model = glm::mat4(1.0f);
   model *= glm::translate( firstTurnTitleTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( firstTurnTitleTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( firstTurnTitle );

   // Draw the text/box for the First Turn toggle
   prog_text.SetUniform("bUseAlpha", 0);
   prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.5f, 0.8f));  // Purple background
   // Render "??" if the first turn is for a random side, or "  " if a X or O is selected to go first
   if( firstTurnSide == 2 )
      pickableFirstTurnString = "??";
   else
      pickableFirstTurnString = "  ";
   // Render the text (which may be a blank box)
   model = glm::mat4(1.0f);
   model *= glm::translate( pickableFirstTurnTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( pickableFirstTurnScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( pickableFirstTurnString );
   // Render a piece over the text with the colour of the currently-selected option
   prog_piece.Use();
   prog_piece.SetUniform("IsCrownPiece", 1);
   setMatrices();
   if( firstTurnSide == 0 )
   {
      prog_piece.SetUniform("Material.Kd", colourO);  // 0 = O
      piece->render();
   }
   else if( firstTurnSide == 1 )
   {
      prog_piece.SetUniform("Material.Kd", colourX);  // 1 = X
      piece->render();
   }

   // --------------------------------------------------------------------- //
   // Draw the Layout Title & board
   // --------------------------------------------------------------------- //
   // Draw the Title & Sub-title
   prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   model = glm::mat4(1.0f);
   model *= glm::translate( layoutTitleTextTranslate );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( layoutTitleTextScale );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   float subTitleOffset = text.RenderString( layoutTitle );
   model *= glm::translate( glm::vec3( subTitleOffset, 0.0f, 0.0f ) );
   model *= glm::scale( glm::vec3(0.5f, 0.5f, 0.5f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( layoutSubTitle );

   // Draw the board
   prog_board.Use();
   model = glm::mat4(1.0f);
   model *= glm::translate( pickableLayoutTranslate );
   model *= glm::scale( pickableLayoutScale );
   setMatrices();
   glBindVertexArray(vaoHandleBoard);
   glDrawArrays(GL_QUADS, 0, nVertsBoard);

   // Draw the pieces
   // Calculate scaling factors
   float scaleX = 1.0f/board.Width();
   float scaleY = 1.0f/board.Height();
   float scaleZ = 1.0f/((board.Width()+board.Height())/2.0f);
   prog_piece.Use();
   for(unsigned int y = 0 ; y < board.Height() ; y++)
      for(unsigned int x = 0 ; x < board.Width() ; x++)
      {
         model = glm::mat4(1.0f);
         model *= glm::translate( pickableLayoutTranslate );
         model *= glm::scale( pickableLayoutScale );
         if( !(board.SquareIsEmpty(x,y)) )
         {
            model *= glm::translate( glm::vec3(x/float(board.Width()), y/float(board.Height()), 0.0f) );
            model *= glm::scale( glm::vec3(scaleX, scaleY, scaleZ) );
            setMatrices();

            if( board.SquareContainsXPiece(x,y) )
            {
               prog_piece.SetUniform("Material.Kd", colourX);
            }
            else
            {
               prog_piece.SetUniform("Material.Kd", colourO);
            }

            if( board.SquareContainsManPiece(x,y) )
               prog_piece.SetUniform("IsCrownPiece", 0);
            else
               prog_piece.SetUniform("IsCrownPiece", 1);

            piece->render();
         }
      }
#ifdef FPS_COUNTER
   // Draw the FPS text
   prog_text.Use();
   prog_text.SetUniform("bUseAlpha", 1);
   //prog_text.SetUniform("backGroundColour", glm::vec3(0.8f, 0.8f, 0.0f));
   model = glm::mat4(1.0f);
   model *= glm::translate( glm::vec3(0.8f, 1.175f, 0.0f) );  // Bottom-left corner of the text to be rendered
   model *= glm::scale( glm::vec3(0.025f, 0.025f, 0.025f) );    // Each character is 0.5f x 1.0f, so must be scaled up/down to a suitable size
   setMatrices();
   text.ResetTextPos();
   text.RenderString( std::to_string(int(fps)) );
#endif
}

void Scene::setMatrices()
{
   glm::mat4 mv = view * model;
   //prog.SetUniform("MVP", projection * mv);
   prog_board.SetUniform("MVP", mv);
   prog_piece.SetUniform("MVP", mv);
   prog_highlight.SetUniform("MVP", mv);
   prog_board_picking.SetUniform("MVP", mv);
   prog_text.SetUniform("MVP", mv);
   //prog_pickable_object.SetUniform("MVP", mv);
}

void Scene::resize(int w, int h)
{
   windowHeight = h; // Window height (used for inverting the mouse y-coord) is what it is

   // Calculate what the dimensions of the viewport should be (to maintain the desired aspect ratio, whilst using as much of the window as possible)
   float newWidthOverHeightRatio = float(w)/float(h);

   int xOffset = 0;
   int yOffset = 0;

   // New window is too wide, so reduce the width for rendering
   if( newWidthOverHeightRatio > widthOverHeightRatio )
   {
      height = h;
      width = h * widthOverHeightRatio;
      xOffset = (w - width)/2;
   }
   // New window is too tall, so reduce the height for rendering
   else if( newWidthOverHeightRatio < widthOverHeightRatio )
   {
      width = w;
      height = w / widthOverHeightRatio;
      yOffset = (h - height)/2;
   }
   // New window aspect ratio is just right, so set the rendering dimensions to the window dimensions
   else
   {
      width = w;
      height = h;
   }

   // Set the viewport dimensions & offset it to make the rendered section centered in the window
   glViewport(xOffset,yOffset,width,height);
   //projection = glm::perspective(90.0f, (float)w/h, 0.3f, 100.0f);
}

void Scene::compileAndLinkShader()
{
   // --------------------------------------------------------------------- //
   // Draughts Board Program
   // --------------------------------------------------------------------- //
   if( prog_board.CompileShaderFromFile("../draughts_board.vert",GLSLShader::VERTEX) )
   {
       std::cerr << "Vertex shader (draughts_board.vert) failed to compile!\n";
       exit(1);
   }
   if( prog_board.CompileShaderFromFile("../draughts_board.frag",GLSLShader::FRAGMENT))
   {
       std::cerr << "Fragment shader (draughts_board.frag) failed to compile!\n";
       exit(1);
   }
   if( prog_board.Link() )
   {
       std::cerr << "Draughts board shader program failed to link!\n";
       exit(1);
   }

   // --------------------------------------------------------------------- //
   // Draughts Piece Program
   // --------------------------------------------------------------------- //
   if( prog_piece.CompileShaderFromFile("../draughts_piece.vert",GLSLShader::VERTEX) )
   {
       std::cerr << "Vertex shader (draughts_piece.vert) failed to compile!\n";
       exit(1);
   }
   if( prog_piece.CompileShaderFromFile("../draughts_piece.frag",GLSLShader::FRAGMENT))
   {
       std::cerr << "Fragment shader (draughts_piece.frag) failed to compile!\n";
       exit(1);
   }
   if( prog_piece.Link() )
   {
       std::cerr << "Draughts piece shader program failed to link!\n";
       exit(1);
   }

   // --------------------------------------------------------------------- //
   // Draughts Piece Program
   // --------------------------------------------------------------------- //
   if( prog_highlight.CompileShaderFromFile("../draughts_highlight.vert",GLSLShader::VERTEX) )
   {
       std::cerr << "Vertex shader (draughts_highlight.vert) failed to compile!\n";
       exit(1);
   }
   if( prog_highlight.CompileShaderFromFile("../draughts_highlight.frag",GLSLShader::FRAGMENT))
   {
       std::cerr << "Fragment shader (draughts_highlight.frag) failed to compile!\n";
       exit(1);
   }
   if( prog_highlight.Link() )
   {
       std::cerr << "Draughts highlight shader program failed to link!\n";
       exit(1);
   }

   // --------------------------------------------------------------------- //
   // Draughts Board Picking
   // --------------------------------------------------------------------- //
   if( prog_board_picking.CompileShaderFromFile("../draughts_board_picking.vert",GLSLShader::VERTEX) )
   {
       std::cerr << "Vertex shader (draughts_board_picking.vert) failed to compile!\n";
       exit(1);
   }
   if( prog_board_picking.CompileShaderFromFile("../draughts_board_picking.frag",GLSLShader::FRAGMENT))
   {
       std::cerr << "Fragment shader (draughts_board_picking.frag) failed to compile!\n";
       exit(1);
   }
   if( prog_board_picking.Link() )
   {
       std::cerr << "Draughts board picking shader program failed to link!\n";
       exit(1);
   }

   // --------------------------------------------------------------------- //
   // Text
   // --------------------------------------------------------------------- //
   if( prog_text.CompileShaderFromFile("../text.vert",GLSLShader::VERTEX) )
   {
       std::cerr << "Vertex shader (text.vert) failed to compile!\n";
       exit(1);
   }
   if( prog_text.CompileShaderFromFile("../text.frag",GLSLShader::FRAGMENT))
   {
       std::cerr << "Fragment shader (text.frag) failed to compile!\n";
       exit(1);
   }
   if( prog_text.Link() )
   {
       std::cerr << "Draughts board text shader program failed to link!\n";
       exit(1);
   }

   // --------------------------------------------------------------------- //
   // Pickable Object Program
   // --------------------------------------------------------------------- //
   //if( prog_pickable_object.CompileShaderFromFile("../pickable_object.vert",GLSLShader::VERTEX) )
   //{
   //    std::cerr << "Vertex shader (pickable_object.vert) failed to compile!\n";
   //    exit(1);
   //}
   //if( prog_pickable_object.CompileShaderFromFile("../pickable_object.frag",GLSLShader::FRAGMENT))
   //{
   //    std::cerr << "Fragment shader (pickable_object.frag) failed to compile!\n";
   //    exit(1);
   //}
   //if( prog_pickable_object.Link() )
   //{
   //    std::cerr << "Pickable object shader program failed to link!\n";
   //    exit(1);
   //}

   // --------------------------------------------------------------------- //
   // Set the board program to be used by default
   // --------------------------------------------------------------------- //
   prog_board.Use();
}


glm::vec3
Scene::IDtoRGBf(const int id)
{
   // Convert the bits per channel into a maximum value per channel
   int RGBMaxValue = (1 << RGBIntBits) -1;

   // Extract the first 3 * RGBIntBits-bits from the integer ID, scale them down to floating point range 0.0-1.0, and put them into the R, G, & B values
   float r = ((id >> 0*RGBIntBits) & RGBMaxValue) / float(RGBMaxValue);
   float g = ((id >> 1*RGBIntBits) & RGBMaxValue) / float(RGBMaxValue);
   float b = ((id >> 2*RGBIntBits) & RGBMaxValue) / float(RGBMaxValue);

   return glm::vec3(r,g,b);
}

int
Scene::RGBtoIDf(const glm::vec3 rgb)
{
   // Convert the bits per channel into a maximum value per channel
   int RGBMaxValue = (1 << RGBIntBits) -1;

   // Scale the floating point values (in range 0.0-1.0) up to RGBIntBits integer
   int r = int(rgb.r * RGBMaxValue);
   int g = int(rgb.g * RGBMaxValue);
   int b = int(rgb.b * RGBMaxValue);

   return RGBtoIDi(r, g, b);
}

int
Scene::RGBtoIDi(const int r, const int g, const int b)
{
   // Insert each of the 3 * RGBIntBits-bits into the correct portion of the id bit-range
   int id = 0;
   id |= r << 0*RGBIntBits;
   id |= g << 1*RGBIntBits;
   id |= b << 2*RGBIntBits;

   return id;
}


void
Scene::ExecuteObject(const int id)
{
   // Only act if a pickable object has been selected
   if( id < int(pickableObjects.size()) )
   {
      pickableObjects[id]->Execute();
   }
}


void
Scene::OpenMenu()
{
   sceneToRender = MENU;
}


void
Scene::CloseMenu()
{
   sceneToRender = BOARD;
   ResetBoard();
}
