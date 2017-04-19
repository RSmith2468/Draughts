#ifndef VBOCYLINDER_H
#define VBOCYLINDER_H

class VBOCylinder
{
   public:
      VBOCylinder(int numSegments);

      void render();

   private:
      VBOCylinder();   // Not defined - must specify the number of segments
      unsigned int vaoHandle;
      unsigned int numVerts;  // Number of vertices that are rendered
};

#endif // VBOCYLINDER_H
