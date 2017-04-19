// --------------------------------------------------------------
// shader_program.cpp
//
//  Created on: 9 Sep 2016
//      Author: Robert
// --------------------------------------------------------------


#include "glslprogram.h"

#include <iostream>
#include <fstream>
#include <iomanip>

CGLSLProgram::CGLSLProgram() : handle(0), linked(false), logString("")
{
}

CGLSLProgram::~CGLSLProgram()
{
   // Delete the shader program & shader objects
   glDeleteProgram(handle);
   for( std::list<GLuint>::iterator it = shaderHandles.begin() ; it != shaderHandles.end() ; it++ )
      if( *it != 0 )
         glDeleteShader(*it);

   // Output the error/info log to a log file
   std::string sFile("./log.txt");
   std::ofstream file(sFile);

   if(file.good())
   {
      file << logString;
   }
   else
   {
      std::cerr << "Error: unable to open output log file \"" << sFile << "\" for writing (CGLSLProgram::~CGLSLProgram)" << std::endl;
   }
}


// ------------------------------------------------------------------------------------
// Function to create a GLSL shader program object (if necessary), & compile
// a shader obect from a source file (& attach it to the shader program object)
// ------------------------------------------------------------------------------------
bool
CGLSLProgram::CompileShaderFromFile(const std::string                &fileName,
                                    const GLSLShader::GLSLShaderType  type)
{
   bool bErr = false;

   // Create the shader program object, if necessary
   if( handle <= 0 )
   {
      handle = glCreateProgram();
      //OutputMessageAndLog(std::cout, "Creating shader program object (CGLSLProgram::CompileShaderFromFile)");
   }

   // Check that shader program object handle is valid
   if( handle <= 0 )
   {
      OutputMessageAndLog(std::cerr, "Error creating shader program object (CGLSLProgram::CompileShaderFromFile)");
      bErr = true;
   }
   else
   {
      // Try to open the shader source file & copy its contents into a string
      std::string shaderCode;
      if(!LoadShaderAsString(fileName, shaderCode))  // Successfully read from file
      {
         // Put shader source into a shader object & attach it to the shader program object
         bErr = CompileShaderFromString(shaderCode, type);

         if( !bErr )
         {
            OutputMessageAndLog(std::cout, "Successfully opened & compiled shader file \""+fileName+"\"");
         }
      }
      else
      {
         // Failed to open the shader file & copy it into the string
         bErr=true;
      }
   }

   return bErr;
}


// ------------------------------------------------------------------------------------
// Function to create a shader program object (if necessary), create a shader object,
// compile the shader obect from a source string, & attach it to the shader program object
// ------------------------------------------------------------------------------------
bool
CGLSLProgram::CompileShaderFromString(const std::string& shaderCode, const GLSLShader::GLSLShaderType type)
{
   bool bErr = false;

   // Create the shader program object, if necessary
   if( handle <= 0 )
   {
      handle = glCreateProgram();
      //OutputMessageAndLog(std::cout, "Creating shader program object (CGLSLProgram::CompileShaderFromString)");
   }

   // Check that shader program object handle is valid
   if( handle <= 0 )
   {
      OutputMessageAndLog(std::cerr, "Error creating shader program object (CGLSLProgram::CompileShaderFromString)");
      bErr = true;
   }
   else
   {
      // -----------------------------------------------------
      // Create the shader object
      // -----------------------------------------------------
      GLuint shaderHandle = 0;

      switch( type )
      {
         case GLSLShader::VERTEX:
         {
            shaderHandle = glCreateShader(GL_VERTEX_SHADER);
            break;
         }
         case GLSLShader::FRAGMENT:
         {
            shaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
            break;
         }
         case GLSLShader::GEOMETRY:
         {
            shaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
            break;
         }
         case GLSLShader::TESS_CONTROL:
         {
            shaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER);
            break;
         }
         case GLSLShader::TESS_EVALUATION:
         {
            shaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER);
            break;
         }
         default:
         {
            OutputMessageAndLog(std::cerr, "Error: unknown shader type specified ("+std::to_string(type)+") (CGLSLProgram::CompileShaderFromString)");
            bErr = true;
            break;
         }
      }

      // Check that shader object creation succeeded
      if( shaderHandle == 0 )
      {
         OutputMessageAndLog(std::cerr, "Error creating shader object (CGLSLProgram::CompileShaderFromString)");
         bErr = true;
      }
      else
      {
         // -----------------------------------------------------
         // Copy the shader source string into the shader object (identified by shaderHandle)
         // -----------------------------------------------------
         // GL function call expects an array of shader strings (i.e. for compiling multiple files at once).
         // In this case there's only one shader source file, so it's a 1-element array
         const GLchar * codeArray[] = {shaderCode.c_str()};

         // Load the shader source string into the shader object
         glShaderSource(shaderHandle, 1, codeArray, NULL);
         //                     |             terminating character of the source code strings (could be an array of the length of each string)
         //                     size of codeArray

         // -----------------------------------------------------
         // Compile the shader
         // -----------------------------------------------------
         glCompileShader(shaderHandle);

         // -----------------------------------------------------
         // Verify Compilation status
         // -----------------------------------------------------
         GLint result;
         glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
         if(result == GL_FALSE)
         {
            bErr=true;
            // If compilation failed then...
            // Get the log info, which should contain the error messages & print this to stderr

            // Create a version of the shader code with tabs at the beginning of each line
            std::string shaderCodeTabbed(shaderCode);
            //std::string shaderCodeTabbed('\t'+shaderCode);
            //for( std::string::iterator it = shaderCodeTabbed.begin() ; it != shaderCodeTabbed.end() ; ++it )
            //   if( *it == '\n' )
            //      shaderCodeTabbed.insert(it+1, '\t');

            // Output error message including the tabbed shader source & append this to the log string
            OutputMessageAndLog(std::cerr, "Error: Failed to compile shader code (CGLSLProgram::CompileShaderFromString):\n"+shaderCodeTabbed);

            // Append the GLSL shader log to the log string
            GLint logLen;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLen);
            if(logLen>0)
            {
               char *log = new char[logLen];
               GLsizei written;
               glGetShaderInfoLog(shaderHandle, logLen, &written, log);
               OutputMessageAndLog(std::cerr, "Shader log (size="
                                              +std::to_string(logLen)
                                              +"):\n"
                                              +std::string(log)
                                              +'\n');
               delete[] log; log=0;
            }
         }
         else
         {
            // If compilation succeeded, then attach the shader object to the shader program object
            glAttachShader(handle, shaderHandle);
            //OutputMessageAndLog(std::cout, "Successfully compiled shader file");
            // Add this shader object handle to the list of shaders for this shader program
            shaderHandles.push_back(shaderHandle);
         }
      }
   }

   return bErr;
}


// ------------------------------------------------------------------------------------
// Program to link the shaders within the GLSL program object
// ------------------------------------------------------------------------------------
bool
CGLSLProgram::Link()
{
   bool bErr = false;

   // Check that shader program object handle is valid
   if( handle <= 0 )
   {
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::Link)");
      bErr = true;
   }

   // Only try to link the shader program object if it has not already been linked
   if( (linked == false) && (bErr == false) )
   {
      // -----------------------------------------------------------------------
      // Link the program
      // -----------------------------------------------------------------------
      glLinkProgram(handle);

      // Verify link status
      GLint status;
      glGetProgramiv(handle, GL_LINK_STATUS, &status);
      if(status==GL_FALSE)
      {
         bErr = true;

         OutputMessageAndLog(std::cerr, "Error: Failed to link shader program (CGLSLProgram::Link)");

         GLint logLen;
         glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
         if(logLen>0)
         {
            char *log = new char[logLen];
            GLsizei written;
            glGetProgramInfoLog(handle, logLen, &written, log);
            OutputMessageAndLog(std::cerr, "Program log (size="+std::to_string(logLen)+"):\n"+log+"\n");
            delete[] log; log=0;
         }
      }
      else
      {
         OutputMessageAndLog(std::cout, "Successfully linked the GLSL Shader program");
         linked = true;
      }
   }

   return bErr;
}


// ------------------------------------------------------------------------------------
// Function to install the GLSL shader program object into the OpenGL pipeline
// ------------------------------------------------------------------------------------
void
CGLSLProgram::Use()
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::Use)");
   else
      glUseProgram(handle);
}


// ------------------------------------------------------------------------------------
// Function to get the message log
// ------------------------------------------------------------------------------------
std::string
CGLSLProgram::Log()
{
   return logString;
}


// ------------------------------------------------------------------------------------
// Function to get the shader program object handle
// ------------------------------------------------------------------------------------
int
CGLSLProgram::GetHandle()
{
   return handle;
}


// ------------------------------------------------------------------------------------
// Function to check whether the shader program object has been linked
// ------------------------------------------------------------------------------------
bool
CGLSLProgram::IsLinked()
{
   return linked;
}


// ------------------------------------------------------------------------------------
// Function to define the mapping between a shader location (index) and shader variable (attribute/name)
// ------------------------------------------------------------------------------------
void
CGLSLProgram::BindAttribLocation(GLuint location, char const * const name)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::BindAttribLocation)");
   else
      glBindAttribLocation(handle, location, name);
}

// ------------------------------------------------------------------------------------
// Function to define the mapping between a fragment shader output location (index) and shader output variable (name)
// ------------------------------------------------------------------------------------
void
CGLSLProgram::BindFragDataLocation(GLuint location, char const * const name)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::BindAttribLocation)");
   else
      glBindFragDataLocation(handle, location, name);
}

void
CGLSLProgram::SetUniform(char const * const name, const float x, const float y, const float z)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,float,float,float))");
   else
   {
      // Find the location (i.e. shader argument index) of the uniform variable in the shader
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniform3f(location,x,y,z);
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const glm::vec3& v)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,vec3))");
   else
   {
      this->SetUniform(name, v.x, v.y, v.z);
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const glm::vec4& v)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,vec4))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniform4f(location, v.x, v.y, v.z, v.w);
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const glm::mat3& m)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,mat3))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniformMatrix3fv(location, 1, GL_FALSE, &(m[0][0]));
         //                 ¦         ¦  ¦         pointer to the data for the uniform variable
         //                 ¦         ¦  whther or not to transpose the matrix when loading it into the uniform variable
         //                 ¦         no. of matrices being assigned (could be an array of matrices)
         //                 uniform variable's location (i.e. shader argument index)
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const glm::mat4& m)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,mat3))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         //               v signals that we're intialising multiple values in an array
         glUniformMatrix4fv(location, 1, GL_FALSE, &(m[0][0]));
         //                 ¦         ¦  ¦         pointer to the data for the uniform variable
         //                 ¦         ¦  whther or not to transpose the matrix when loading it into the uniform variable
         //                 ¦         no. of matrices being assigned (could be an array of matrices)
         //                 uniform variable's location (i.e. shader argument index)

         // For data structures, each data member must be initialised individually
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const float val)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,mat4))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniform1f(location, val);
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const int val)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,int))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniform1i(location, val);
      }
   }
}

void
CGLSLProgram::SetUniform(char const * const name, const bool val)
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::SetUniform(char*,bool))");
   else
   {
      int location = GetUniformLocation(name);
      if( location >= 0 )  // Invalid (i.e. non-existing) location = -1
      {
         glUniform1i(location, val);
      }
   }
}


// ------------------------------------------------------------------------------------
// Function to print a list of active uniform variables
// ------------------------------------------------------------------------------------
void
CGLSLProgram::PrintActiveUniforms()
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::PrintActiveUniforms)");
   else
   {
      // -----------------------------------------------------------------------
      // Retrieve a list of active uniform variables
      // -----------------------------------------------------------------------
      GLint maxLength, nUniforms;
      glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &nUniforms);
      glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

      // Allocate a buffer to hold each attribute name
      std::string name(maxLength, ' ');
      // Get & print info. about each active uniform
      GLint written, size, location;
      GLenum type;
      std::cout << "\nList of active uniforms:" << std::endl;
      std::cout << "Index | Name" << std::endl;
      std::cout << "--------" << std::string(maxLength, '-') << std::endl;
      for( unsigned int i = 0 ; i < GLuint(nUniforms) ; i++ )
      {
         name.assign(maxLength, ' ');
         glGetActiveUniform( handle, i, maxLength, &written, &size, &type, &(name[0]));
         location = glGetUniformLocation(handle, &(name[0]));  // location = generic attribute index
         std::cout << std::setw(5) << location << " | " << name << std::endl;
      }
   }
}

void
CGLSLProgram::PrintActiveAttribs()
{
   // Check that shader program object handle is valid
   if( handle <= 0 )
      OutputMessageAndLog(std::cerr, "Error: Invalid shader program object handle (CGLSLProgram::PrintActiveAttribs)");
   else
   {
      // -----------------------------------------------------------------------
      // Retrieve no. of active attributes & max. length of their names
      // -----------------------------------------------------------------------
      GLint maxLength, nAttribs;
      glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &nAttribs);
      glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);

      // Allocate a buffer to hold each attribute name
      std::string name(maxLength, ' ');

      // Get & print info. about each active attribute
      GLint written, size, location;
      GLenum type;
      std::cout << "\nList of active attributes:" << std::endl;
      std::cout << "Index | Name" << std::endl;
      std::cout << "--------" << std::string(maxLength, '-') << std::endl;
      for( unsigned int i = 0 ; i < GLuint(nAttribs) ; i++ )
      {
         name.assign(maxLength, ' ');
         glGetActiveAttrib( handle, i, maxLength, &written, &size, &type, &(name[0]));
         location = glGetAttribLocation(handle, &(name[0]));  // location = generic attribute index
         std::cout << std::setw(5) << location << " | " << name << std::endl;
      }
   }
}

// ------------------------------------------------------------------------------------
// Private Functions
// ------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------
// Function to find the location (i.e. shader program argument index) of the uniform
// variable in the shader program object
// ------------------------------------------------------------------------------------
int
CGLSLProgram::GetUniformLocation(char const * const name)
{
   return glGetUniformLocation(handle, name);
   // Could query the location of a specific element by calling:
   //                glGetUniformLocation(programHandle, "RotationMatrix[0][1]");
   // For data structures, could query the location of a data member by calling:
   //                glGetUniformLocation(programHandle, "MyStruct.MyData");
}

bool
CGLSLProgram::LoadShaderAsString(const std::string& sFile,
                                       std::string& sShader)
{
   sShader="";
   bool bErr=false;
   std::ifstream file(sFile);

   if(file.good())
   {
      std::string sTmp;
      int i=0;
      while(file.good())
      {
         std::getline(file,sTmp);
         sShader+=sTmp+'\n';
         i++;
      }
      // Add null termination to the end (possibly unnecessary?)
      sShader+=char(0);
   }
   else
   {
      OutputMessageAndLog(std::cerr, "Error: unable to open shader \""+sFile+"\" (CGLSLProgram::LoadShaderAsString)");
      bErr=true;
   }

   return bErr;
}


void
CGLSLProgram::OutputMessageAndLog(      std::ostream &os,
                                  const std::string  &message)
{
   os << message << std::endl;
   logString += message+'\n';
}
