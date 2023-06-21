#ifndef __OBJECT__
#define __OBJECT__

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Context.h"
#include "Shader.h"

class Object {
 protected:
  const unsigned int start_x;
  const unsigned int start_y;
  const unsigned int width;
  const unsigned int height;
  Mesh* mesh;
  Context* ctxt;
  Shader* shader;
  std::unordered_map<std::string, int> attribs;
  std::unordered_map<std::string, int> uniforms;
  std::unordered_map<std::string, int> textures;
 public:
  Object(const char*, const char*,
	 unsigned int x, unsigned int y,
	 unsigned int w, unsigned int h):start_x(x), start_y(y), width(w), height(h){
    mesh = nullptr;
  };
  virtual ~Object(){
    delete mesh;
    delete ctxt;
    delete shader;
  };
  void addMesh(Mesh* m) {
    if (!mesh)
      mesh = m;
  }
  virtual bool prepare()=0;
  virtual bool activate()=0;
  virtual bool activateWithTexture(std::string)=0;
  virtual bool deactivate()=0;
  virtual bool deactivateWithTexture(std::string)=0;
  virtual void draw()=0;
  virtual bool addAttributeByType(std::string, int)=0;
  virtual bool addAttribute(std::string, void*, int)=0;
  virtual bool addIndexBuffer()=0;
  virtual bool addUniform(std::string)=0;
  virtual bool addTexture(std::string, int, int, std::string)=0;
  // ToDo: add for other vec, mat types
  virtual bool updateUniformMat4(std::string, glm::mat4&)=0;
  virtual bool updateUniform1i(std::string, int)=0;
};

#endif
