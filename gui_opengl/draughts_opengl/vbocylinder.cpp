#include "vbocylinder.h"
#include "defines.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include "glutils.h"
#include "defines.h"

#include <cstdio>
#include <cmath>

#include <vector>

VBOCylinder::VBOCylinder(int numSegments)
{
   // The minimum number of segments is 3
   if( numSegments < 3 )
      numSegments = 3;

   // Currently everything is drawn as GL_TRIANGLES (= 3 vertices specified per triangle)
   // Each circular face (top/bottom of the cylinder) is made up of numSegments triangles
   const unsigned int numVertsCircleFace = numSegments * 3;
   // Each square face (sides of cylinder) is made up of 2 triangles
   const unsigned int numVertsSquareFace = 3 * 2;
   // The cylinder is comprised of two circular faces & numSegments square faces
   numVerts = (numVertsCircleFace * 2) + (numVertsSquareFace * numSegments);

   // Calculate how many degrees of the circle each segment will cover
   const float anglePerSegment = TWOPI/numSegments;

   // --------------------------------------------------------------------- //
   // Set up CPU storage for the vertex positions, normals, & texture coordinates
   // --------------------------------------------------------------------- //
   // Vertices
   std::vector<float> pos(3 * numVerts);
   // Normals
   std::vector<float> norm(3 * numVerts);
   // Texture coordinates
   std::vector<float> texCoord(2 * numVerts);

   // --------------------------------------------------------------------- //
   // Calculate the vertex positions, normals, & texture coordinates
   // --------------------------------------------------------------------- //
   unsigned int indexPos = 0;
   unsigned int indexNorm = 0;
   unsigned int indexTexCoord = 0;

   // Bottom (z = 0) & Top (z = 1) circle faces
   // Note: texture coordinates are the same as the vertex (x,y) positions
   for( unsigned int posZ = 0 ; posZ < 2 ; posZ++ )
   {
      float normZ = posZ==0 ? -1.0f : 1.0f; // Z normal is the same for all vertices

      for( unsigned int seg = 0 ; int(seg) < numSegments ; seg++ )
      {
         float angleStart = seg * anglePerSegment;
         float angleEnd   = (seg+1) * anglePerSegment;

         // First point is the centre of the circle (0.5,0.5)
         pos[indexPos++] = texCoord[indexTexCoord++] = 0.5f; // x
         pos[indexPos++] = texCoord[indexTexCoord++] = 0.5f; // y
         pos[indexPos++] = float(posZ);// z
         norm[indexNorm++] = 0.0f;  // x
         norm[indexNorm++] = 0.0f;  // y
         norm[indexNorm++] = normZ; // z

         // Next point is at the edge at the start of the segment
         pos[indexPos++] = texCoord[indexTexCoord++] = (sin(angleStart)+1)/2.0f;  // x
         pos[indexPos++] = texCoord[indexTexCoord++] = (cos(angleStart)+1)/2.0f;  // y
         pos[indexPos++] = float(posZ);// z
         norm[indexNorm++] = 0.0f;  // x
         norm[indexNorm++] = 0.0f;  // y
         norm[indexNorm++] = normZ; // z

         // Next point is at the edge at the end of the segment
         pos[indexPos++] = texCoord[indexTexCoord++] = (sin(angleEnd)+1)/2.0f;  // x
         pos[indexPos++] = texCoord[indexTexCoord++] = (cos(angleEnd)+1)/2.0f;  // y
         pos[indexPos++] = float(posZ);// z
         norm[indexNorm++] = 0.0f;  // x
         norm[indexNorm++] = 0.0f;  // y
         norm[indexNorm++] = normZ; // z
      }
   }

   // Side faces
   for( unsigned int seg = 0 ; int(seg) < numSegments ; seg++ )
   {
      std::vector<float> angle = { seg * anglePerSegment , (seg+1) * anglePerSegment };
      std::vector<float> posX = { (sin(angle[0])+1)/2.0f ,(sin(angle[1])+1)/2.0f };
      std::vector<float> posY = { (cos(angle[0])+1)/2.0f ,(cos(angle[1])+1)/2.0f };
      //float angleStart = seg * anglePerSegment;
      //float angleEnd   = (seg+1) * anglePerSegment;

      // --- First Triangle --- //
      // First point is at the edge at the start of the segment, at the bottom of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[0];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[0];  // y
      pos[indexPos++] = norm[indexNorm++] = 0.0f; // z

      // Next point is at the edge at the start of the segment, at the top of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[0];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[0];  // y
      pos[indexPos++] = norm[indexNorm++] = 1.0f; // z

      // Next point is at the edge at the end of the segment, at the bottom of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[1];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[1];  // y
      pos[indexPos++] = norm[indexNorm++] = 0.0f; // z

      // --- Second Triangle --- //
      // First point is at the edge at the end of the segment, at the bottom of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[1];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[1];  // y
      pos[indexPos++] = norm[indexNorm++] = 0.0f; // z

      // Next point is at the edge at the start of the segment, at the top of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[1];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[1];  // y
      pos[indexPos++] = norm[indexNorm++] = 1.0f; // z

      // Next point is at the edge at the end of the segment, at the bottom of the cylinder
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posX[0];  // x
      pos[indexPos++] = texCoord[indexTexCoord++] = norm[indexNorm++] = posY[0];  // y
      pos[indexPos++] = norm[indexNorm++] = 1.0f; // z
   }

   // --------------------------------------------------------------------- //
   // Transfer the data to VBOs & setup the VAO
   // --------------------------------------------------------------------- //
   glGenVertexArrays( 1, &vaoHandle );
   glBindVertexArray(vaoHandle);

   unsigned int handle[3];
   glGenBuffers(3, handle);

   glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
   glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(float), &(pos[0]), GL_STATIC_DRAW);
   glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
   glEnableVertexAttribArray(0);  // Vertex position

   glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
   glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(float), &(norm[0]), GL_STATIC_DRAW);
   glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
   glEnableVertexAttribArray(1);  // Vertex normal

   glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
   glBufferData(GL_ARRAY_BUFFER, texCoord.size() * sizeof(float), &(texCoord[0]), GL_STATIC_DRAW);
   glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );
   glEnableVertexAttribArray(2);  // texture coords

   glBindVertexArray(0);
}

void VBOCylinder::render()
{
    glBindVertexArray(vaoHandle);
    //glDrawElements(GL_TRIANGLES, numVerts, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
    glDrawArrays(GL_TRIANGLES, 0, numVerts);
}
