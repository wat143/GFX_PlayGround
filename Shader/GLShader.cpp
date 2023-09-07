#include <iostream>
#include "GLShader.h"

GLShader::GLShader(const char* vshader, const char* fshader):Shader(vshader, fshader){}
GLShader::~GLShader(){}

void GLShader::ShaderLog(GLuint shader) {
    unsigned int size = 255;
    char log[size];
    glGetShaderInfoLog(shader, sizeof(char) * size, nullptr, log);
    std::cout << "Shader " << shader << " Error:" << log << std::endl;
}

void GLShader::initShader() {
    GLint status;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vshaderSource, 0);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        ShaderLog(vertexShader);
        return;
    }
  
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fshaderSource, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        ShaderLog(fragmentShader);
        return;
    }

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
}

void GLShader::useProgram() {
    glUseProgram(program);
}

unsigned int GLShader::getAttribLocation(const char* attr) {
    unsigned int ret = glGetAttribLocation(program, attr);
    int error = glGetError();
    if (error != 0)
        return -1 * error;
    return ret;
}

unsigned int GLShader::getUniLocation(const char* uni) {
    unsigned int ret = glGetUniformLocation(program, uni);
    int error = glGetError();
    if (error != 0)
        return -1 * error;
    return ret;
}
