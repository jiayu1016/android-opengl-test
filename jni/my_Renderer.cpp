/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define GLM_FORCE_RADIANS
#include "gles3jni.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

extern "C" {
#include <time.h>
#include <unistd.h>
#include "image.h"
}

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define TEX_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char vertexShaderCode[] =
    "#version 300 es\n"
    "layout(location = 0) in vec3 vPosition;\n"
    "layout(location = 1) in vec2 vTexPos;\n"
    "uniform mat4 vMVPMatrix;\n"
    "out vec2 fTexPos;\n"
    "void main() {\n" 
    "  gl_Position = vMVPMatrix * vec4(vPosition, 1.0f);\n"
    "  fTexPos = vTexPos;\n"
    "}\n";

static const char fragmentShaderCode[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform sampler2D u_TextureUnit;\n"
    "in vec2 fTexPos;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  color = texture(u_TextureUnit, fTexPos);\n"
    "}\n";

class my_Renderer: public Renderer {
public:
    my_Renderer();
    virtual ~my_Renderer();
    bool init();

private:
    enum {VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT};

    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVertexBuffer;

    GLuint mVertexTex;
	GLuint mTexture0;
	GLuint mTextUnitLoc0;

	GLint mMVPMartixLoc;
	glm::mat4 mP;
	glm::mat4 mV;
	float width;
	float height;
	float ratio;
};

Renderer* createMyRenderer() {
    my_Renderer* renderer = new my_Renderer;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

my_Renderer::my_Renderer()
:   mEglContext(eglGetCurrentContext()),
    mProgram(0)
{
}

// An array of 4 vectors which represents 4 vertices
static const GLfloat g_vertex_buffer_data[] = {
	/*
    -0.8f, -0.4f, 0.0f,
     0.8f, -0.4f, 0.0f,
    -0.8f,  0.4f, 0.0f,
     0.8f,  0.4f, 0.0f,
	 */
    -0.9f, -0.9f, 0.0f,
     0.9f, -0.9f, 0.0f,
    -0.9f,  0.9f, 0.0f,
     0.9f,  0.9f, 0.0f,
};

static const GLfloat g_vertex_texture_coordinate[] = {
	0.0f, 1.0f,     // 0 bottom left
	1.0f, 1.0f,     // 1 bottom right
	0.0f, 0.0f,     // 2 top left
	1.0f, 0.0f      // 3 top right
};

static const GLfloat g_vertex_color_data[] = {
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
};

long current_time_msec(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec*1000L + now.tv_nsec/1000000L;
}

bool my_Renderer::init() {
	mProgram = createProgram(vertexShaderCode, fragmentShaderCode);
	if (!mProgram)
		return false;

	glUseProgram(mProgram);

	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW); 
	glVertexAttribPointer(
			POS_ATTRIB,         // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
	glEnableVertexAttribArray(POS_ATTRIB);

	mTexture0 = load_texture_from_png_file("/storage/sdcard0/Pictures/zj.png");
	mTextUnitLoc0 = glGetUniformLocation(mProgram, "u_TextureUnit");
	glGenBuffers(1, &mVertexTex);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexTex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_texture_coordinate), g_vertex_texture_coordinate, GL_STATIC_DRAW); 
	glVertexAttribPointer(
			TEX_ATTRIB,         // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
	glEnableVertexAttribArray(TEX_ATTRIB);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	GLint tmp[4];
	glGetIntegerv(GL_VIEWPORT, tmp);
	width = tmp[2];
	height = tmp[3];
	ratio = height / width;
	mP = glm::ortho(-1.f, 1.f, -ratio, ratio, -1.0f, 1.0f);
//	mP = glm::ortho((float)(-width) / 2.0f, (float)width / 2.0f, (float)(-height) / 2.0f, (float)height / 2.0f);
//	mP = glm::perspective(30.f, (float)tmp[2] / (float)tmp[3], 0.1f, 100.f);
	mV = glm::lookAt(glm::vec3(0.f, 0.0f, 1.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.0f, 0.0f));

	ALOGV("Using OpenGL ES 3.0 renderer ");
	return true;
}

my_Renderer::~my_Renderer() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDisable(GL_CULL_FACE);
    glDeleteBuffers(1, &mVertexTex);
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteProgram(mProgram);
}

float* my_Renderer::mapOffsetBuf() {
	return NULL;
}

void my_Renderer::unmapOffsetBuf() {
}

float* my_Renderer::mapTransformBuf() {
	return NULL;
}

void my_Renderer::unmapTransformBuf() {
}

void my_Renderer::draw(unsigned int numInstances) {
	long before = current_time_msec();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), mAngles * (3.1415f / 180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) ;
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.f/ratio, 1.0f, 1.f));
	glm::mat4 mM = rotation * scale;
	glm::mat4 MVP = mP * mV * mM;
	glUseProgram(mProgram);
	mMVPMartixLoc = glGetUniformLocation(mProgram, "vMVPMatrix");
	glUniformMatrix4fv(mMVPMartixLoc, 1, false, glm::value_ptr(MVP));
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture0);
	glUniform1i(mTextUnitLoc0, 0);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Starting from vertex 0; 3 vertices total -> 1 triangle
	long after = current_time_msec();
	ALOGE("lskakaxi, draw cost %ld ms!", after - before);
}
