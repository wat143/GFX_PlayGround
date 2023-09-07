#ifndef __PIGLOBJECT__
#define __PIGLOBJECT__

#include <cassert>
#include <unordered_map>
#include <GLES2/gl2.h>
#include "Object.h"
#include "AssimpMesh.h"
#include "Context.h"
#include "ContextFactory.h"
#include "GLShader.h"
#include "Utils.h"

#define GL_ERROR_CHECK() assert(glGetError() == 0)

class PiGLObject : public Object {
private:
    std::unordered_map<GLuint, GLuint> attrBuf;
    std::unordered_map<GLuint, GLuint> uniBuf;
    GLuint indexBuf;
public:
    PiGLObject(const char*, const char*,
               unsigned int, unsigned int, unsigned int, unsigned int);
    ~PiGLObject();
    bool prepare();
    bool activate();
    bool activateTexture(std::string);
    bool activateFB(std::string);
    bool deactivate();
    void draw();
    void swapBuffers();
    bool addAttributeByType(std::string, int);
    bool addAttribute(std::string, void*, int);
    bool addIndexBuffer();
    bool addUniform(std::string);
    bool addTexture(std::string, int, int, std::string);
    bool addTexture(std::string, unsigned int);
    bool addFB(std::string);
    bool updateUniformVec3(std::string, glm::vec3&);
    bool updateUniformMat4(std::string, glm::mat4&);
    bool updateUniform1i(std::string, int);
};

#endif
