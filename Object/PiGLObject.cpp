#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "PiGLObject.h"
#include "AssimpMesh.h"
#include "EglContext.h"
#include "GLShader.h"
#include "ImageLoader.h"

PiGLObject::PiGLObject(const char* vs,const char* fs,
		       unsigned int x, unsigned int y, unsigned int w, unsigned int h)
  :Object(vs, fs, x, y, w, h)
{
  if (singleton == nullptr) {
    ContextFactory* ctxtFactory = new PiContextFactory();
    ctxt = std::shared_ptr<Context>(ctxtFactory->create());
    ContextSingleton::createInstance(ctxt);
    singleton = ContextSingleton::getInstance();
    delete ctxtFactory;
  }
  else
    ctxt = singleton->getContext();
  shader = new GLShader(vs, fs);
}

PiGLObject::~PiGLObject() {}

bool PiGLObject::prepare() {
  ctxt->makeCurrent();
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  shader->initShader();
  // Setup viewport
  glViewport(start_x, start_y, start_x + width, start_y + height);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::activate() {
  shader->useProgram();
  // Enable attributes
  for (auto itr = attribs.begin(); itr != attribs.end(); itr++) {
    glEnableVertexAttribArray(itr->second);
  }
  GL_ERROR_CHECK();
  // Bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::activateWithTexture(std::string name) {
  shader->useProgram();
  glBindTexture(GL_TEXTURE_2D, textures[name]);
  GL_ERROR_CHECK();
  updateUniform1i(name, 0);
  // Enable attributes
  for (auto itr = attribs.begin(); itr != attribs.end(); itr++) {
    glEnableVertexAttribArray(itr->second);
  }
  GL_ERROR_CHECK();
  // Bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::deactivate() {
  // Disable attributes
  for (auto itr = attribs.begin(); itr != attribs.end(); itr++) {
    glDisableVertexAttribArray(itr->second);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::deactivateWithTexture(std::string name) {
  glBindTexture(GL_TEXTURE_2D, 0);
  GL_ERROR_CHECK();
  return deactivate();
}

void PiGLObject::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES, mesh->getIndexSize(), GL_UNSIGNED_SHORT, 0);
  GL_ERROR_CHECK();
  glFlush();
  glFinish();
  ctxt->swapBuffers();
  GL_ERROR_CHECK();
}

bool PiGLObject::addAttributeByType(std::string str, int attrType) {
  if (attrType < 1 || attrType >= ATTR_TYPE_MAX)
    return false;
  GLuint attr = shader->getAttribLocation(str.c_str());
  attribs[str] = attr;
  // Generate buffer
  GLuint buf;
  GLfloat *data = nullptr;
  int size, num;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  if (attrType == VERTEX_DATA) {
    data = static_cast<GLfloat*>(mesh->getVertexPos());
    size = mesh->getVertexDataSize();
    num = 3;
  }
  else if (attrType == VERTEX_COLOR) {
    data = static_cast<GLfloat*>(mesh->getVertexColor());
    size = mesh->getVertexColorSize();
    num = 3;
  }
  else if (attrType == VERTEX_NORMAL) {
    data = static_cast<GLfloat*>(mesh->getVertexNormal());
    size = mesh->getVertexNormalSize();
    num = 3;
  }
  else if (attrType == UV_DATA) {
    data = static_cast<GLfloat*>(mesh->getUV());
    size = mesh->getUVDataSize();
    num = 2;
  }
  if (!data || !size) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return false;
  }
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size, data, GL_STATIC_DRAW);
  glVertexAttribPointer(attr, num, GL_FLOAT, GL_TRUE, 0, 0);
  GL_ERROR_CHECK();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return true;
}

bool PiGLObject::addAttribute(std::string str, void* data, int size) {
  GLuint attr = shader->getAttribLocation(str.c_str());
  attribs[str] = attr;
  // Generate buffer
  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size,
	       static_cast<GLfloat*>(data), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::addIndexBuffer() {
  glGenBuffers(1, &indexBuf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(GLushort) * mesh->getIndexSize(),
	       mesh->getIndex(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::addTexture(std::string filePath,int width,
			    int height, std::string name) {
  GLuint tex;
  // ToDo Image should be loaded out of the Object?
  char* image = ImageLoader().loadImageFromFile(filePath.c_str(), RAW, width, height);
  if (!image)
    return false;
  glGenTextures(1, &tex);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  GL_ERROR_CHECK();
  textures[name] = tex;
  glBindTexture(GL_TEXTURE_2D, tex);
  // ToDo Consider scalability for using eglImage and offscreen rendering case.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
	       GL_RGB, GL_UNSIGNED_BYTE, image);
  GL_ERROR_CHECK();
  // ToDo Consider configurable parameters
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
  GL_ERROR_CHECK();
  delete image;
  return true;
}

bool PiGLObject::addUniform(std::string str) {
  GLuint uni = shader->getUniLocation(str.c_str());
  GL_ERROR_CHECK();
  uniforms[str] = uni;
  return true;
}

bool PiGLObject::updateUniformVec3(std::string uni, glm::vec3& vec ) {
  if (!uniforms.count(uni))
    return false;
  glUniform3fv(uniforms[uni], 3, &vec[0]);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::updateUniformMat4(std::string uni, glm::mat4& mat ) {
  if (!uniforms.count(uni))
    return false;
  glUniformMatrix4fv(uniforms[uni], 1, GL_FALSE, &mat[0][0]);
  GL_ERROR_CHECK();
  return true;
}

bool PiGLObject::updateUniform1i(std::string uni, int i) {
  if (!uniforms.count(uni))
    return false;
  glUniform1i(uniforms[uni], i);
  GL_ERROR_CHECK();
  return true;
}
