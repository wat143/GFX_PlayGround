#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <GLES2/gl2.h>

#include "Utils.h"
#include "glUtils.h"
#include "ContextFactory.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define LOOP_COUNT 1000000

static const char *vshader_code = {
    "attribute vec3 vertex;                              \n"
    "attribute vec4 color;                               \n"
    "uniform   mat4 mvp;                                 \n"
    "varying   vec4 vColor;                              \n"
    "void main() {                                       \n"
    "    gl_Position = mvp * vec4(vertex, 1.0) ;         \n"
    "    vColor = color;                                 \n"
    "}                                                   \n"
};

static const char *fshader_code = {
    "precision mediump float;                            \n"
    "varying vec4 vColor;                                \n"
    "void main() {                                       \n"
    "    gl_FragColor = vColor;                          \n"
    "}                                                   \n"
};

static const char *tex_vshader_code = {
    "attribute vec3 vertex;                              \n"
    "attribute vec2 texCoord;                            \n"
    "uniform mat4 mvp;                                   \n"
    "varying vec2 vert_texCoord;                         \n"
    "void main() {                                       \n"
    "    vert_texCoord = texCoord;                       \n"
    "    gl_Position = mvp * vec4(vertex, 1.0);          \n"
    "}                                                   \n"
};

static const char *tex_fshader_code = {
    "precision mediump float;                            \n"
    "uniform sampler2D texture;                          \n"
    "varying vec2 vert_texCoord;                         \n"
    "void main() {                                       \n"
    "  gl_FragColor = vec4(0.3, 0.3, 0.3, 1.0)           \n"
    "               + texture2D(texture, vert_texCoord); \n"
    "}                                                   \n"
};

const GLfloat vertex[] = {
     0.0,  1.0, 0.0,
    -0.5, -1.0, 0.0,
     0.5, -1.0, 0.0
};

const GLfloat color1[] = {
    1.0, 0.0, 0.0, 1.0,
    1.0, 1.0, 0.0, 1.0,
    1.0, 0.0, 1.0, 1.0
};

const GLfloat color2[] = {
    1.0, 0.0, 1.0, 1.0,
    0.0, 1.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0
};

const GLfloat color3[] = {
    1.0, 0.0, 1.0, 0.6,
    0.0, 1.0, 1.0, 0.6,
    0.0, 0.0, 1.0, 0.6,
    1.0, 0.0, 1.0, 0.6
};

const GLfloat rect_vertex[] = {
    -1.0,  1.0, 0.0,
     1.0,  1.0, 0.0,
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0
};

const GLfloat rect_vertex2[] = {
    -1.0,  1.0, 0.5,
     1.0,  1.0, 0.5,
    -1.0, -1.0, 0.5,
     1.0, -1.0, 0.5
};

const GLfloat tex_coord[] = {
    1.0, 1.0,
    0.0, 1.0,
    1.0, 0.0,
    0.0, 0.0
};

const GLuint index[] = {
    0, 1, 2,
    3, 2, 1
};

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
        printShaderLog(shader, VERTEX_SHADER);
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

void init_triangle(Context *context, Model *model)
{
    GLint status;
    unsigned int w, h;
    /* Enable depth test */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);
    /* create VBO and pass attributes to GPU */
    glGenBuffers(3, model->bufs);
    /* set viewport */
    context->getMode(w, h);
    glViewport(0, 0, w, h);
    std::cout << "width, height = " << w << ", " << h << std::endl;
    /* Get attribute locations in vertex shader */
    model->attr["vertex"] = glGetAttribLocation(model->program, "vertex");
    model->attr["color"] = glGetAttribLocation(model->program, "color");

    /* bind VBO data */
    /* vertex position */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex),
                 &vertex[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* color1 to buffer */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color1),
                 &color1[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* color2 to buffer */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color2),
                 &color2[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* get uniform location */
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
}

void init_tex(Context *context, Model *model) {
    GLint status;
    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);

    /* get attribute locations */
    model->attr["vertex"] =
        glGetAttribLocation(model->program, "vertex");
    model->attr["texCoord"] =
        glGetAttribLocation(model->program, "texCoord");

    /* create VBOs and IBO and transfer data */
    /* vertex position */
    glGenBuffers(3, model->bufs);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertex),
                 &rect_vertex[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* texture coordinate */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coord),
                 &tex_coord[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index),
                 &index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    /* get uniforms */
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
    model->uni["texture"] = 
        glGetUniformLocation(model->program, "texture");
}

void init_rect(Context *context, Model *model) {
    GLint status;

    /* compile shader and link program */
    build_program(model->shader_codes, model->program);
    glUseProgram(model->program);

    /* get attribute locations */
    model->attr["vertex"] =
        glGetAttribLocation(model->program, "vertex");
    model->attr["color"] =
        glGetAttribLocation(model->program, "color");

    /* create VBOs and IBO and transfer data */
    /* vertex position */
    glGenBuffers(3, model->bufs);
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertex2),
                 &rect_vertex2[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* texture coordinate */
    glBindBuffer(GL_ARRAY_BUFFER, model->bufs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color3),
                 &color3[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /* index buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->bufs[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index),
                 &index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    /* get uniforms */
    model->uni["mvp"] = glGetUniformLocation(model->program, "mvp");
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

void redraw_triangle(Context *context, Model *model_t,
                     GLuint framebuffer, int cnt)
{
    std::map<std::string, GLuint> attr = model_t->attr;
    std::map<std::string, GLuint> uni = model_t->uni;
    GLuint *bufs = model_t->bufs;
    float rad = M_PI * cnt / 180, aspect;
    glm::mat4 model, view, projection, mvp;

    /* render to generated framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    /* use program for triangles */
    glUseProgram(model_t->program);
    /* Clear previous rendering */
    glClearDepthf(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* calculate aspect ration for projection matrix creation */
    aspect = 1.0f;
    /* setup matrix */
    view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0),
                       glm::vec3(0.0, 0.0, 0.0),
                       glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective(glm::radians(45.0f),
                                  aspect, 0.1f, 100.0f);

    /* Enable attribute arrays */
    glEnableVertexAttribArray(model_t->attr["vertex"]);
    glEnableVertexAttribArray(model_t->attr["color"]);
    /* Bind vertex position to attribute "vertex" */
    glBindBuffer(GL_ARRAY_BUFFER, model_t->bufs[0]);
    glVertexAttribPointer(model_t->attr["vertex"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    /* 1st Triangle */
    /* claculate MVP matrix and bind */
    model = glm::identity<glm::mat4>();
    model = glm::translate(model, glm::vec3(0.0, cos(rad), sin(rad)));
    model = glm::rotate(model, rad, glm::vec3(0.0, 1.0, 0.0));
    mvp = projection * view * model;
    glUniformMatrix4fv(uni["mvp"], 1, GL_FALSE, &mvp[0][0]);

    /* update attribute "color" */
    glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
    glVertexAttribPointer(attr["color"], 4, GL_FLOAT, GL_TRUE, 0, 0);

    /* draw 1st triangle */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* 2nd Triangle */
    /* claculate MVP matrix and bind */
    model = glm::identity<glm::mat4>();
    model =
        glm::translate(model, glm::vec3(cos(rad), 0.0, -1 * sin(rad)));
    model = glm::rotate(model, rad, glm::vec3(1.0, 0.0, 0.0));
    mvp = projection * view * model;
    glUniformMatrix4fv(uni["mvp"], 1, GL_FALSE, &mvp[0][0]);

    /* update attribute "color" */
    glBindBuffer(GL_ARRAY_BUFFER, bufs[2]);
    glVertexAttribPointer(attr["color"], 4, GL_FLOAT, GL_TRUE, 0, 0);

    /* draw 2nd trignale */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* disable attribute arrays */
    glDisableVertexAttribArray(model_t->attr["vertex"]);
    glDisableVertexAttribArray(model_t->attr["color"]);

    /* flush commands and wait completion */
    glFlush();
    glFinish();
}

void redraw_tex(Context *context, Model *model_t, GLuint tex, int cnt) {
    std::map<std::string, GLuint> uni = model_t->uni;
    GLuint *bufs = model_t->bufs;
    float rad = M_PI * cnt / 180, aspect;
    glm::mat4 model, view, projection, mvp;

    /* bind default framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* use program for texture */
    glUseProgram(model_t->program);

    /* Clear previous rendering */
    glClearDepthf(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* setup MVP matrix */
    /* calculate aspect ration for projection matrix creation */
    aspect = 1.0f;
    view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0),
                       glm::vec3(0.0, 0.0, 0.0),
                       glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective(glm::radians(45.0f),
                                  aspect, 0.1f, 100.0f);
    model = glm::identity<glm::mat4>();
    model = glm::translate(model, glm::vec3(0.0, cos(rad), 0.0));
    model = glm::rotate(model, rad, glm::vec3(0.0, 1.0, 0.0));
    mvp = projection * view * model;

    /* Enable attribute arrays */
    glEnableVertexAttribArray(model_t->attr["vertex"]);
    glEnableVertexAttribArray(model_t->attr["texCoord"]);

    /* Bind vertex position and texture coordinate */
    glBindBuffer(GL_ARRAY_BUFFER, model_t->bufs[0]);
    glVertexAttribPointer(model_t->attr["vertex"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model_t->bufs[1]);
    glVertexAttribPointer(model_t->attr["texCoord"], 2,
                          GL_FLOAT, GL_TRUE, 0, 0);

    /* pass uniforms */
    /* bind MVP matrix */
    glUniformMatrix4fv(uni["mvp"], 1, GL_FALSE, &mvp[0][0]);

    /* bind texture rendered by redraw_triangle() */
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uni["texture"], 0);

    /* bind VIO and draw rectangle */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model_t->bufs[2]);
    glDrawElements(GL_TRIANGLES, sizeof(index) / sizeof(GLuint),
                   GL_UNSIGNED_INT, 0);

    /* unbind vio and texture */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* disable attribute arrays */
    glDisableVertexAttribArray(model_t->attr["vertex"]);
    glDisableVertexAttribArray(model_t->attr["texCoord"]);

    /* flush commands */
    glFlush();
}

void redraw_rect(Context *context, Model *model_t, int cnt) {
    std::map<std::string, GLuint> uni = model_t->uni;
    GLuint *bufs = model_t->bufs;
    float rad = M_PI * cnt / 180, aspect;
    glm::mat4 model, view, projection, mvp;

    /* enable alpha blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* bind default framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* use program for texture */
    glUseProgram(model_t->program);

    /* setup MVP matrix */
    /* calculate aspect ration for projection matrix creation */
    aspect = 1.0f;
    view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0),
                       glm::vec3(0.0, 0.0, 0.0),
                       glm::vec3(0.0, 1.0, 0.0));
    projection = glm::perspective(glm::radians(45.0f),
                                  aspect, 0.1f, 100.0f);
    model = glm::identity<glm::mat4>();
    model = glm::translate(model, glm::vec3(cos(rad), 0.0, 0.0));
    mvp = projection * view * model;

    /* Enable attribute arrays */
    glEnableVertexAttribArray(model_t->attr["vertex"]);
    glEnableVertexAttribArray(model_t->attr["color"]);

    /* Bind vertex position and texture coordinate */
    glBindBuffer(GL_ARRAY_BUFFER, model_t->bufs[0]);
    glVertexAttribPointer(model_t->attr["vertex"], 3,
                          GL_FLOAT, GL_TRUE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, model_t->bufs[1]);
    glVertexAttribPointer(model_t->attr["color"], 4,
                          GL_FLOAT, GL_TRUE, 0, 0);

    /* pass uniforms */
    /* bind MVP matrix */
    glUniformMatrix4fv(uni["mvp"], 1, GL_FALSE, &mvp[0][0]);

    /* bind VIO and draw rectangle */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model_t->bufs[2]);
    glDrawElements(GL_TRIANGLES, sizeof(index) / sizeof(GLuint),
                   GL_UNSIGNED_INT, 0);

    /* unbind vio and texture */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /* disable attribute arrays */
    glDisableVertexAttribArray(model_t->attr["vertex"]);
    glDisableVertexAttribArray(model_t->attr["color"]);

    /* disable alpha blending */
    glDisable(GL_BLEND);

    /* flush commands */
    glFlush();
}

int main() {
    ContextFactory factory;
    Context *context = nullptr;
    Model model_triangle, model_tex, model_rect;
    GLuint texture, fb;
    int cnt = 0;

    std::cout << "alphaBlend\n";
#ifdef NATIVE_DISP_WAYLAND
    std::cout << "NativeDisp = Wayland\n";    
    context = factory.create(Wayland);
#elif NATIVE_DISP_DRM
    std::cout << "NativeDisp = Drm\n";    
    context = factory.create(Drm);
#endif
    assert(context);

    /*** init model ***/
    /* triangle model */
    model_triangle.shader_codes.push_back(vshader_code);
    model_triangle.shader_codes.push_back(fshader_code);
    model_triangle.bufs = new GLuint[3];
    init_triangle(context, &model_triangle);

    /* texture model */
    model_tex.shader_codes.push_back(tex_vshader_code);
    model_tex.shader_codes.push_back(tex_fshader_code);
    model_tex.bufs = new GLuint[3];
    init_tex(context, &model_tex);

    /* rectangle model */
    model_rect.shader_codes.push_back(vshader_code);
    model_rect.shader_codes.push_back(fshader_code);
    model_rect.bufs = new GLuint[3];
    init_rect(context, &model_rect);

    /* init texture and framebuffer */
    init_fb(context, texture, fb);

    while (cnt < LOOP_COUNT) {
        /* render to texture */
        redraw_triangle(context, &model_triangle, fb, cnt);
        /* render to default framebuffer */
        redraw_tex(context, &model_tex, texture, cnt);
        redraw_rect(context, &model_rect, cnt);
        context->swapBuffers();
        cnt++;
    }

    /* free resources */
    delete[] model_triangle.bufs;
    delete[] model_tex.bufs;
    delete[] model_rect.bufs;
    delete context;

    return 0;
}
