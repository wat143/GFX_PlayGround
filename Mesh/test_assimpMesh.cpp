#include <iostream>
#include <string>
#include "Mesh.h"
#include "AssimpMesh.h"

int main() {
    //  std::string filePath{"/home/pi/work/FBX-glTF/models/teapot/dae/teapot.gltf"};
    std::string filePath{"/home/pi/work/OpenGL/Binaries/teapot.obj"};
    Mesh *m = new AssimpMesh(filePath);
    m->import();
    GLfloat *tmp = m->getVertexPos();
    std::cout << "===Vertex Position\n";
    for (int i = 0; i < m->getVertexSize(); i+=3)
        std::cout << tmp[i] << "," << tmp[i+1] << "," << tmp[i+2] << std::endl;
    std::cout << "===Vertex Normal\n";
    tmp = m->getVertexNormal();
    for (int i = 0; i < m->getVertexSize(); i+=3)
        std::cout << tmp[i] << "," << tmp[i+1] << "," << tmp[i+2] << std::endl;

    delete m;

    return 0;
}
