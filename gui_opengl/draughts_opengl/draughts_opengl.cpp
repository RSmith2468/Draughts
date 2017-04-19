// --------------------------------------------------------------
// Description: GUI for the draughts game
// --------------------------------------------------------------

#include <GL/glew.h>
#include <GL/freeglut.h>    // OpenGL Utility Toolkit header

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>

#include "glslprogram.h"

#include "scene.h"

// -------------------------------------------------------------------
// Nasty global hack for timing
// -------------------------------------------------------------------
//#include <ctime>
//#include <ratio>
#include <chrono>
std::chrono::high_resolution_clock::time_point startTime;
// -------------------------------------------------------------------

// -------------------------------------------------------------------
// Function Prototypes
// -------------------------------------------------------------------
// Window Initialisation
void Init();
// Main drawing function
void Redraw();
// Window Reshape
void Reshape(int w, int h);
// Normal key press
void key(unsigned char c, int x, int y);
// Special key press
void special(int k, int x, int y);
// Mouse click
void mouse(int button, int state, int mouseX, int mouseY);


// -----------------------------------------------------------------------
// Create the "scene" object that will be used
// -----------------------------------------------------------------------
Scene scene;

// -------------------------------------------------------------------
// Main Function
// -------------------------------------------------------------------
int
main(int argc, char **argv)
{
	bool bErr = false;
   // -----------------------------------------------------
   // GLUT window initialisation
   // -----------------------------------------------------
   glutInit(&argc, argv);
   int initW(600), initH(750), initX(100), initY(100);
   glutInitWindowSize(initW, initH);
   glutInitWindowPosition ( initX, initY );

   scene.resize(initW, initH);

   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   // Initialise the window to use multi-sample anti-aliasing (4 samples)
   //glutSetOption(GLUT_MULTISAMPLE, 4);
   //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
   //glEnable(GL_MULTISAMPLE);

   /*int windowNum=*/glutCreateWindow("Draughts Game");

   glutDisplayFunc(Redraw);

   glutReshapeFunc(Reshape);

   glutKeyboardFunc(key);
   glutSpecialFunc(special);
   glutMouseFunc(mouse);

   // -----------------------------------------------------
   // Initialise GLEW
   // -----------------------------------------------------
   GLenum err = glewInit();
   if(err != GLEW_OK)
   {
      std::cerr << "Error initialising GLEW: " << glewGetErrorString(err) << std::endl;
      bErr = true;
   }
   else
      std::cout << "Successfully initialised GLEW" << std::endl;

   //Init();

   if( ! bErr )
   {
      // -----------------------------------------------------
      // Output OpenGL version info.
      // -----------------------------------------------------
      std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl
                << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
                << "Renderer:       " << glGetString(GL_RENDERER) << std::endl;

      // Get the OpenGL version as numbers (for checking/analysis)
      GLint gl_major, gl_minor;
      glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
      glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
      std::cout << "OpenGL Version (numerical): " << gl_major << "." << gl_minor << std::endl;

      // -----------------------------------------------------------------------
      // Check whether multisample buffers are available & how many samples per pixel are being used
      // -----------------------------------------------------------------------
      //GLint bufs, samples;
      //glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
      //glGetIntegerv(GL_SAMPLES, &samples);
      //std::cout << "MSAA buffers = " << bufs << ", MSAA samples = " << samples << std::endl;

      // -----------------------------------------------------------------------
      // Initialise the scene
      // -----------------------------------------------------------------------
      scene.initScene();

      // -----------------------------------------------------------------------
      // Get initial start time of the program
      // -----------------------------------------------------------------------
      startTime = std::chrono::high_resolution_clock::now();

      // -----------------------------------------------------------------------
      // Run the main processing loop
      // -----------------------------------------------------------------------
      glutMainLoop();

   } // bErr (GLEW initialisation)

   return bErr;
}

// -------------------------------------------------------------------
// Window Initialisation
// -------------------------------------------------------------------
void
Init()
{
   //glShadeModel(GL_SMOOTH);
   //glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
   //glClearDepth(1.0f);
   //glEnable(GL_DEPTH_TEST);
   //glDepthFunc(GL_LEQUAL);
   //glEnable ( GL_COLOR_MATERIAL );
   //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

   //sceneTorusWithDiffuseLighting.initScene();
}


// -------------------------------------------------------------------
// Main Drawing Function
// -------------------------------------------------------------------
void
Redraw()
{
   glClear(GL_COLOR_BUFFER_BIT);

   std::chrono::high_resolution_clock::time_point currTime = std::chrono::high_resolution_clock::now();

   //std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(currTime - startTime);
//std::cout << "elapsedSecs = " << time_span.count() << std::endl;
   //scene.update(time_span.count()/1000);

   std::chrono::duration<double, std::milli> time_span = currTime - startTime;
   scene.update(time_span.count()/1000);

   //scene.update(0.001f);
   scene.render();

   glutSwapBuffers();
   glutPostRedisplay();

}


// -------------------------------------------------------------------
// Window Resize Function
// -------------------------------------------------------------------
void
Reshape(int w, int h)
{
   scene.resize(w, h);
}


// -------------------------------------------------------------------
// Key Press Function
// -------------------------------------------------------------------
void
key(unsigned char c, int x, int y)
{
   if (c == 27)
   {
     exit(0);  /* IRIS GLism, Escape quits. */
   }
   else if( c == 13 || c == 32 ) // Carriage return or space
   {
      scene.UserInputSelect();
   }
}



// -------------------------------------------------------------------
// Key Press Function
// -------------------------------------------------------------------
void
special(int k, int x, int y)
{
   if( k == GLUT_KEY_UP )
      //scene.UserInputUp();
      scene.UserInputDown();
   else if( k == GLUT_KEY_DOWN )
      //scene.UserInputDown();
      scene.UserInputUp();
   else if( k == GLUT_KEY_LEFT )
      scene.UserInputLeft();
   else if( k == GLUT_KEY_RIGHT )
      scene.UserInputRight();

}


// -------------------------------------------------------------------
// Mouse Click Function
// -------------------------------------------------------------------
void
mouse(int button, int state, int mouseX, int mouseY)
{
   if( (state == GLUT_UP) && (button == GLUT_LEFT_BUTTON) )
   {
      scene.ClickLocation(mouseX, mouseY);
   }
}

