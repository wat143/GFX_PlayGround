#ifndef __GLSHADER__
#define __GLSHADER__

#include "Shader.h"
#include <GLES2/gl2.h>

class GLShader : public Shader {
 private:
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
  void ShaderLog(GLuint);
 public:
  GLShader(const char*, const char*);
  ~GLShader();
  void initShader();
  void useProgram();
  unsigned int getAttribLocation(const char*);
  unsigned int getUniLocation(const char*);
};

#endif
