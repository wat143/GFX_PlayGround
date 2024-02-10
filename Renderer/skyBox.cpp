#include <iostream>
#include <map>
#include <cmath>
#include <GLES2/gl2.h>

#include "ContextFactory.h"
#include "glUtils.h"
#include "Utils.h"
#include "AssimpMesh.h"
#include "ContextFactory.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEAPOT_OBJ_PATH "../Resources/teapot.obj"
#define LOOP_COUNT 1000000

static const char *vshader_cube = {
    "attribute vec3 vertex;               \n"
    "uniform mat4 mvp;                    \n"
    "varying vec3 texCoord;               \n"
    "void main() {                        \n"
    "    texCoord = vertex;               \n"
    "    gl_Position = vec4(vertex, 1.0); \n"
    "}                                    \n"
};

static const char *fshader_cube = {
    "precision mediump float;                        \n"
    "uniform samplerCube cube;                       \n"
    "varying vec3 texCoord;                          \n"
    "void main() {                                   \n"
    "    gl_FragColor = textureCube(cube, texCoord); \n"
    "}                                               \n"
};

static const char *vshader_code = {
    "attribute vec3 vertex;                                  \n"
    "attribute vec3 normal;                                  \n"
    "uniform   mat4 model;                                   \n"
    "uniform   mat4 view;                                    \n"
    "uniform   mat4 projection;                              \n"
    "varying vec3 vPos;                                      \n"
    "varying vec3 vNormal;                                   \n"
    "void main() {                                           \n"
    "    gl_Position =                                       \n"
    "        projection * view * model * vec4(vertex, 1.0) ; \n"
    "    vPos = (model * vec4(vertex, 1.0)).xyz;             \n"
    "    vNormal = (model * vec4(normal, 0.0)).xyz;          \n"
    "}                                                       \n"
};

static const char *fshader_code = {
    "precision mediump float;                        \n"
    "uniform samplerCube cube;                       \n"
    "uniform vec3 eyePos;                            \n"
    "varying vec3 vPos;                              \n"
    "varying vec3 vNormal;                           \n"
    "void main() {                                   \n"
    "    vec3 ref = reflect(vPos - eyePos, vNormal); \n"
    "    gl_FragColor = textureCube(cube, ref);      \n"
    "}                                               \n"
};

glm::vec3 eyePos = {0.0, 2.0, 5.0};

const GLfloat cubeVertices[] = {
    /* Front face */
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

    /* Back face */
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f
};

const GLuint cubeIndices[] = {
    /* Front face */
    0, 1, 2,
    2, 3, 0,

    /* Right face */
    1, 5, 6,
    6, 2, 1,

    /* Back face */
    7, 6, 5,
    5, 4, 7,

    /* Left face */
    4, 0, 3,
    3, 7, 4,

    /* Bottom face */
    0, 4, 5,
    5, 1, 0,

    /* Top face */
    3, 2, 6,
    6, 7, 3
};

const std::map<std::string, GLuint> cube_image_loc = {
    {"posX.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
    {"negX.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
    {"posY.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
    {"negY.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
    {"posZ.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
    {"negZ.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
};

void load_images(std::map<GLuint, unsigned char*> &images) {
    std::string prefix = "../Resources/cubemap/";
    int w, h, channel;
    for (auto img_loc : cube_image_loc) {
        std::string loc = img_loc.first;
        GLuint target = img_loc.second;
        images[target] = stbi_load((prefix+loc).c_str(), &w, &h, &channel, 0);
        if (images[target] == nullptr)
            std::cout << "Failed to get image: " << stbi_failure_reason() << std::endl;
    }
}

void free_images(std::map<GLuint, unsigned char*> &images) {
    for (auto itr = images.begin(); itr != images.end(); itr++) {
        stbi_image_free(itr->second);
    }
}

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
    glViewport(0, 0, w, h);
}

void init_cube(Model *model) {
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);
    /* generate buffers */
    glGenBuffers(3, model->bufs);
    /* get attributes */
    model->attr["vertex"] =
        glGetAttribLocation(model->program, "vertex");
    /* bind data to VBO */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cubeVertices),
                 cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* bind index data */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(cubeIndices),
                 cubeIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    /* get uniforms */
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
    model->uni["cube"] = glGetUniformLocation(model->program, "cube");
}

void init_model(Model *model, Mesh *mesh) {
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);
    /* generate buffers */
    glGenBuffers(3, model->bufs);
    /* get attributes */
    model->attr["vertex"] =
        glGetAttribLocation(model->program, "vertex");
    model->attr["normal"] =
        glGetAttribLocation(model->program, "normal");
    /* bind data to VBO */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->getVertexDataSize() * sizeof(float),
                 mesh->getVertexPos(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    model->uni["view"] = glGetUniformLocation(model->program, "view");
    model->uni["projection"] = glGetUniformLocation(model->program, "projection");
    model->uni["cube"] = glGetUniformLocation(model->program, "cube");
    model->uni["eyePos"] = glGetUniformLocation(model->program, "eyePos");
}

void init_texture(GLuint &texture) {
    std::map<GLuint, unsigned char*> cube_images;
    load_images(cube_images);
    /* generate texture and bind */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    /* copy image data to target */
    for (auto itr = cube_images.begin(); itr != cube_images.end(); itr++) {
        GLuint target = itr->first;
        unsigned char* img = itr->second;
        glTexImage2D(target, 0, GL_RGBA, 250, 250,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    }
    free_images(cube_images);
    /* generate mipmap */
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    /* parameter setup */
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* unbind texture */
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void redraw_cube(Context *context, Model *model, unsigned int cnt, GLuint texture) {
    float rad = M_PI * cnt / 180, aspect;
    glm::mat4 M, V, P, mvp;

    /* enable program */
    glUseProgram(model->program);
    /* Clear previous rendering */
    glClear(GL_COLOR_BUFFER_BIT);
    /* setup matrix */
    M = glm::identity<glm::mat4>();
    M = glm::scale(M, glm::vec3(200.0f, 200.0f, 200.0f));
    V = glm::lookAt(glm::vec3(1.0, 0.0, 0.0),
                    glm::vec3(0.0, 0.0, 0.0),
                    glm::vec3(0.0, 1.0, 0.0));
    P = glm::perspective(glm::radians(45.0f),
                         1.0f, 0.1f, 100.0f);
    mvp = P * V * M;
    /* enable attributes */
    glEnableVertexAttribArray(model->attr["vertex"]);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glVertexAttribPointer(model->attr["vertex"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    /* update MVP matrix */
    glUniformMatrix4fv(model->uni["mvp"], 1, GL_FALSE, &mvp[0][0]);
    /* bind texture */
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glUniform1i(model->uni["cube"], 0);
    /* enable element array buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[1]);
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLuint),
                   GL_UNSIGNED_INT, 0);
    /* unbind texture */
    glFlush();
}

void redraw(Context *context, Model *model, Mesh *mesh, GLuint &texture, int cnt) {
    float rad = M_PI * cnt / 180, aspect;
    unsigned int w, h;
    glm::mat4 M, V, P;

    /* enable program */
    glUseProgram(model->program);

    /* calculate aspect ration for projection matrix creation */
    context->getMode(w, h);
    aspect = static_cast<float>(h) / static_cast<float>(w);
    /* setup matrix */
    M = glm::identity<glm::mat4>();
    M = glm::rotate(M, rad, glm::vec3(0.0, 1.0, 0.0));
    V = glm::lookAt(glm::vec3(0.0, 0.0, 10.0),
                    glm::vec3(0.0, 0.0, 0.0),
                    glm::vec3(0.0, 1.0, 0.0));
    P = glm::perspective(glm::radians(45.0f),
                         1.0f, 0.1f, 100.0f);
    /* enable attributes */
    glEnableVertexAttribArray(model->attr["vertex"]);
    glEnableVertexAttribArray(model->attr["normal"]);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glVertexAttribPointer(model->attr["vertex"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glVertexAttribPointer(model->attr["normal"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    /* bind texture data */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glUniform1i(model->uni["cube"], 0);
    /* update mvp matrix in shader */
    glUniformMatrix4fv(model->uni["model"], 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(model->uni["view"], 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(model->uni["projection"], 1, GL_FALSE, &P[0][0]);
    glUniform3fv(model->uni["eyePos"], 1, &eyePos[0]);
    /* enable element array buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    glDrawElements(GL_TRIANGLES, mesh->getIndexSize(),
                   GL_UNSIGNED_INT, 0);
    /* unbind texture */
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glFlush();
}

int main() {
    ContextFactory factory;
    Context *context = nullptr;
    Model model_teapot, model_cube;
    Mesh* mesh_teapot = new AssimpMesh(TEAPOT_OBJ_PATH);
    GLuint texture;
    unsigned int cnt = 0;
    int terminate = 0;
#ifdef NATIVE_DISP_WAYLAND
    std::cout << "NativeDisp = Wayland\n";    
    context = factory.create(Wayland);
#elif NATIVE_DISP_DRM
    std::cout << "NativeDisp = Drm\n";    
    context = factory.create(Drm);
#endif
    assert(context);
    init_gl(context);
    /* init cube model */
    model_cube.shader_codes.push_back(vshader_cube);
    model_cube.shader_codes.push_back(fshader_cube);
    model_cube.bufs = new GLuint[2];
    init_cube(&model_cube);
    /* init teapot model */
    model_teapot.shader_codes.push_back(vshader_code);
    model_teapot.shader_codes.push_back(fshader_code);
    model_teapot.bufs = new GLuint[3];
    mesh_teapot->import();
    init_model(&model_teapot, mesh_teapot);
    /* load cube map texture data */
    init_texture(texture);
    while (cnt < LOOP_COUNT) {
        redraw_cube(context, &model_cube, cnt, texture);
        redraw(context, &model_teapot, mesh_teapot, texture, cnt);
        cnt++;
        context->swapBuffers();
    }

    /* free resources */
    delete context;
    delete[] model_cube.bufs;
    delete[] model_teapot.bufs;

    return 0;
}
