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

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef DISPMANX
#include <revision.h>
#include <bcm_host.h>
#endif

#include "AssimpMesh.h"
#include "Context.h"
#include "ContextFactory.h"
#include "Utils.h"
#include "glUtils.h"

#define TEAPOT_OBJ_PATH "../Resources/teapot.obj"
#define MONKEY_OBJ_PATH "../Resources/monkey.obj"
#define TEX_FILE_PATH "../Resources/Gaudi_128_128.raw"
#define check() assert(glGetError() == 0)

/* Vertex Shader */
const GLchar *vshader_source =
    "attribute vec3 vertexModelSpace;\n"
    "attribute vec3 normalModelSpace;\n"
    "uniform mat4 model;\n"
    "uniform mat4 mvp;\n"
    "varying vec2 tcoord;\n"
    "varying vec3 normal;\n"
    "varying vec3 vertexWorldSpace;\n"
    "void main(void) {\n"
    "  tcoord = vertexModelSpace.xy;\n"
    "  normal = normalModelSpace;\n"
    "  vertexWorldSpace = (model * vec4(vertexModelSpace, 1.0)).xyz;\n"
    "  gl_Position = mvp * vec4(vertexModelSpace, 1.0);\n"
    "}\n";

/* Fragment Shader */
const GLchar *fshader_source =
    "precision mediump float;\n"
    "uniform sampler2D tex;\n"
    "uniform mat4 invMatrix;\n"
    "uniform vec3 eyeDir;\n"
    "uniform vec3 lightDir;\n"
    "varying vec2 tcoord;\n"
    "varying vec3 normal;\n"
    "varying vec3 vertexWorldSpace;\n"
    "void main(void) {\n"
    "   vec4 ambient = vec4(vec3(0.03), 1.0);\n"
    //    "   vec3 invLight = normalize(invMatrix * vec4(lightDir, 0.0)).xyz;\n"
    "   vec3 invEye = normalize(invMatrix * vec4(eyeDir, 0.0)).xyz;\n"
    "   vec3 lightVec = normalize(lightDir - vertexWorldSpace);\n"
    "   vec3 invLight = normalize(invMatrix * vec4(lightVec, 0.0)).xyz;\n"
    "   float diffuse = clamp(dot(invLight, normal), 0.1, 1.0);\n"
    "   vec3 halfVector = normalize(invLight + invEye);\n"
    //"   vec3 halfVector = normalize(lightVec + invEye);\n"
    "   float cos = clamp(dot(normal, halfVector), 0.0, 1.0);\n"
    "   float specular = pow(cos, 25.0);\n"
    "   vec4 color = texture2D(tex, tcoord);\n"
    "   color = vec4(1.0, 0.5, 0.5, 1.0);\n"
    "   vec4 light = color * vec4(vec3(diffuse), 1.0) + vec4(vec3(specular), 1.0);\n"
    "   gl_FragColor = light + ambient;\n"
    "}\n";

/* Fragment Shader */
const GLchar *fshader_source_tex =
    "precision mediump float;\n"
    "uniform sampler2D tex;\n"
    "uniform mat4 invMatrix;\n"
    "uniform vec3 eyeDir;\n"
    "uniform vec3 lightDir;\n"
    "varying vec2 tcoord;\n"
    "varying vec3 normal;\n"
    "varying vec3 vertexWorldSpace;\n"
    "void main(void) {\n"
    "   vec4 ambient = vec4(vec3(0.03), 1.0);\n"
    //    "   vec3 invLight = normalize(invMatrix * vec4(lightDir, 0.0)).xyz;\n"
    "   vec3 invEye = normalize(invMatrix * vec4(eyeDir, 0.0)).xyz;\n"
    "   vec3 lightVec = normalize(lightDir - vertexWorldSpace);\n"
    "   vec3 invLight = normalize(invMatrix * vec4(lightVec, 0.0)).xyz;\n"
    "   float diffuse = clamp(dot(invLight, normal), 0.1, 1.0);\n"
    "   vec3 halfVector = normalize(invLight + invEye);\n"
    //"   vec3 halfVector = normalize(lightVec + invEye);\n"
    "   float cos = clamp(dot(normal, halfVector), 0.0, 1.0);\n"
    "   float specular = pow(cos, 25.0);\n"
    "   vec4 color = texture2D(tex, tcoord);\n"
    "   vec4 light = color * vec4(vec3(diffuse), 1.0) + vec4(vec3(specular), 1.0);\n"
    "   gl_FragColor = light + ambient;\n"
    "}\n";

glm::vec3 lightDir = {3.0, 2.0, 3.0};
glm::vec3 eyePos = {0.0, 2.0, 5.0};

void compile_shader(GLuint &shader, const char* shader_code,
                   GLuint shaderType)
{
    GLint status;
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shader_code, nullptr);
    glCompileShader(shader);
    /* check compile result */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        printShaderLog(shader, shaderType);
        assert(status);
    }
}

void build_program(std::vector<const char*> shader_codes,
                    GLuint &program)
{
    GLuint vert_shader, frag_shader;
    program = glCreateProgram();
    compile_shader(vert_shader, shader_codes[0], GL_VERTEX_SHADER);
    compile_shader(frag_shader, shader_codes[1], GL_FRAGMENT_SHADER);
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
}

void init_gl(Context *context) {
    unsigned int w, h;
    context->getMode(w, h);

    /* Enable depth test */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glViewport(0, 0, w, h);
    glClearColor(0.3, 0.3, 0.3, 1.0);
}

void init_model_tex(Model *model, Mesh *mesh) {
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);
    /* generate buffers */
    glGenBuffers(3, model->bufs);
    /* get attributes */
    model->attr["vertexModelSpace"] =
        glGetAttribLocation(model->program, "vertexModelSpace");
    model->attr["normalModelSpace"] =
        glGetAttribLocation(model->program, "normalModelSpace");
    /* bind data to VBO */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh->getVertexDataSize() * sizeof(float),
                 mesh->getVertexPos(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh->getVertexNormalSize() * sizeof(float),
                 mesh->getVertexNormal(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* bind index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh->getIndexSize() * sizeof(int),
                 mesh->getIndex(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    /* get uniforms */
    model->uni["model"] = glGetUniformLocation(model->program, "model");
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
    model->uni["tex"] = glGetUniformLocation(model->program, "tex");
    model->uni["invMatrix"] = glGetUniformLocation(model->program, "invMatrix");
    model->uni["eyeDir"] = glGetUniformLocation(model->program, "eyeDir");
    model->uni["lightDir"] = glGetUniformLocation(model->program, "lightDir");
}

void init_model_off(Model *model, Mesh *mesh) {
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);
    /* generate buffers */
    glGenBuffers(3, model->bufs);
    /* get attributes */
    model->attr["vertexModelSpace"] =
        glGetAttribLocation(model->program, "vertexModelSpace");
    model->attr["normalModelSpace"] =
        glGetAttribLocation(model->program, "normalModelSpace");
    /* bind data to VBO */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh->getVertexDataSize() * sizeof(float),
                 mesh->getVertexPos(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh->getVertexNormalSize() * sizeof(float),
                 mesh->getVertexNormal(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* bind index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh->getIndexSize() * sizeof(int),
                 mesh->getIndex(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    /* get uniforms */
    model->uni["model"] = glGetUniformLocation(model->program, "model");
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
    model->uni["invMatrix"] = glGetUniformLocation(model->program, "invMatrix");
    model->uni["eyeDir"] = glGetUniformLocation(model->program, "eyeDir");
    model->uni["lightDir"] = glGetUniformLocation(model->program, "lightDir");
}

void init_fb(Context *context, GLuint &texture, GLuint &fb) {
    unsigned int width, height;
    /* get width and height */
    context->getMode(width, height);
    /* generate texture and configure parameters */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* allocate storage for texture data */
    /* this is used for framebuffer storage, do not set data ptr */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    /* generate framebuffer */
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    /* attach texture to framebuffer */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texture, 0);

    /* unbind framebuffer and texture */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void redraw_off(Context *context, Model *model, Mesh* mesh,
                       GLuint fb, unsigned int counter)

{
    /* Bind default framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    /* clar previous rendering */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* calculate MVP matrix*/
    float rad = (counter % 360) * (M_PI / 180);
    unsigned int h, w;
    context->getMode(w, h);
    float aspect = w / h;
    glm::mat4 M, MVP, invM;
    M = glm::identity<glm::mat4>();
    MVP = glm::identity<glm::mat4>();
    invM = glm::identity<glm::mat4>();
    glm::mat4 V = glm::lookAt(glm::vec3(0.0, 2.0, 5.0),
                                 glm::vec3(0.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 P = glm::perspective(glm::radians(45.0f),
                                      aspect, 0.1f, 100.0f);
    M = glm::scale(M, glm::vec3(0.7, 0.7, 0.7));
    invM = glm::inverse(M);
    MVP = P * V * M;

    /* Enable program and attributes */
    glUseProgram(model->program);
    glEnableVertexAttribArray(model->attr["vertexModelSpace"]);
    glEnableVertexAttribArray(model->attr["normalModelSpace"]);
    /* bind attributes */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glVertexAttribPointer(model->attr["vertexModelSpace"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glVertexAttribPointer(model->attr["normalModelSpace"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);

    /* Update uniforms */
    glUniformMatrix4fv(model->uni["model"], 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(model->uni["mvp"], 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(model->uni["invMatrix"], 1, GL_FALSE, &invM[0][0]);    
    check();
    glUniform3fv(model->uni["eyeDir"], 1, &eyePos[0]);
    lightDir[0] = sin(rad) * 3;
    lightDir[1] = cos(rad) * 3;
    lightDir[2] = 3;
    glUniform3fv(model->uni["lightDir"], 1, &lightDir[0]);
    check();
    /* Bind index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    
    glDrawElements(GL_TRIANGLES, mesh->getIndexSize(),
                   GL_UNSIGNED_INT, 0);
    check();

    /* unbind VIO and texture */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glFlush();
    glFinish();
}

static void redraw_tex(Context *context, Model *model, Mesh* mesh,
                 GLuint texture, unsigned int counter)

{
    /* Bind default framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* clar previous rendering */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* calculate MVP matrix*/
    float rad = (counter % 360) * (M_PI / 180);
    unsigned int h, w;
    context->getMode(w, h);
    float aspect = w / h;
    glm::mat4 M, MVP, invM;
    M = glm::identity<glm::mat4>();
    invM = glm::identity<glm::mat4>();
    MVP = glm::identity<glm::mat4>();
    glm::mat4 V = glm::lookAt(glm::vec3(0.0, 2.0, 8.0),
                                 glm::vec3(0.0, 0.0, 0.0),
                                 glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 P = glm::perspective(glm::radians(45.0f),
                                      aspect, 0.1f, 100.0f);
    invM = glm::inverse(M);
    MVP = P * V * M;

    /* Enable program and attributes */
    glUseProgram(model->program);
    glEnableVertexAttribArray(model->attr["vertexModelSpace"]);
    glEnableVertexAttribArray(model->attr["normalModelSpace"]);
    /* bind attributes */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glVertexAttribPointer(model->attr["vertexModelSpace"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glVertexAttribPointer(model->attr["normalModelSpace"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);

    /* Bind texture */
    glBindTexture(GL_TEXTURE_2D, texture);

    /* Update uniforms */
    glUniformMatrix4fv(model->uni["model"], 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(model->uni["mvp"], 1, GL_FALSE, &MVP[0][0]);
    glUniform1i(model->uni["tex"], 0);
    glUniformMatrix4fv(model->uni["invMatrix"], 1, GL_FALSE, &invM[0][0]);    
    check();
    glUniform3fv(model->uni["eyeDir"], 1, &eyePos[0]);
    lightDir[0] = sin(rad) * 3;
    lightDir[1] = cos(rad) * 3;
    lightDir[2] = 3;
    glUniform3fv(model->uni["lightDir"], 1, &lightDir[0]);
    check();
    /* Bind index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    
    glDrawElements(GL_TRIANGLES, mesh->getIndexSize(),
                   GL_UNSIGNED_INT, 0);
    check();

    /* unbind VIO and texture */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFlush();
}

int main() {
    ContextFactory factory;
    Context *context = nullptr;
    Model model_teapot, model_monkey;
    Mesh* mesh_teapot = new AssimpMesh(TEAPOT_OBJ_PATH);
    Mesh* mesh_monkey = new AssimpMesh(MONKEY_OBJ_PATH);
    GLuint texture, fb;
    int terminate = 0;
    unsigned int counter = 0;

#ifdef NATIVE_DISP_WAYLAND
    std::cout << "NativeDisp = Wayland\n";    
    context = factory.create(Wayland);
#elif NATIVE_DISP_DRM
    std::cout << "NativeDisp = Drm\n";    
    context = factory.create(Drm);
#endif
    init_gl(context);
    /* init teapot model*/
    model_teapot.shader_codes.push_back(vshader_source);
    model_teapot.shader_codes.push_back(fshader_source_tex);
    model_teapot.bufs = new GLuint[3];
    mesh_teapot->import();
    init_model_tex(&model_teapot, mesh_teapot);

    /* init monkey model*/
    model_monkey.shader_codes.push_back(vshader_source);
    model_monkey.shader_codes.push_back(fshader_source);
    model_monkey.bufs = new GLuint[3];
    mesh_monkey->import();
    init_model_off(&model_monkey, mesh_monkey);

    /* init fb */
    init_fb(context, texture, fb);

    /* drawing */
    check();
    fprintf(stderr, "Start drawing\n");
    while (!terminate) {
        redraw_off(context, &model_monkey, mesh_monkey, texture, counter);
        redraw_tex(context, &model_teapot, mesh_teapot, fb, counter);
        /* swap buffers */
        context->swapBuffers();
        counter++;
    }
    return 0;
}
