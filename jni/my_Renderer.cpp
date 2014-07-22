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

#include "gles3jni.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include "glm/glm.hpp"

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
	"layout(location = 0) in vec4 vPosition;\n"
    "layout(location=" STRV(COLOR_ATTRIB) ") in vec4 color;\n"
    "out vec4 vColor;\n"
	"void main() {\n" 
	"  gl_Position = vPosition;\n"
    "  vColor = color;\n"
	"}\n";

static const char fragmentShaderCode[] =
    "#version 300 es\n"
    "precision mediump float;\n"
	"out vec4 color;\n"
	"void main() {\n"
	"  color= vec4(1,1,0,0);\n"
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
    GLuint vertexbuffer;
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

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f,
};

bool my_Renderer::init() {
	mProgram = createProgram(vertexShaderCode, fragmentShaderCode);
	if (!mProgram)
		return false;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW); 

	ALOGV("Using OpenGL ES 3.0 renderer");
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
	glUseProgram(mProgram);
	glEnableVertexAttribArray(0);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

	glDisableVertexAttribArray(0);
}
