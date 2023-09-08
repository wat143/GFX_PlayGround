#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "AssimpMesh.h"
#include "Context.h"
#include "GLShader.h"
#include "Object.h"
#include "PiGLObject.h"
#include "Utils.h"

static void initGL(Object* object) {
    std::cout << "add vertex data\n";
    if (!object->addAttributeByType("vertexModelSpace", VERTEX_DATA)) {
        std::cerr << "Failed to add vertex data\n";
        return;
    }
    std::cout << "add uniform\n";
    if (!object->addUniform("mvp")) {
        std::cerr << "Failed to add uniform mvp\n";;
        return;
    }
    object->addIndexBuffer();
}

static void draw_triangle(unsigned int counter, Object *object)
{
    float rad = (counter % 360) * (M_PI / 180);
    float h = static_cast<float>(object->getHeight());
    float w = static_cast<float>(object->getWidth());
    float aspect = w / h;
    glm::mat4 Model, MVP;
    Model = glm::identity<glm::mat4>();
    MVP = glm::identity<glm::mat4>();
    glm::mat4 View = glm::lookAt(glm::vec3(0.0, 2.0, 5.0),
                                 glm::vec3(0.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 Proj = glm::perspective(glm::radians(45.0f),
                                      aspect, 0.1f, 100.0f);
    Model = glm::rotate(Model, rad, glm::vec3(0, 1, 0));
    MVP = Proj * View * Model;
    object->updateUniformMat4("mvp", MVP);
    object->draw();
}

int main() {
    float vertex[] = {
        0.0,  1.0,  0.0,
        -1.0, -1.0,  0.0,
        1.0, -1.0,  0.0
    };
    unsigned short index[] = {
        0, 1, 2
    };
    const GLchar* vshader_source = 
        "attribute vec3 vertexModelSpace;"
        "uniform mat4 mvp;"
        "void main(void) {"
        " gl_Position = mvp * vec4(vertexModelSpace, 1.0);"
        "}";
    const GLchar* fshader_source =
        "precision mediump float;"
        "void main(void) {"
        " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
        "}";
    unsigned int counter = 0;
    Object* object = new PiGLObject(vshader_source, fshader_source, DRM);
    Mesh* mesh = new AssimpMesh();
    std::cout << "Prepare mesh\n";
    mesh->updateVertexPos(vertex, 9);
    mesh->updateIndex(index, 3);
    object->addMesh(mesh);
    object->prepare();
    initGL(object);
    object->activate();
    std::cout << "Start drawing\n";
    while (1) {
        draw_triangle(counter, object);
        object->swapBuffers();
        counter++;
    }
    object->deactivate();
    return 0;
}
