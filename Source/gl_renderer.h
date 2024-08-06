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

typedef intptr_t khronos_intptr_t;
typedef khronos_intptr_t GLintptr;
typedef void(*glBufferSubDataFunc)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
extern glBufferSubDataFunc glBufferSubData;

typedef void(*glDeleteBuffersFunc)(GLsizei n, const GLuint * buffers);
extern glDeleteBuffersFunc glDeleteBuffers;

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

void loadGLFunctions();

enum Shader_Program_Type {
	SPT_COLOR,
	SPT_TEXTURE,
	SPT_3D,
	SPT_3D_Lines,
	SPT_Cube_Face,
	SPT_String,
	SPT_Fireball,
	SPT_TOTAL
};

extern GLuint shader_program_types[SPT_TOTAL];
GLuint create_shader(const std::string shader_file_path, GLenum shader_type);
GLuint create_shader_program(const char* vertex_shader_file_path, const char* fragment_shader_file_path);
void load_shaders();

// Archived for now
#if 0 

	// 1 0 0 1
	// 0 1 0 3
	// 0 0 1 5
	// 0 0 0 1
	MX4 mx_one = translation_matrix_mx_4(1, 3, 5);
	// 1 0 0 7 
	// 0 1 0 9 
	// 0 0 1 11
	// 0 0 0 1
	MX4 mx_two = translation_matrix_mx_4(7, 9, 11);

	// 1 0 0 7   *   1 0 0 1    =    1 0 1 8
	// 0 1 0 9       0 1 0 3         0 1 0 12
	// 0 0 1 11      0 0 1 5         0 0 1 16
	// 0 0 0 1       0 0 0 1         0 0 0 1
	MX4 result_mx = mx_one * mx_two;

	V4 vec_one = { 2, 4, 6, 1 };

	V4 result_v4 = result_mx * vec_one;

	// RECT rect_temp = {};
	// GetClientRect(window, &rect_temp);

	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	// SDL_RenderClear(renderer);

	SDL_Rect clip_rect = { 100, 10, 500, 500 };
	// SDL_SetRenderDrawColor(renderer, 0, 100, 100, 255);
	// SDL_RenderFillRect(renderer, &clip_rect);
	// SDL_RenderSetClipRect(renderer, &clip_rect);

	SDL_Rect viewport_rect = { 100, 10, 500, 500 };
	// SDL_RenderSetViewport(renderer, &clip_rect);
	
	// draw_debug_images(renderer);

	// SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Rect castle_rect = { 100,400,200,200 };
	SDL_SetTextureAlphaMod(castle_infernal_image.texture, 155);
	// SDL_RenderCopy(renderer, castle_infernal_image.texture, NULL, &castle_rect);

	SDL_Rect azir_rect = { 200,400,200,200 };
	SDL_SetTextureColorMod(azir_image.texture, 255, 255, 255);
	SDL_SetTextureAlphaMod(azir_image.texture, 155);
	// SDL_RenderCopy(renderer, azir_image.texture, NULL, &azir_rect);

	draw_chunks(renderer);

	// draw_wire_frame(renderer, { 0,0,0 }, 2, 1, 1);
	mp_draw_line_3d(renderer, { 0,0,0 }, { 1000, 1000, 1000});

	MP_Rect_3D rect;
	rect.bottom_left = { 0,0,0 };
	rect.bottom_right = { 10,0,0 };
	rect.top_right = { 10,10,0 };
	rect.top_left = { 10,0,0 };
	// mp_draw_cube_face(renderer, rect, nullptr);

	SDL_RenderPresent(renderer);
#endif

// Before while loop
#if 0
	Vertex vertices[6];
	// ***First Square - Left side***
	// First Triangle
	// Bottom Left
	vertices[0].pos = { -0.8f, -0.5f }; 
	vertices[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertices[0].uv = { 0.0f, 0.0f };
	// Top Left
	vertices[1].pos = { -0.8f, 0.5f }; 
	vertices[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertices[1].uv = { 0.0f, 1.0f };
	// Top Right
	vertices[2].pos = { -0.2f, 0.5f };
	vertices[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	vertices[2].uv = { 1.0f, 1.0f };

	// Second Triangle
	// Bottom Left
	vertices[3].pos = { -0.8f, -0.5f };
	vertices[3].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertices[3].uv = { 0.0f, 0.0f };
	// Bottom Right
	vertices[4].pos = { -0.2f, -0.5f };
	vertices[4].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertices[4].uv = { 1.0f, 0.0f };
	// Top Right
	vertices[5].pos = { -0.2f, 0.5f };
	vertices[5].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	vertices[5].uv = { 1.0f, 1.0f };

	GLuint vbo, vao;
	glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	Vertex vertices_two[6];
	// ***Second Square - Right side***
	// First Triangle
	// Bottom Left
	vertices_two[0].pos = { 0.2f, -0.5f }; 
	vertices_two[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertices_two[0].uv = { 0.0f, 0.0f };
	// Top Left
	vertices_two[1].pos = { 0.2f, 0.5f }; 
	vertices_two[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertices_two[1].uv = { 0.0f, 1.0f };
	// Top Right
	vertices_two[2].pos = { 0.8f, 0.5f };
	vertices_two[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	vertices_two[2].uv = { 1.0f, 1.0f };

	// Second Triangle
	// Bottom Left
	vertices_two[3].pos = { 0.2f, -0.5f }; 
	vertices_two[3].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	vertices_two[3].uv = { 0.0f, 0.0f };
	// Bottom Right
	vertices_two[4].pos = { 0.8f, -0.5f };
	vertices_two[4].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	vertices_two[4].uv = { 1.0f, 0.0f };
	// Top Right
	vertices_two[5].pos = { 0.8f, 0.5f };
	vertices_two[5].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	vertices_two[5].uv = { 1.0f, 1.0f };

	GLuint vbo_two;
    glGenBuffers(1, &vbo_two);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_two);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), vertices_two, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	const char* vertex_shader_file_path = "Shaders\\vertex_shader.txt";
	const char* fragment_shader_file_path = "Shaders\\fragment_shader.txt";
	GLuint shader_program = create_shader_program(vertex_shader_file_path, fragment_shader_file_path);
	
	glUseProgram(shader_program);
	GLint tex_Diffuse_Location = glGetUniformLocation(shader_program, "texDiffuse");
	if (tex_Diffuse_Location != -1) {
		glUniform1i(tex_Diffuse_Location, 0);
	}
	GLint tex_Diffuse_2_Location = glGetUniformLocation(shader_program, "texDiffuse_2");
	if (tex_Diffuse_2_Location != -1) {
		glUniform1i(tex_Diffuse_2_Location, 1);
	}
	Texture temp = load_Texture_Data("assets\\azir.jpg");
	Texture temp_2 = load_Texture_Data("assets\\smolder.jpg");

	Texture castle_Infernal = load_Texture_Data("assets\\castle_Infernal.png");
	Texture water_Sprite = load_Texture_Data("assets\\water_Sprite.jpg");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	
// In main loop
#if 0
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, castle_Infernal.handle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, water_Sprite.handle);

		GLint offset_X_Location = glGetUniformLocation(shader_program, "u_uv_Offset_X");
		GLint offset_Y_Location = glGetUniformLocation(shader_program, "u_uv_Offset_Y");
		if (offset_X_Location != -1 && offset_Y_Location != -1) {
			static float x = 0;
			static float y = 0;
			x += 0.01f;
			y += 0.02f;
			glUniform1f(offset_X_Location, x);
			glUniform1f(offset_Y_Location, y);
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_two);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		SwapBuffers(hdc);
#endif