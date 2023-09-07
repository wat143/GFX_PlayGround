#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <bcm_host.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef DISPMANX
#include <revision.h>
#endif

#include "AssimpMesh.h"
#include "Context.h"
#include "GLShader.h"
#include "Object.h"
#include "PiGLObject.h"

#define TEAPOT_OBJ_PATH "/home/pi/work/OpenGL/Binaries/teapot.obj"
#define MONKEY_OBJ_PATH "/home/pi/work/OpenGL/Binaries/monkey.obj"
#define TEX_FILE_PATH "/home/pi/work/GFX_PlayGround/Renderer/Gaudi_128_128.raw"
#define check() assert(glGetError() == 0)

glm::vec3 lightDir = {3.0, 2.0, 3.0};
glm::vec3 eyePos = {0.0, 2.0, 5.0};

void initGL_Offscreen(Object* object) {
    if (!object->addAttributeByType("vertexModelSpace", VERTEX_DATA)) {
        fprintf(stderr, "Failed to add attibute vertex\n");
        return;
    }
    check();
    if (!object->addAttributeByType("normalModelSpace", VERTEX_NORMAL)) {
        fprintf(stderr, "Failed to add attibute vertex normal\n");
        return;
    }
    check();
    if (!object->addUniform("model")) {
        fprintf(stderr, "Failed to add uniform mvp\n");
        return;
    }
    check();
    if (!object->addUniform("mvp")) {
        fprintf(stderr, "Failed to add uniform mvp\n");
        return;
    }
    check();
    if (!object->addUniform("tex")) {
        fprintf(stderr, "Failed to add uniform tex\n");
        return;
    }
    if (!object->addUniform("invMatrix")) {
        fprintf(stderr, "Failed to add uniform invMatrix\n");
        return;
    }
    if (!object->addUniform("eyePos")) {
        fprintf(stderr, "Failed to add uniform eyePos\n");
        return;
    }
    if (!object->addUniform("lightDir")) {
        fprintf(stderr, "Failed to add uniform lightDir\n");
        return;
    }
    check();
    if (!object->addTexture(TEX_FILE_PATH, 128, 128, "tex")) {
        fprintf(stderr, "Failed to add texture\n");
        return;
    }
    object->addFB("tex_fb");
    check();
    object->addIndexBuffer();
}

void initGL(Object* object, GLuint tex) {
    check();
    if (!object->addAttributeByType("vertexModelSpace", VERTEX_DATA)) {
        fprintf(stderr, "Failed to add attibute vertex\n");
        return;
    }
    check();
    if (!object->addAttributeByType("normalModelSpace", VERTEX_NORMAL)) {
        fprintf(stderr, "Failed to add attibute vertex normal\n");
        return;
    }
    check();
    //  if (!object->addAttributeByType("uv", UV_DATA)) {
    //    fprintf(stderr, "Failed to uniform UV\n");
    //    return;
    //  }
    check();
    if (!object->addUniform("model")) {
        fprintf(stderr, "Failed to add uniform mvp\n");
        return;
    }
    check();
    if (!object->addUniform("mvp")) {
        fprintf(stderr, "Failed to add uniform mvp\n");
        return;
    }
    check();
    if (!object->addUniform("tex")) {
        fprintf(stderr, "Failed to add uniform tex\n");
        return;
    }
    if (!object->addUniform("invMatrix")) {
        fprintf(stderr, "Failed to add uniform invMatrix\n");
        return;
    }
    if (!object->addUniform("eyeDir")) {
        fprintf(stderr, "Failed to add uniform eyePos\n");
        return;
    }
    if (!object->addUniform("lightDir")) {
        fprintf(stderr, "Failed to add uniform lightDir\n");
        return;
    }
    check();
    if (!object->addTexture("tex_fb", tex)) {
        fprintf(stderr, "Failed to add texture\n");
        return;
    }
    check();
    object->addIndexBuffer();
}

static void draw_offscreen(unsigned int counter,
                           Object *object)
{
    float rad = (counter % 360) * (M_PI / 180);
    float h = static_cast<float>(object->getHeight());
    float w = static_cast<float>(object->getWidth());
    float aspect = w / h;
    glm::mat4 Model, MVP, invModel;
    Model = glm::identity<glm::mat4>();
    MVP = glm::identity<glm::mat4>();
    invModel = glm::identity<glm::mat4>();
    glm::mat4 View = glm::lookAt(glm::vec3(0.0, 1.0, 3.0),
                                 glm::vec3(0.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 Proj = glm::perspective(glm::radians(45.0f),
                                      aspect, 0.1f, 100.0f);
    Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
    Model = glm::rotate(Model, rad, glm::vec3(0, 1, 0));
    invModel = glm::inverse(Model);
    MVP = Proj * View * Model;
    // Configure uniforms
    object->updateUniformMat4("model", Model);
    object->updateUniformMat4("mvp", MVP);
    object->updateUniformMat4("invMatrix", invModel);
    check();
    object->updateUniformVec3("eyeDir", eyePos);
    lightDir[0] = sin(rad) * 3;
    lightDir[1] = cos(rad) * 3;
    lightDir[2] = 3;
    object->updateUniformVec3("lightDir", lightDir);
    check();
    // Draw
    object->draw();
    check();
}
static void draw_triangles(unsigned int counter,
                           Object *object)
{
    float rad = (counter % 360) * (M_PI / 180);
    float h = static_cast<float>(object->getHeight());
    float w = static_cast<float>(object->getWidth());
    float aspect = w / h;
    glm::mat4 Model, MVP, invModel;
    Model = glm::identity<glm::mat4>();
    MVP = glm::identity<glm::mat4>();
    invModel = glm::identity<glm::mat4>();
    glm::mat4 View = glm::lookAt(glm::vec3(0.0, 2.0, 5.0),
                                 glm::vec3(0.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 Proj = glm::perspective(glm::radians(45.0f),
                                      aspect, 0.1f, 100.0f);
    Model = glm::scale(Model, glm::vec3(0.7, 0.7, 0.7));
    invModel = glm::inverse(Model);
    MVP = Proj * View * Model;
    // Configure uniforms
    object->updateUniformMat4("model", Model);
    object->updateUniformMat4("mvp", MVP);
    object->updateUniformMat4("invMatrix", invModel);
    check();
    object->updateUniformVec3("eyeDir", eyePos);
    lightDir[0] = sin(rad) * 3;
    lightDir[1] = cos(rad) * 3;
    lightDir[2] = 3;
    object->updateUniformVec3("lightDir", lightDir);
    check();
    // Draw
    object->draw();
    check();
}

int main() {
    // Vertex Shader
    const GLchar *vshader_source =
        "attribute vec3 vertexModelSpace;"
        "attribute vec3 normalModelSpace;"
        "uniform mat4 model;"
        "uniform mat4 mvp;"
        "varying vec2 tcoord;"
        "varying vec3 normal;"
        "varying vec3 vertexWorldSpace;"
        "void main(void) {"
        "  tcoord = vertexModelSpace.xy;"
        "  normal = normalModelSpace;"
        "  vertexWorldSpace = (model * vec4(vertexModelSpace, 1.0)).xyz;"
        "  gl_Position = mvp * vec4(vertexModelSpace, 1.0);"
        "}";

    //Fragment Shader
    const GLchar *fshader_source =
        "precision mediump float;"
        "uniform sampler2D tex;"
        "uniform mat4 invMatrix;"
        "uniform vec3 eyeDir;"
        "uniform vec3 lightDir;"
        "varying vec2 tcoord;"
        "varying vec3 normal;"
        "varying vec3 vertexWorldSpace;"
        "void main(void) {"
        "   vec4 ambient = vec4(vec3(0.03), 1.0);"
        //    "   vec3 invLight = normalize(invMatrix * vec4(lightDir, 0.0)).xyz;"
        "   vec3 invEye = normalize(invMatrix * vec4(eyeDir, 0.0)).xyz;"
        "   vec3 lightVec = normalize(lightDir - vertexWorldSpace);"
        "   vec3 invLight = normalize(invMatrix * vec4(lightVec, 0.0)).xyz;"
        "   float diffuse = clamp(dot(invLight, normal), 0.1, 1.0);"
        "   vec3 halfVector = normalize(invLight + invEye);"
        //"   vec3 halfVector = normalize(lightVec + invEye);"
        "   float cos = clamp(dot(normal, halfVector), 0.0, 1.0);"
        "   float specular = pow(cos, 25.0);"
        "   vec4 color = texture2D(tex, tcoord);"
        "   vec4 light = color * vec4(vec3(diffuse), 1.0) + vec4(vec3(specular), 1.0);"
        "   gl_FragColor = light + ambient;"
        "}";
    int terminate = 0;
    unsigned int counter = 0;
    Object* object = new PiGLObject(vshader_source, fshader_source,
                                    0, 0, 600, 480);
    Object* objectOffscreen = new PiGLObject(vshader_source, fshader_source,
                                             0, 0, 600, 480);
    Mesh* meshTeapot = new AssimpMesh(TEAPOT_OBJ_PATH);
    Mesh* meshMonkey = new AssimpMesh(MONKEY_OBJ_PATH);

    meshTeapot->import();
    object->addMesh(meshTeapot);
    object->prepare();
    meshMonkey->import();
    objectOffscreen->addMesh(meshMonkey);
    objectOffscreen->prepare();
    initGL_Offscreen(objectOffscreen);
    initGL(object, objectOffscreen->getTexture("tex_fb"));
    check();
    fprintf(stderr, "Start drawing\n");
    while (!terminate) {
        // Offscreen rendering
        objectOffscreen->activate();
        objectOffscreen->activateTexture("tex");
        objectOffscreen->activateFB("tex_fb");
        draw_offscreen(counter, objectOffscreen);
        objectOffscreen->deactivate();
        // Get FB
        object->activate();
        object->activateTexture("tex_fb");
        draw_triangles(counter, object);
        object->deactivate();
        //Submit buffer to window system
        object->swapBuffers();
        counter++;
    }
    delete object;
    delete objectOffscreen;
    return 0;
}
