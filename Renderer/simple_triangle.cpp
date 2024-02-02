#include <iostream>
#include <map>
#include <cmath>
#include <GLES2/gl2.h>

#include "ContextFactory.h"
#include "glUtils.h"
#include "Utils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define LOOP_COUNT 1000000

static const char *vshader_code = {
    "attribute vec3 vertex;                      \n"
    "attribute vec3 color;                       \n"
    "uniform   mat4 mvp;                         \n"
    "varying   vec4 vColor;                      \n"
    "void main() {                               \n"
    "    gl_Position = mvp * vec4(vertex, 1.0) ; \n"
    "    vColor = vec4(color, 1.0);              \n"
    "}                                           \n"
};

static const char *fshader_code = {
    "precision mediump float;                    \n"
    "varying vec4 vColor;                        \n"
    "void main() {                               \n"
    "    gl_FragColor = vColor;                  \n"
    "}                                           \n"
};

const GLfloat vertex[] = {
     0.0,  1.0, 0.0,
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0
};

const GLfloat color[] = {
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0
};

void createShader(GLuint &vert_shader, GLuint &frag_shader, GLuint &program) {
    GLint status;
    /* vertex shader */
    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, (const char**)&vshader_code, nullptr);
    glCompileShader(vert_shader);
    /* check compile result */
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &status);
    if (!status)
        printShaderLog(vert_shader, VERTEX_SHADER);
    /* vertex shader */
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, (const char**)&fshader_code, nullptr);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &status);
    if (!status)
        printShaderLog(frag_shader, FRAGMENT_SHADER);
    program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glUseProgram(program);
}

void init_gl(Context *context,
            std::map<std::string, GLuint> &attr,
            std::map<std::string, GLuint> &uni,
            GLuint &program, GLuint *bufs) {
    GLuint vert_shader, frag_shader;
    GLint status, attrPos;
    unsigned int w, h;
    /* compile shader and link program */
    createShader(vert_shader, frag_shader, program);
    /* create VBO and pass attributes to GPU */
    glGenBuffers(2, bufs);
    /* set viewport */
    context->getMode(w, h);
    glViewport(0, 0, w, h);
    std::cout << "width, height = " << w << ", " << h << std::endl;
    /* attach vertex position */
    glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), &vertex[0], GL_STATIC_DRAW);
    attr["vertex"] = glGetAttribLocation(program, "vertex");
    glVertexAttribPointer(attr["vertex"], 3, GL_FLOAT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(attr["vertex"]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /*attach vertex color */
    glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), &color[0], GL_STATIC_DRAW);
    attr["color"] = glGetAttribLocation(program, "color");
    glVertexAttribPointer(attr["color"], 3, GL_FLOAT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(attr["color"]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* get uniform location */
    uni["mvp"] = glGetUniformLocation(program, "mvp");
}

void redraw(Context *context, std::map<std::string, GLuint> uni, int cnt) {
    float rad = M_PI * cnt / 180, aspect;
    unsigned int w, h;
    glm::mat4 model, view, projection, mvp;

    /* Clear previous rendering */
    glClear(GL_COLOR_BUFFER_BIT);

    /* calculate aspect ration for projection matrix creation */
    context->getMode(w, h);
    aspect = static_cast<float>(h) / static_cast<float>(w);
    /* setup matrix */
    model = glm::identity<glm::mat4>();
    model = glm::rotate(model, rad, glm::vec3(0.0, 1.0, 0.0));
    view = glm::lookAt(glm::vec3(0.0, 2.0, 5.0),
                       glm::vec3(0.0, 0.0, 0.0),
                       glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective(glm::radians(45.0f),
                                  aspect, 0.1f, 100.0f);
    mvp = projection * view * model;
    /* update mvp matrix in shader */
    glUniformMatrix4fv(uni["mvp"], 1, GL_FALSE, &mvp[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glFlush();
}

int main() {
    ContextFactory factory;
    Context *context = nullptr;
    GLuint program;
    GLuint *bufs = nullptr;
    int cnt = 0;
    std::map<std::string, GLuint> attr, uni;
    std::cout << "HelloTriangle\n";
#ifdef NATIVE_DISP_WAYLAND
    std::cout << "NativeDisp = Wayland\n";    
    context = factory.create(Wayland);
#elif NATIVE_DISP_DRM
    std::cout << "NativeDisp = Drm\n";    
    context = factory.create(Drm);
#endif
    assert(context);
    bufs = new GLuint[3];
    init_gl(context, attr, uni, program, bufs);
    while (cnt < LOOP_COUNT) {
        redraw(context, uni, cnt++);
        context->swapBuffers();
    }

    /* free resources */
    delete context;
    delete[] bufs;

    return 0;
}
