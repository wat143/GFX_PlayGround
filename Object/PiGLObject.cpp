#include <glm/glm.hpp>
#include "PiGLObject.h"
#include "AssimpMesh.h"
#include "EglContext.h"
#include "GLShader.h"

PiGLObject::PiGLObject(std::string meshPath, const char* vs,const char* fs,
		       unsigned int x, unsigned int y, unsigned int w, unsigned int h)
  :Object(meshPath, vs, fs, x, y, w, h)
{
  mesh = new AssimpMesh(meshPath.c_str());
  ctxtFactory = new PiContextFactory();
  ctxt = ctxtFactory->create();
  shader = new GLShader(vs, fs);
}

PiGLObject::~PiGLObject() {
  delete ctxtFactory;
}

bool PiGLObject::prepare() {
  ctxt->makeCurrent();
  // Set background color and clear buffers
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  shader->initShader();
  // Setup viewport
  glViewport(start_x, start_y, start_x + width, start_y + height);
  ErrorCheck();
  // Import mesh
  mesh->import();
  return true;
}

bool PiGLObject::activate() {
  shader->useProgram();
  // Enable attributes
  for (auto itr = attribs.begin(); itr != attribs.end(); itr++) {
    glEnableVertexAttribArray(itr->second);
  }
  // Bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
  ErrorCheck();
  return true;
}

bool PiGLObject::deactivate() {
  // Disable attributes
  for (auto itr = attribs.begin(); itr != attribs.end(); itr++) {
    glDisableVertexAttribArray(itr->second);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  ErrorCheck();
  return true;
}

void PiGLObject::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES, mesh->getIndexSize(), GL_UNSIGNED_SHORT, 0);
  ErrorCheck();
  glFlush();
  glFinish();
  ctxt->swapBuffers();
  ErrorCheck();
}

bool PiGLObject::addAttributeByType(std::string str, int attrType) {
  if (attrType < 1 || attrType >= ATTR_TYPE_MAX)
    return false;
  GLuint attr = shader->getAttribLocation(str.c_str());
  attribs[str] = attr;
  // Generate buffer
  GLuint buf;
  GLfloat *data = nullptr;
  int size, num = 3;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  if (attrType == VERTEX_DATA) {
    data = static_cast<GLfloat*>(mesh->getVertexPos());
    size = mesh->getVertexDataSize();
  }
  else if (attrType == VERTEX_COLOR) {
    data = static_cast<GLfloat*>(mesh->getVertexColor());
    size = mesh->getVertexColorSize();
  }
  else if (attrType == VERTEX_NORMAL) {
    data = static_cast<GLfloat*>(mesh->getVertexNormal());
    size = mesh->getVertexNormalSize();
  }
  else if (attrType == UV_DATA) {
    data = static_cast<GLfloat*>(mesh->getUV());
    size = mesh->getUVDataSize();
  }
  if (!data)
    return false;
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size, data, GL_STATIC_DRAW);
  glVertexAttribPointer(attr, num, GL_FLOAT, GL_TRUE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  ErrorCheck();
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
  ErrorCheck();
  return true;
}

bool PiGLObject::addIndexBuffer() {
  glGenBuffers(1, &indexBuf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(GLushort) * mesh->getIndexSize(),
	       mesh->getIndex(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  ErrorCheck();
  return true;
}

bool PiGLObject::addUniform(std::string str) {
  GLuint uni = shader->getUniLocation(str.c_str());
  ErrorCheck();
  uniforms[str] = uni;
  return true;
}

bool PiGLObject::updateUniformMat4(std::string uni, glm::mat4& mat ) {
  if (!uniforms.count(uni))
    return false;
  glUniformMatrix4fv(uniforms[uni], 1, GL_FALSE, &mat[0][0]);
  ErrorCheck();
  return true;
}
