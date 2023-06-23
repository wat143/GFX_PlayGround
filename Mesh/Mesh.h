#ifndef __MESH__
#define __MESH__

#include <vector>
#include <string>

class Mesh {
 protected:
  std::vector<float> vertex_data;
  std::vector<float> vertex_color;
  std::vector<float> vertex_normal;
  std::vector<float> uv_data;
  std::vector<unsigned short> index_data;
  const std::string file_path;
  int index_num;
 public:
  Mesh(){}
  Mesh(std::string path):file_path(path),index_num(0){}
  virtual ~Mesh(){}
  virtual void import()=0;
  int getVertexDataSize() { return vertex_data.size(); }
  int getVertexColorSize() { return vertex_color.size(); }
  int getVertexNormalSize() { return vertex_normal.size(); }
  int getUVDataSize() { return uv_data.size(); }
  int getIndexSize() { return index_num; }
  float* getVertexPos() { return vertex_data.data(); }
  float* getVertexColor() { return vertex_color.data(); }
  float* getVertexNormal() { return vertex_normal.data(); }
  float* getUV() { return uv_data.data(); }
  unsigned short* getIndex() { return index_data.data(); }
  void updateVertexPos(float* data, int size) {
    vertex_data.clear();
    vertex_data.resize(size);
    for (int i = 0; i < size; i++)
      vertex_data[i] = data[i];
  }
  void updateVertexColor(float* data, int size) {
    vertex_color.clear();
    vertex_color.resize(size);
    for (int i = 0; i < size; i++)
      vertex_color[i] = data[i];
  }
  void updateVertexNormal(float* data, int size) {
    vertex_normal.clear();
    vertex_normal.resize(size);
    for (int i = 0; i < size; i++)
      vertex_normal[i] = data[i];
  }
  void updateUV(float* data, int size) {
    uv_data.clear();
    uv_data.resize(size);
    for (int i = 0; i < size; i++)
      uv_data[i] = data[i];
  }
  void updateIndex(unsigned short* data, int size) {
    index_data.clear();
    index_data.resize(size);
    index_num = size;
    for (int i = 0; i < size; i++)
      index_data[i] = data[i];
  }
};

#endif
