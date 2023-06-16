#include <fstream>
#include <iostream>
#include "ImageLoader.h"

char* ImageLoader::loadImageFromFile(const char* path, int type,
				    int width, int height) {
  int fsize;
  std::ifstream ifs(path);
  char* out = nullptr;
  if (type == RAW) {
    fsize = width * height * 3;
  }
  out = new char[fsize];
  if (ifs.fail()) {
    std::cout << "Failed to open file\n";
    return nullptr;
  }
  while (ifs.getline(out, fsize));
  return out;
}
