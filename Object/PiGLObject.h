#ifndef __PIGLOBJECT__
#define __PIGLOBJECT__

#include <cassert>
#include <unordered_map>
#include "GLES2/gl2.h"
#include "Object.h"
#include "AssimpMesh.h"
#include "Context.h"
#include "PiContextFactory.h"
#include "GLShader.h"
#include "Utils.h"

#define GL_ERROR_CHECK() assert(glGetError() == 0)

class PiGLObject : public Object {
 private:
  ContextFactory* ctxtFactory;
  std::unordered_map<GLuint, GLuint> attrBuf;
  std::unordered_map<GLuint, GLuint> uniBuf;
  GLuint indexBuf;
 public:
  PiGLObject(std::string, const char*, const char*,
	     unsigned int, unsigned int, unsigned int, unsigned int);
  ~PiGLObject();
  bool prepare();
  bool activate();
  bool activateWithTexture(std::string);
  bool deactivate();
  bool deactivateWithTexture(std::string);
  void draw();
  bool addAttributeByType(std::string, int);
  bool addAttribute(std::string, void*, int);
  bool addIndexBuffer();
  bool addUniform(std::string);
  bool addTexture(std::string, int, int, std::string);
  bool updateUniformMat4(std::string, glm::mat4&);
  bool updateUniform1i(std::string, int);
};

#endif
