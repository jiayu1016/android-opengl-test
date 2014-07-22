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

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "layout(location = " STRV(POS_ATTRIB) ") in vec2 pos;\n"
    "layout(location=" STRV(COLOR_ATTRIB) ") in vec4 color;\n"
    "layout(location=" STRV(SCALEROT_ATTRIB) ") in vec4 scaleRot;\n"
    "layout(location=" STRV(OFFSET_ATTRIB) ") in vec2 offset;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    mat2 sr = mat2(scaleRot.xy, scaleRot.zw);\n"
    "    gl_Position = vec4(sr*pos + offset, 0.0, 1.0);\n"
    "    vColor = color;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "    outColor = vColor;\n"
    "}\n";

static const char vertexShaderCode[] =
    "#version 300 es\n"
    "layout(location = 0) in vec3 vPosition;\n"
    "layout(location = 1) in vec4 color;\n"
	"uniform mat4 vMVPMatrix;\n"
    "out vec4 vColor;\n"
    "void main() {\n" 
    "  gl_Position = vMVPMatrix * vec4(vPosition, 1.0f);\n"
	"  vColor = color;\n"
    "}\n";

static const char fragmentShaderCode[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec4 vColor;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  color= vColor;\n"
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
    GLuint mVertexColor;

	GLint mMVPMartixLoc;
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
    -0.8f, -0.8f, 0.0f,
     0.8f, -0.8f, 0.0f,
    -0.8f,  0.8f, 0.0f,
     0.8f,  0.8f, 0.0f,
};

static const GLfloat g_vertex_color_data[] = {
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
};


bool my_Renderer::init() {
	mProgram = createProgram(vertexShaderCode, fragmentShaderCode);
	if (!mProgram)
		return false;

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

	glGenBuffers(1, &mVertexColor);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_color_data), g_vertex_color_data, GL_STATIC_DRAW); 
	glVertexAttribPointer(
			COLOR_ATTRIB,       // attribute 1. No particular reason for 0, but must match the layout in the shader.
			4,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
	glEnableVertexAttribArray(COLOR_ATTRIB);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
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
	glDeleteBuffers(1, &mVertexColor);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), mAngles * (3.1415f / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUseProgram(mProgram);
	mMVPMartixLoc = glGetUniformLocation(mProgram, "vMVPMatrix");
	glUniformMatrix4fv(mMVPMartixLoc, 1, false, glm::value_ptr(rotation));
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
