#include "GLShader.h"

GLShader::GLShader(const char* vshader, const char* fshader):Shader(vshader, fshader){}
GLShader::~GLShader(){}

void GLShader::initShader() {
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertexShader, 1, &vshaderSource, 0);
  glShaderSource(fragmentShader, 1, &fshaderSource, 0);
  glCompileShader(vertexShader);
  glCompileShader(fragmentShader);
  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
}

void GLShader::useProgram() {
  glUseProgram(program);
}

unsigned int GLShader::getAttribLocation(char* attr) {
  unsigned int ret = glGetAttribLocation(program, attr);
  int error = glGetError();
  if (error != 0)
    return -1 * error;
  return ret;
}

unsigned int GLShader::getUniLocation(char* uni) {
  unsigned int ret = glGetUniformLocation(program, uni);
  int error = glGetError();
  if (error != 0)
    return -1 * error;
  return ret;
}
