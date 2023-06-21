#include <fstream>
#include <iostream>
#include "ImageLoader.h"

char* ImageLoader::loadImageFromFile(const char* path, int type,
				    int width, int height) {
  int fsize;
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  char* out = nullptr;
  if (type == RAW) {
    fsize = width * height * 3;
  }
  out = new char[fsize];
  if (ifs.fail()) {
    std::cout << "Failed to open file\n";
    return nullptr;
  }
  ifs.read(out, fsize);
  return out;
}
