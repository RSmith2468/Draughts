// --------------------------------------------------------------
// shader_program.h
//
//  Created on: 8 Sep 2016
//      Author: Robert
// --------------------------------------------------------------

#ifndef GLSLPROGRAM_H_
#define GLSLPROGRAM_H_

#include <string>
#include <list>
#include <GL/glew.h>
//#include <GL/freeglut.h>    // OpenGL Utility Toolkit header
#include <glm.hpp>

namespace GLSLShader
{
   enum GLSLShaderType
   {
      VERTEX,
      FRAGMENT,
      GEOMETRY,
      TESS_CONTROL,
      TESS_EVALUATION
   };
};

class CGLSLProgram
{
   public:
      CGLSLProgram();
      ~CGLSLProgram();

      bool CompileShaderFromFile(const std::string &fileName, const GLSLShader::GLSLShaderType type);
      bool CompileShaderFromString(const std::string &shaderCode, const GLSLShader::GLSLShaderType type);

      bool Link();
      void Use();

      std::string Log();

      int GetHandle();
      bool IsLinked();

      void BindAttribLocation(GLuint location, char const * const name);
      void BindFragDataLocation(GLuint location, char const * const name);

      void SetUniform(char const * const name, const float x, const float y, const float z);
      void SetUniform(char const * const name, const glm::vec3 &v);
      void SetUniform(char const * const name, const glm::vec4 &v);
      void SetUniform(char const * const name, const glm::mat3 &m);
      void SetUniform(char const * const name, const glm::mat4 &m);
      void SetUniform(char const * const name, const float val);
      void SetUniform(char const * const name, const int val);
      void SetUniform(char const * const name, const bool val);

      void PrintActiveUniforms();
      void PrintActiveAttribs();

   private:
      // Handle to the shader program
      int handle;
      // List of handles to the shader objects
      std::list<GLuint> shaderHandles;
      bool linked;
      std::string logString;
      int GetUniformLocation(char const * const name);
      bool FileExists(const std::string &fileName);
      bool LoadShaderAsString(const std::string &sFile, std::string &sShader);

      void OutputMessageAndLog(std::ostream &os, const std::string &message);

};

#endif /* GLSLPROGRAM_H_ */
