#ifndef __MESH__
#define __MESH__

#include <vector>
#include <string>
#include "GLES2/gl2.h"

class Mesh {
 protected:
  std::vector<GLfloat> vertex_data;
  std::vector<GLfloat> vertex_color;
  std::vector<GLfloat> vertex_normal;
  std::vector<GLfloat> uv_data;
  std::vector<GLushort> index_data;
  const std::string file_path;
 public:
  Mesh(std::string path):file_path(path){};
  virtual ~Mesh(){};
  virtual void import()=0;
  int getVertexSize() { return vertex_data.size(); }
  GLfloat* getVertexPos() { return vertex_data.data(); }
  GLfloat* getVertexColor() { return vertex_color.data(); }
  GLfloat* getVertexNormal() { return vertex_normal.data(); }
  GLfloat* getUV() { return uv_data.data(); }
  GLushort* getIndex() { return index_data.data(); }
};

#endif
