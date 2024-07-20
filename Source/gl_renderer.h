#pragma once

#include <stdio.h>
#include <Windows.h>
#include <WindowsX.h>
#include <stdint.h>
#include <stdlib.h>
#include <gl/GL.h>

#include "Utility.h"

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>

#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_LINK_STATUS                    0x8B82
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1

#define GL_PROGRAM_POINT_SIZE             0x8642

#define GL_FUNC_ADD                       0x8006

typedef int64_t GLsizeiptr;

typedef GLuint(*glCreateShaderFunc)(GLenum shaderType);
extern glCreateShaderFunc glCreateShader;

typedef char GLchar;

typedef void(*glShaderSourceFunc)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
extern glShaderSourceFunc glShaderSource;

typedef void(*glCompileShaderFunc)(GLuint shader);
extern glCompileShaderFunc glCompileShader;

typedef void(*glGetShaderivFunc)(GLuint shader, GLenum pname, GLint* params);
extern glGetShaderivFunc glGetShaderiv;

typedef void(*glGetShaderInfoLogFunc)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
extern glGetShaderInfoLogFunc glGetShaderInfoLog;

typedef GLuint(*glCreateProgramFunc)(void);
extern glCreateProgramFunc glCreateProgram;

typedef void(*glDeleteProgramFunc)(GLuint program);
extern glDeleteProgramFunc glDeleteProgram;

typedef void(*glAttachShaderFunc)(GLuint program, GLuint shader);
extern glAttachShaderFunc glAttachShader;

typedef void(*glLinkProgramFunc)(GLuint program);
extern glLinkProgramFunc glLinkProgram;

typedef void(*glGetProgramivFunc)(GLuint program, GLenum pname, GLint* params);
extern glGetProgramivFunc glGetProgramiv;

typedef void(*glGetProgramInfoLogFunc)(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
extern glGetProgramInfoLogFunc glGetProgramInfoLog;

typedef void(*glDetachShaderFunc)(GLuint program, GLuint shader);
extern glDetachShaderFunc glDetachShader;

typedef void(*glDeleteShaderFunc)(GLuint shader);
extern glDeleteShaderFunc glDeleteShader;

typedef void(*glUseProgramFunc)(GLuint program);
extern glUseProgramFunc glUseProgram;

typedef void(*glGenVertexArraysFunc)(GLsizei n, GLuint* arrays);
extern glGenVertexArraysFunc glGenVertexArrays;

typedef void(*glGenBuffersFunc)(GLsizei n, GLuint* buffers);
extern glGenBuffersFunc glGenBuffers;

typedef void(*glVertexAttribPointerFunc)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
extern glVertexAttribPointerFunc glVertexAttribPointer;

typedef void(*glEnableVertexAttribArrayFunc)(GLuint index);
extern glEnableVertexAttribArrayFunc glEnableVertexAttribArray;

typedef void(*glDisableVertexAttribArrayFunc)(GLuint index);
extern glDisableVertexAttribArrayFunc glDisableVertexAttribArray;

typedef void(*glBindVertexArrayFunc)(GLuint array);
extern glBindVertexArrayFunc glBindVertexArray;

typedef void(*glBindBufferFunc)(GLenum target, GLuint buffer);
extern glBindBufferFunc glBindBuffer;

typedef void(*glBufferDataFunc)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
extern glBufferDataFunc glBufferData;

typedef GLuint(*glGetUniformLocationFunc)(GLuint program, const GLchar* name);
extern glGetUniformLocationFunc glGetUniformLocation;

typedef void(*glUniform1fFunc)(GLint location, GLfloat v0);
extern glUniform1fFunc glUniform1f;

typedef void (*glUniformMatrix4fvFunc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern glUniformMatrix4fvFunc glUniformMatrix4fv;

typedef HGLRC(WINAPI* wglCreateContextAttribsARBFunc) (HDC hDC, HGLRC hShareContext, const int* attribList);
extern wglCreateContextAttribsARBFunc wglCreateContextAttribsARB;

typedef void(*glActiveTextureFunc)(GLenum texture);
extern glActiveTextureFunc glActiveTexture;

typedef void(*glUniform1iFunc)(GLint location, GLint v0);
extern glUniform1iFunc glUniform1i;

typedef void(*glBlendEquationFunc)(GLenum mode);
extern glBlendEquationFunc glBlendEquation;

enum Shader_Program_Type {
	SPT_COLOR,
	SPT_TEXTURE,
	SPT_3D,
	SPT_3D_Lines,
	SPT_Cube_Face,
	SPT_TOTAL
};

void loadGLFunctions();

extern GLuint shader_program_types[SPT_TOTAL];
GLuint create_shader(const std::string shader_file_path, GLenum shader_type);
GLuint create_shader_program(const char* vertex_shader_file_path, const char* fragment_shader_file_path);
void load_shaders();