#ifndef __GL_UTILS__
#define __GL_UTILS__

#include <vector>
#include <map>
#include <GLES2/gl2.h>

enum SHADER_TYPE {
    VERTEX_SHADER,
    FRAGMENT_SHADER
};

struct Model {
    std::vector<const char*> shader_codes;
    std::map<std::string, GLuint> attr, uni;
    std::vector<const GLfloat*> attr_data;
    std::vector<GLuint> attr_size;
    GLuint program;
    GLuint *bufs = nullptr;
};

void printShaderLog(GLuint shader, int shader_type) {
    GLchar msg[1024] = "\0";
    GLsizei len;
    glGetShaderInfoLog(shader, 1024, &len, msg);
    std::cout << "========== !!!Shader Compile Error!!! ==========\n";
    std::cout << (shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
              << " shader compile error: " << msg << std::endl;
    std::cout << "================================================\n";
    exit(1);
}

#endif
