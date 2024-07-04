#include "Renderer.h"

#include "Utility.h"

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>

#include <gl/GL.h>

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
glCreateShaderFunc glCreateShader = {};

typedef char GLchar;

typedef void(*glShaderSourceFunc)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
glShaderSourceFunc glShaderSource = {};

typedef void(*glCompileShaderFunc)(GLuint shader);
glCompileShaderFunc glCompileShader = {};

typedef void(*glGetShaderivFunc)(GLuint shader, GLenum pname, GLint* params);
glGetShaderivFunc glGetShaderiv = {};

typedef void(*glGetShaderInfoLogFunc)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
glGetShaderInfoLogFunc glGetShaderInfoLog = {};

typedef GLuint(*glCreateProgramFunc)(void);
glCreateProgramFunc glCreateProgram = {};

typedef void(*glDeleteProgramFunc)(GLuint program);
glDeleteProgramFunc glDeleteProgram = {};

typedef void(*glAttachShaderFunc)(GLuint program, GLuint shader);
glAttachShaderFunc glAttachShader = {};

typedef void(*glLinkProgramFunc)(GLuint program);
glLinkProgramFunc glLinkProgram = {};

typedef void(*glGetProgramivFunc)(GLuint program, GLenum pname, GLint* params);
glGetProgramivFunc glGetProgramiv = {};

typedef void(*glGetProgramInfoLogFunc)(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
glGetProgramInfoLogFunc glGetProgramInfoLog = {};

typedef void(*glDetachShaderFunc)(GLuint program, GLuint shader);
glDetachShaderFunc glDetachShader = {};

typedef void(*glDeleteShaderFunc)(GLuint shader);
glDeleteShaderFunc glDeleteShader = {};

typedef void(*glUseProgramFunc)(GLuint program);
glUseProgramFunc glUseProgram = {};

typedef void(*glGenVertexArraysFunc)(GLsizei n, GLuint* arrays);
glGenVertexArraysFunc glGenVertexArrays = {};

typedef void(*glGenBuffersFunc)(GLsizei n, GLuint* buffers);
glGenBuffersFunc glGenBuffers = {};

typedef void(*glVertexAttribPointerFunc)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
glVertexAttribPointerFunc glVertexAttribPointer = {};

typedef void(*glEnableVertexAttribArrayFunc)(GLuint index);
glEnableVertexAttribArrayFunc glEnableVertexAttribArray = {};

typedef void(*glBindVertexArrayFunc)(GLuint array);
glBindVertexArrayFunc glBindVertexArray = {};

typedef void(*glBindBufferFunc)(GLenum target, GLuint buffer);
glBindBufferFunc glBindBuffer = {};

typedef void(*glBufferDataFunc)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
glBufferDataFunc glBufferData = {};

typedef GLuint(*glGetUniformLocationFunc)(GLuint program, const GLchar* name);
glGetUniformLocationFunc glGetUniformLocation = {};

typedef void(*glUniform1fFunc)(GLint location, GLfloat v0);
glUniform1fFunc glUniform1f = {};

typedef HGLRC(WINAPI* wglCreateContextAttribsARBFunc) (HDC hDC, HGLRC hShareContext, const int* attribList);
wglCreateContextAttribsARBFunc wglCreateContextAttribsARB = {};

typedef void(*glActiveTextureFunc)(GLenum texture);
glActiveTextureFunc glActiveTexture = {};

typedef void(*glUniform1iFunc)(GLint location, GLint v0);
glUniform1iFunc glUniform1i = {};

typedef void(*glBlendEquationFunc)(GLenum mode);
glBlendEquationFunc glBlendEquation = {};

void loadGLFunctions() {
	glCreateShader = (glCreateShaderFunc)wglGetProcAddress("glCreateShader");
	glShaderSource = (glShaderSourceFunc)wglGetProcAddress("glShaderSource");
	glCompileShader = (glCompileShaderFunc)wglGetProcAddress("glCompileShader");
	glGetShaderiv = (glGetShaderivFunc)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (glGetShaderInfoLogFunc)wglGetProcAddress("glGetShaderInfoLog");

	glCreateProgram = (glCreateProgramFunc)wglGetProcAddress("glCreateProgram");

	glDeleteProgram = (glDeleteProgramFunc)wglGetProcAddress("glDeleteProgram");
	glAttachShader = (glAttachShaderFunc)wglGetProcAddress("glAttachShader");
	glLinkProgram = (glLinkProgramFunc)wglGetProcAddress("glLinkProgram");
	glGetProgramiv = (glGetProgramivFunc)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (glGetProgramInfoLogFunc)wglGetProcAddress("glGetProgramInfoLog");
	glDetachShader = (glDetachShaderFunc)wglGetProcAddress("glDetachShader");
	glDeleteShader = (glDeleteShaderFunc)wglGetProcAddress("glDeleteShader");
	glUseProgram = (glUseProgramFunc)wglGetProcAddress("glUseProgram");

	glGenVertexArrays = (glGenVertexArraysFunc)wglGetProcAddress("glGenVertexArrays");
	glGenBuffers = (glGenBuffersFunc)wglGetProcAddress("glGenBuffers");
	glVertexAttribPointer = (glVertexAttribPointerFunc)wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (glEnableVertexAttribArrayFunc)wglGetProcAddress("glEnableVertexAttribArray");
	glBindVertexArray = (glBindVertexArrayFunc)wglGetProcAddress("glBindVertexArray");
	glBindBuffer = (glBindBufferFunc)wglGetProcAddress("glBindBuffer");
	glBufferData = (glBufferDataFunc)wglGetProcAddress("glBufferData");

	glGetUniformLocation = (glGetUniformLocationFunc)wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (glUniform1fFunc)wglGetProcAddress("glUniform1f");
	glUniform1i = (glUniform1iFunc)wglGetProcAddress("glUniform1i");

	wglCreateContextAttribsARB = (wglCreateContextAttribsARBFunc)wglGetProcAddress("wglCreateContextAttribsARB");

	glActiveTexture = (glActiveTextureFunc)wglGetProcAddress("glActiveTexture");
}

#if 0
std::string vertex_Shader =
"#version 330\n"
"layout(location = 0) in vec2 v_position;\n"
"layout(location = 1) in vec4 v_color;\n"
"out vec4 f_color;\n"
"void main()\n"
"{\n"
"    f_color = v_color;\n"
"    gl_Position = vec4(v_position, 0, 1);\n"
"}	\n";

std::string pixel_Shader =
"#version 330\n"
"out vec4 o_color;\n"
"in vec4 f_color;\n"
"void main()\n"
"{\n"
"	o_color = f_color;\n"
"};\n";

const char* vertex_Shader_Source =
"#version 330\n"
"layout(location = 0) in vec2 v_position;\n"
"layout(location = 1) in vec4 v_color;\n"
"layout(location = 2) in vec2 v_uv;\n"
"out vec4 f_color;\n"
"out vec2 f_uv;\n"
"void main()\n"
"{\n"
"    f_color = v_color;\n"
"    f_uv = v_uv;\n"
"    gl_Position = vec4(v_position, 0, 1);\n"
"}	\n";

const char* fragment_Shader_Source =
"#version 330\n"
"out vec4 o_color;\n"
"in vec4 f_color;\n"
"in vec2 f_uv;\n"
"uniform sampler2D texDiffuse;\n"
"uniform sampler2D texDiffuse_2;\n"
"uniform float u_uv_Offset_X;\n"
"uniform float u_uv_Offset_Y;\n"
"void main()\n"
"{\n"

"	// o_color = vec4(f_uv, 0, 1);\n"
"	float alpha = texture(texDiffuse, f_uv).a;\n"
"	vec2 uv = f_uv;\n"
"	uv.x += u_uv_Offset_X;\n"
"	uv.y += u_uv_Offset_Y;\n"
"	o_color = vec4(texture(texDiffuse_2, uv).rgb, alpha);\n"
"};\n";

#version 330
layout(location = 0) in vec2 v_position;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;

out vec4 f_color;
out vec2 f_uv;

void main()
{
    f_color = v_color;
    f_uv = v_uv;
    gl_Position = vec4(v_position, 0.0, 1.0);
}

#version 330
out vec4 o_color;
in vec4 f_color;
in vec2 f_uv;

void main()
{
    o_color = f_color;
}
#endif
// *********************************************************

struct V2 {
	float x;
	float y;
};

struct Color_f {
	float r;
	float g;
	float b;
	float a;
};

struct Vertex {
	V2 pos;
	Color_f color;
	V2 uv;
};

GLuint create_shader(const std::string shader_file_path, GLenum shader_type) {
	std::ifstream file(shader_file_path);
	if (!file.is_open()) {
		log("ERROR: shader file did not open");
		assert(false);
	}

	std::stringstream shader_source;
	shader_source << file.rdbuf();
	file.close();

	std::string shader_source_string = shader_source.str();
	const char* shader_source_cstr = shader_source_string.c_str();

	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_source_cstr, NULL);
	glCompileShader(shader);
	GLint success;
	char info_Log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, info_Log);
		log("ERROR: Vertex shader compilation failed: %s", info_Log);
		assert(false);
	}

	return shader;
}

GLuint create_shader_program(const char* vertex_shader_file_path, const char* fragment_shader_file_path) {
	GLuint result;
	result = glCreateProgram();
	GLuint vertex_shader = create_shader(vertex_shader_file_path, GL_VERTEX_SHADER);
	GLuint fragment_shader = create_shader(fragment_shader_file_path, GL_FRAGMENT_SHADER);

	glAttachShader(result, vertex_shader);
	glAttachShader(result, fragment_shader);
	glLinkProgram(result);
	GLint success;
	glGetProgramiv(result, GL_LINK_STATUS, &success);
	if (!success) {
		assert(false);
	}

	// No longer needed
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	return result;
}

struct Color_8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

 enum Shader_Program_Type {
	SPT_COLOR,
	SPT_TEXTURE,
	SPT_TOTAL
};
 
struct Vertices_Info {
	int total_vertices;
	size_t starting_index;
	Uint32 index_buffer_index;
	int total_indices;
	int draw_type;
	Shader_Program_Type type;
	GLuint texture_handle;
	SDL_BlendMode blend_mode;
};

struct Renderer {
	HWND window;
	HDC hdc;
	Color_8 render_draw_color;
	SDL_BlendMode blend_mode;

	GLuint vao;
	GLuint vbo;
	std::vector<Vertex> vertices;
	GLuint ebo;
	std::vector<Uint32> vertices_indices;
	std::vector<Vertices_Info> vertices_info;

	SDL_Rect clip_rect;     
	bool clip_rect_set;
};

// Set initialization list to 0
static GLuint shader_program_types[SPT_TOTAL] = { 0 };
void load_shaders() {
	const char* color_vertex_shader_file_path = "Shaders\\vertex_shader_color.txt";
	const char* color_fragment_shader_file_path = "Shaders\\fragment_shader_color.txt";
	GLuint color_shader = create_shader_program(color_vertex_shader_file_path, color_fragment_shader_file_path);
	shader_program_types[SPT_COLOR] = color_shader;

	const char* texture_vertex_shader_file_path = "Shaders\\vertex_shader_texture.txt";
	const char* texture_fragment_shader_file_path = "Shaders\\fragment_shader_texture.txt";
	GLuint texture_shader = create_shader_program(texture_vertex_shader_file_path, texture_fragment_shader_file_path);
	shader_program_types[SPT_TEXTURE] = texture_shader;
}

#define REF(v) (void)v
// Changing back to a SDL_Window when we transfer it back to other project
SDL_Renderer* SDL_CreateRenderer(HWND window, int index, uint32_t flags) {
	REF(index);
	REF(flags);

	Renderer* renderer = new Renderer();

	renderer->window = window;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbGuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	renderer->hdc = GetDC(window);
	int pixelFormatIndex = ChoosePixelFormat(renderer->hdc, &pfd);

	if (pixelFormatIndex == 0) {
		log("Error: ChoosePixelFormat function returned 0");
		return NULL;
	}

	if (!SetPixelFormat(renderer->hdc, pixelFormatIndex, &pfd)) {
		log("Error: SetPixelFormat function returned false");
		return NULL;
	}

	// We need this context (which is literally garbage) to load the functions
	// We need wglCreateContext to load the context that we ACTUALLY need
	HGLRC temp_context = wglCreateContext(renderer->hdc);
	wglMakeCurrent(renderer->hdc, temp_context);
	loadGLFunctions();

	int attrib_list[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
#if _DEBUG
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, //WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0, 0
	};

	HGLRC glRendereringContext = wglCreateContextAttribsARB(renderer->hdc, 0, attrib_list);
	// Make the new context current
	wglMakeCurrent(renderer->hdc, glRendereringContext);
	// Delete the garbage context
	wglDeleteContext(temp_context);

	if (glRendereringContext == NULL) {
		log("Error: wglCreateContext function returned false");
		return NULL;
	}
	
	// Stores the configuration of the vertex attributes and the buffers used
	// NOTE: Both buffers will use this vao
	glGenVertexArrays(1, &renderer->vao);
	glBindVertexArray(renderer->vao);

	glGenBuffers(1, &renderer->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

	glGenBuffers(1, &renderer->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	load_shaders();

	renderer->clip_rect_set = false;

	return (SDL_Renderer*)renderer;
}

void get_window_size(HWND window, int& w, int& h) {
	RECT rect = {};
	if (GetClientRect(window, &rect) != 0) {
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
	} else {
		w = 0;
		h = 0;
		log("Window width and height are 0");
	}
}

int SDL_GetRendererOutputSize(SDL_Renderer* sdl_renderer, int* w, int* h) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	get_window_size(renderer->window, *w, *h);

	return 0;
}

int SDL_SetRenderDrawColor(SDL_Renderer* sdl_renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
	renderer->render_draw_color = { r, g, b, a };

	return 0;
}

int SDL_GetRenderDrawColor(SDL_Renderer* sdl_renderer, Uint8* r, Uint8* g, Uint8* b, Uint8* a) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	*r = renderer->render_draw_color.r;
	*g = renderer->render_draw_color.g;
	*b = renderer->render_draw_color.b;
	*a = renderer->render_draw_color.a;

	return 0;
}

Color_f convert_color_8_to_floating_point(Color_8 color) {
	Color_f result;

	result.r = (float)color.r / 255.0f;
	result.g = (float)color.g / 255.0f;
	result.b = (float)color.b / 255.0f;
	result.a = (float)color.a / 255.0f;

	return result;
}

// This function clears the entire rendering target, ignoring the viewport and the clip rectangle.
int SDL_RenderClear(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	Color_f c = convert_color_8_to_floating_point(renderer->render_draw_color);

	SDL_RenderSetClipRect(sdl_renderer, nullptr);

	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);

	if (renderer->clip_rect_set) {
		SDL_RenderSetClipRect(sdl_renderer, &renderer->clip_rect);
	}

	return 0;
}

V2 convert_to_ndc(SDL_Renderer* sdl_renderer, V2 pos) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	int screen_w, screen_h;
	get_window_size(renderer->window, screen_w, screen_h);

	V2 ndc;
	ndc.x = ((2.0f * pos.x) / screen_w) - 1.0f;
	ndc.y = (1.0f - ((2.0f * pos.y) / screen_h));
	return ndc;
}

V2 convert_to_ndc(SDL_Renderer* sdl_renderer, int x, int y) {
	return convert_to_ndc(sdl_renderer, { (float)x, (float)y });
}

V2 convert_to_uv_coordinates(V2 pos, int w, int h) {
	V2 uv;
	uv.x = pos.x / w;
	uv.y = pos.y / h;
	return uv;
}

int SDL_RenderFillRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) { 
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        return -1;
    }
    if (rect == nullptr) {
        log("ERROR: rect is nullptr");
        return -1;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

    Color_f c = convert_color_8_to_floating_point(renderer->render_draw_color);
    SDL_Rect rect_result = *rect;

    // Calculate the vertices based on the top-left corner
    V2 top_left = { (float)rect_result.x, (float)rect_result.y };
    V2 top_right = { (float)(rect_result.x + rect_result.w), (float)rect_result.y };
    V2 bottom_right = { (float)(rect_result.x + rect_result.w), (float)(rect_result.y + rect_result.h) };
    V2 bottom_left = { (float)rect_result.x, (float)(rect_result.y + rect_result.h) };

    V2 top_left_ndc = convert_to_ndc(sdl_renderer, top_left);
    V2 top_right_ndc = convert_to_ndc(sdl_renderer, top_right);
    V2 bottom_right_ndc = convert_to_ndc(sdl_renderer, bottom_right);
    V2 bottom_left_ndc = convert_to_ndc(sdl_renderer, bottom_left);
	
	Vertex vertices[6] = {};
	// NOTE: Ignore the UV value. No texture.
	// ***First Triangle***
	// Bottom Left
	vertices[0].pos = bottom_left_ndc;
	vertices[0].color = c;
	renderer->vertices.push_back(vertices[0]);
	// Top Left
	vertices[1].pos = top_left_ndc;
	vertices[1].color = c;
	renderer->vertices.push_back(vertices[1]);
	// Top Right
	vertices[2].pos = top_right_ndc;
	vertices[2].color = c;
	renderer->vertices.push_back(vertices[2]);

	// ***Second Triangle***
	// Bottom Left
	vertices[3].pos = bottom_left_ndc;
	vertices[3].color = c;
	renderer->vertices.push_back(vertices[3]);
	// Bottom Right
	vertices[4].pos = bottom_right_ndc;
	vertices[4].color = c;
	renderer->vertices.push_back(vertices[4]);
	// Top Right
	vertices[5].pos = top_right_ndc;
	vertices[5].color = c;
	renderer->vertices.push_back(vertices[5]);

	// Store the number of vertices to be rendered for this group
	Vertices_Info info;
	info.draw_type = GL_TRIANGLES;
	info.total_vertices = ARRAYSIZE(vertices);
	info.starting_index = renderer->vertices.size() - info.total_vertices;
	info.type = SPT_COLOR;
	info.blend_mode = renderer->blend_mode;
	// Store the number of vertices to be rendered for this group
	renderer->vertices_info.push_back(info);

	// Returns 0 on success
	return 0;
}

int SDL_RenderFillRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}

	for (int i = 0; i < count; i++) {
		if (SDL_RenderFillRect(sdl_renderer, &rects[i]) != 0) {
			return -1;
		}
	}

	return 0;
}

int SDL_RenderDrawLine(SDL_Renderer* sdl_renderer, int x1, int y1, int x2, int y2) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	Color_f c =	convert_color_8_to_floating_point(renderer->render_draw_color);
	
	V2 point_one =		convert_to_ndc(sdl_renderer, x1, y1);
	V2 point_two =		convert_to_ndc(sdl_renderer, x2, y2);
	
	Vertex vertices[2] = {};
	vertices[0].pos = point_one;
	vertices[0].color = c;
	renderer->vertices.push_back(vertices[0]);
	vertices[1].pos = point_two;
	vertices[1].color = c;
	renderer->vertices.push_back(vertices[1]);

	Vertices_Info info;
	info.draw_type = GL_LINES;
	info.total_vertices = ARRAYSIZE(vertices);
	// Subtract the already added vertices to get the starting index
	info.starting_index = renderer->vertices.size() - info.total_vertices;
	info.type = SPT_COLOR;
	info.blend_mode = renderer->blend_mode;

	// Store the number of vertices to be rendered for this group
	renderer->vertices_info.push_back(info);
	
	return 0;
}

int SDL_RenderDrawLines(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count) { 
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}

	if (count <= 0) {
		log("SDL_RenderDrawLines count is <= 0");
	}

	for (int i = 0; i < count - 1; i++) {
		SDL_Point p1 = points[i];
		SDL_Point p2 = points[i + 1];
		SDL_RenderDrawLine(sdl_renderer, p1.x, p1.y, p2.x, p2.y);
	}

	return 0;
}

int SDL_RenderDrawPoint(SDL_Renderer* sdl_renderer, int x, int y) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}

	SDL_Rect rect = { x, y, 5, 5 };
	SDL_RenderFillRect(sdl_renderer, &rect);

	return 0;
}

int SDL_RenderDrawPoints(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}

	if (count <= 0) {
		log("SDL_RenderDrawLines count is <= 0");
	}

	for (int i = 0; i < count; i++) {
		SDL_Point p1 = points[i];
		SDL_RenderDrawPoint(sdl_renderer, p1.x, p1.y);
	}

	return 0;
}

int SDL_RenderDrawRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        return -1;
    }
    if (rect == nullptr) {
        log("ERROR: rect is nullptr");
        return -1;
    }
    // Renderer* renderer = (Renderer*)sdl_renderer;

    // Use the provided rectangle directly
    SDL_Rect rect_result = *rect;

    // Calculate the vertices based on the top-left corner
    V2 top_left = { (float)rect_result.x, (float)rect_result.y };
    V2 top_right = { (float)(rect_result.x + rect_result.w), (float)rect_result.y };
    V2 bottom_right = { (float)(rect_result.x + rect_result.w), (float)(rect_result.y + rect_result.h) };
    V2 bottom_left = { (float)rect_result.x, (float)(rect_result.y + rect_result.h) };

    // Draw the rectangle's edges
    SDL_RenderDrawLine(sdl_renderer, (int)top_left.x, (int)top_left.y, (int)top_right.x, (int)top_right.y);
    SDL_RenderDrawLine(sdl_renderer, (int)top_right.x, (int)top_right.y, (int)bottom_right.x, (int)bottom_right.y);
    SDL_RenderDrawLine(sdl_renderer, (int)bottom_right.x, (int)bottom_right.y, (int)bottom_left.x, (int)bottom_left.y);
    SDL_RenderDrawLine(sdl_renderer, (int)bottom_left.x, (int)bottom_left.y, (int)top_left.x, (int)top_left.y);

    // Return 0 on success
    return 0;
}

int SDL_RenderDrawRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}

	if (count <= 0) {
		log("SDL_RenderDrawLines count is <= 0");
	}

	for (int i = 0; i < count; i++) {
		SDL_Rect r = rects[i];
		SDL_RenderDrawRect(sdl_renderer, &r);
	}

	return 0;
}

struct GL_Texture {
	int w;
	int h;
	GLuint handle;
};

GL_Texture load_gl_texture_data(const char* file_name) {
	GL_Texture result;

	stbi_set_flip_vertically_on_load(true);

	int x, y, n;
	unsigned char* data = stbi_load(file_name, &x, &y, &n, 4);

	if (data == nullptr) {
		log("ERROR: stbi_load returned nullptr");
		assert(false);
	}
	DEFER{
		stbi_image_free(data);
	};
	result.w = x;
	result.h = y;

	// Done in create texture
	glGenTextures(1, &result.handle);
	// Done in lock texture
	glBindTexture(GL_TEXTURE_2D, result.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.w, result.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return result;
}

typedef enum SDL_TextureAccess
{
    SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
    SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
    SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
} SDL_TextureAccess;

struct SDL_Texture {
	GLuint handle;
	uint32_t format;
	int access;
	int w, h;

	SDL_BlendMode blend_mode;

	Uint8 alpha_mod;

	int pitch;
	// Locked if null
	void* pixels;
	SDL_Rect portion;
}; 

SDL_Texture* SDL_CreateTexture(SDL_Renderer* sdl_renderer, uint32_t format, int access, int w, int h) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
		return NULL;
	}

	SDL_Texture* result = new SDL_Texture();
	// One texture with one name
	glGenTextures(1, &result->handle);
	result->format = format;
	result->access = access;
	result->w = w;
	result->h = h;

	// Default value for textures
	SDL_SetTextureBlendMode(result, SDL_BLENDMODE_NONE);

	// If the pixels variable is null
	result->pitch = 0;
	result->pixels = NULL;

	// Default alpha mod
	SDL_SetTextureAlphaMod(result, 255);

	glBindTexture(GL_TEXTURE_2D, result->handle);
	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the texture
	// Just allocate the memory and give it data later
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result->w, result->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	return result;
}

void my_Memory_Copy(void* dest, const void* src, size_t count) {
	unsigned char* destination = (unsigned char*)dest;
	unsigned char* source = (unsigned char*)src;
	for (int i = 0; i < count; i++) {
		destination[i] = source[i];
	}
}

int is_valid_sdl_blend_mode(int mode) {
	switch (mode) {
	case SDL_BLENDMODE_NONE:
		return 0;
	case SDL_BLENDMODE_BLEND:
		return 0;
	case SDL_BLENDMODE_ADD:
		return 0;
	case SDL_BLENDMODE_MOD:
		return 0;
	case SDL_BLENDMODE_MUL:
		return 0;
	default:
		return -1;
	}
}

void SDL_DestroyTexture(SDL_Texture* texture) {
	if (texture != NULL) {
		delete texture->pixels;
		glDeleteTextures(1, &texture->handle);
		delete texture;
	}
}

int SDL_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blendMode) {
	if (is_valid_sdl_blend_mode(blendMode)) {
		// Default blend mode
		texture->blend_mode= SDL_BLENDMODE_BLEND;
		log("ERROR: Invalid blend_mode");
		assert(false);
		return -1;
	} 

	if (texture->blend_mode != blendMode) {
		texture->blend_mode = blendMode;
	}

	return 0;
}

int SDL_GetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode* blendMode) {
	// These should never be the case, but we'll go ahead and guard against it
	if (texture == nullptr) {
		log("ERROR: Invalid texture pointer");
		return -1;
	}
	if (blendMode == nullptr) {
		log("ERROR: Invalid blendMode pointer");
		return -1;
	}

	*blendMode = texture->blend_mode;

	return 0;
}

int SDL_SetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode blendMode) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	if (is_valid_sdl_blend_mode(blendMode)) {
		// Default blend mode
		renderer->blend_mode = SDL_BLENDMODE_BLEND;
		log("ERROR: Invalid blend_mode");
		assert(false);
		return -1;
	} 

    // Only set the blend mode if it is different from the current one
    if (renderer->blend_mode != blendMode) {
        renderer->blend_mode = blendMode;
    }

	return 0;
}

int SDL_GetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode* blendMode) {
	// These should never be the case, but we'll go ahead and guard against it
	if (sdl_renderer == nullptr) {
		log("ERROR: Invalid sdl_renderer pointer");
		(false);
		return -1;
	}
	if (blendMode == nullptr) {
		log("ERROR: Invalid blendMode pointer");
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
	*blendMode = renderer->blend_mode;

	return 0;
}


// My own function
int set_gl_blend_mode(SDL_BlendMode blend_mode) {
	switch (blend_mode) {
		case SDL_BLENDMODE_NONE:
			glDisable(GL_BLEND);
			break;
		case SDL_BLENDMODE_BLEND:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// glBlendEquation(GL_FUNC_ADD);
			break;
		case SDL_BLENDMODE_ADD:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			// glBlendEquation(GL_FUNC_ADD);
			break;
		case SDL_BLENDMODE_MOD:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			// glBlendEquation(GL_FUNC_ADD);
			break;
		case SDL_BLENDMODE_MUL:
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
			// glBlendEquation(GL_FUNC_ADD);
			break;
		default:
			log("ERROR: Invalid blend mode");
			assert(false);
			return -1;
	}

	return 0;
}

// NOTE: For changing the texture on the CPU's side
int SDL_LockTexture(SDL_Texture* texture, const SDL_Rect* rect, void **pixels, int *pitch) {
	if (texture == NULL) {
		log("ERROR: Invalid texture");
		return -1;
	}

	if (texture->pixels != NULL) {
		log("ERROR: Texture is already locked");
		return -1;
	}

	SDL_Rect full_rect = { 0, 0, texture->w, texture->h };
	if (rect == NULL) {
		texture->portion = full_rect;
	}
	else {
		texture->portion = *rect;
	}

	if (texture->portion.x < 0 || 
		texture->portion.y < 0 || 
		texture->portion.x + texture->portion.w > texture->w || 
		texture->portion.y + texture->portion.h > texture->h) {
		log("ERROR: Lock area out of bounds");
		return -1;
	}

	int buffer_size = texture->portion.w * texture->portion.h * SDL_BYTESPERPIXEL(texture->format);
	Uint8* buffer = new Uint8[buffer_size];
	if (buffer == NULL) {
		log("ERROR: Memory allocation failed");
		return -1;
	}

	*pixels = buffer;
	*pitch = texture->portion.w * SDL_BYTESPERPIXEL(texture->format);
	texture->pixels = *pixels;

	return 0;
}

// Uploading the texture to the GPU
// Just changing the pixels
void SDL_UnlockTexture(SDL_Texture* texture) {
	glBindTexture(GL_TEXTURE_2D, texture->handle);

	if (texture->pixels == NULL) {
		log("ERROR: Pixels == NULL");
		assert(false);
		return;
	} 

	glTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		texture->portion.x,
		texture->portion.y,
		texture->portion.w,
		texture->portion.h,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		texture->pixels
	);

	delete texture->pixels;
	texture->pixels = nullptr;
}

int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void *pixels, int pitch) {
	if (texture == NULL || pixels == NULL) {
		log("ERROR: Invalid texture or pixels");
		return -1;
	}

	void* pixels_temp = NULL;
	int locked_pitch = 0;

	// Lock the texture to get access to the pixel buffer
	if (SDL_LockTexture(texture, rect, &pixels_temp, &locked_pitch) != 0) {
		log("ERROR: Failed to lock texture");
		return -1;
	}

	// Calculate the width and height to be updated
	int width = rect ? rect->w : texture->w;
	int height = rect ? rect->h : texture->h;

	// Update the texture's pixel data
	Uint8* dest = (Uint8*)pixels_temp;
	const Uint8* src = (const Uint8*)pixels;

	for (int y = 0; y < height; y++) {
		my_Memory_Copy(dest + y * locked_pitch, src + y * pitch, width * SDL_BYTESPERPIXEL(texture->format));
	}

	SDL_UnlockTexture(texture);

	return 0;
}

int SDL_SetTextureAlphaMod(SDL_Texture* texture, Uint8 alpha) {
	if (texture == NULL) {
		log("ERROR: Invalid texture or pixels");
		return -1;
	}

	if (alpha < 0 || alpha > 255) {
		log("ERROR: Invalid alpha mod");
		return -1;
	}

	texture->alpha_mod = alpha;

	return 0;
}

int SDL_GetTextureAlphaMod(SDL_Texture* texture, Uint8* alpha) {
	if (texture == NULL) {
		log("ERROR: Invalid texture or pixels");
		return -1;
	}

	*alpha = texture->alpha_mod; 
	
	return 0;
}

// Calculate the intersection of two rectangles.
bool SDL_IntersectRect(const SDL_Rect* A, const SDL_Rect* B, SDL_Rect* result) {
    if (!A || !B || !result) {
        return false;
    }

    int A_x_min = A->x;
    int A_x_max = A->x + A->w;
    int A_y_min = A->y;
    int A_y_max = A->y + A->h;

    int B_x_min = B->x;
    int B_x_max = B->x + B->w;
    int B_y_min = B->y;
    int B_y_max = B->y + B->h;

    // Calculate the intersecting rectangle
    int x_min = A_x_min > B_x_min ? A_x_min : B_x_min;
    int x_max = A_x_max < B_x_max ? A_x_max : B_x_max;
    int y_min = A_y_min > B_y_min ? A_y_min : B_y_min;
    int y_max = A_y_max < B_y_max ? A_y_max : B_y_max;

    // Check if there is an intersection
    if (x_min < x_max && y_min < y_max) {
        result->x = x_min;
        result->y = y_min;
        result->w = x_max - x_min;
        result->h = y_max - y_min;
        return true;
    } else {
        // No intersection
        result->x = 0;
        result->y = 0;
        result->w = 0;
        result->h = 0;
        return false;
    }
}

int SDL_RenderCopy(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	if (texture == NULL) {
		log("ERROR: Invalid texture");
		return -1;
	}

	// Null for entire texture
	SDL_Rect srcrect_final;
	if (srcrect == NULL) {
		srcrect_final = { 0, 0, texture->w, texture->h };
	}
	else {
		srcrect_final = *srcrect;
	}
	
	// Null for entire rendering target
    SDL_Rect dstrect_final;
    if (dstrect == NULL) {
        int screen_w = 0;
        int screen_h = 0;
        get_window_size(renderer->window, screen_w, screen_h);
        dstrect_final.x = 0;
        dstrect_final.y = 0;
        dstrect_final.w = screen_w;
        dstrect_final.h = screen_h;
    } else {
        dstrect_final = *dstrect;
    }

	// Calculate the UV coordinates based on the source rectangle
    V2 bottom_left_src = { (float)srcrect_final.x, (float)srcrect_final.y + srcrect_final.h };
    V2 bottom_right_src = { (float)(srcrect_final.x + srcrect_final.w), (float)(srcrect_final.y + srcrect_final.h) };
    V2 top_right_src = { (float)(srcrect_final.x + srcrect_final.w), (float)srcrect_final.y };
    V2 top_left_src = { (float)srcrect_final.x, (float)srcrect_final.y };

    V2 bottom_left_uv = convert_to_uv_coordinates(bottom_left_src, texture->w, texture->h);
    V2 bottom_right_uv = convert_to_uv_coordinates(bottom_right_src, texture->w, texture->h);
    V2 top_right_uv = convert_to_uv_coordinates(top_right_src, texture->w, texture->h);
    V2 top_left_uv = convert_to_uv_coordinates(top_left_src, texture->w, texture->h);

	// Calculate the vertices positions
	Color_f c = convert_color_8_to_floating_point(renderer->render_draw_color);

	// Calculate the destination vertices based on the top-left corner
    V2 top_left_dst = { (float)dstrect_final.x, (float)dstrect_final.y };
    V2 top_right_dst = { (float)(dstrect_final.x + dstrect_final.w), (float)dstrect_final.y };
    V2 bottom_right_dst = { (float)(dstrect_final.x + dstrect_final.w), (float)(dstrect_final.y + dstrect_final.h) };
    V2 bottom_left_dst = { (float)dstrect_final.x, (float)(dstrect_final.y + dstrect_final.h) };

    V2 top_left_ndc = convert_to_ndc(sdl_renderer, top_left_dst);
    V2 top_right_ndc = convert_to_ndc(sdl_renderer, top_right_dst);
    V2 bottom_right_ndc = convert_to_ndc(sdl_renderer, bottom_right_dst);
    V2 bottom_left_ndc = convert_to_ndc(sdl_renderer, bottom_left_dst);

#if 0
	Vertex vertices[6] = {};
	// NOTE: Ignore the UV value. No texture.
	// ***First Triangle***
	// Bottom Left
	vertices[0].pos = bottom_left_ndc;
	vertices[0].color = c;
	// Modify the alpha mod
	vertices[0].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[0].uv = bottom_left_uv;
	renderer->vertices_vbo.push_back(vertices[0]);
	// Top Left
	vertices[1].pos = top_left_ndc;
	vertices[1].color = c;
	vertices[1].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[1].uv = top_left_uv;
	renderer->vertices_vbo.push_back(vertices[1]);
	// Top Right
	vertices[2].pos = top_right_ndc;
	vertices[2].color = c;
	vertices[2].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[2].uv = top_right_uv;
	renderer->vertices_vbo.push_back(vertices[2]);

	// ***Second Triangle***
	// Bottom Left
	vertices[3].pos = bottom_left_ndc;
	vertices[3].color = c;
	vertices[3].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[3].uv = bottom_left_uv;
	renderer->vertices_vbo.push_back(vertices[3]);
	// Bottom Right
	vertices[4].pos = bottom_right_ndc;
	vertices[4].color = c;
	vertices[4].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[4].uv = bottom_right_uv;
	renderer->vertices_vbo.push_back(vertices[4]);
	// Top Right
	vertices[5].pos = top_right_ndc;
	vertices[5].color = c;
	vertices[5].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[5].uv = top_right_uv;
	renderer->vertices_vbo.push_back(vertices[5]);

#endif

	// Vertices for the quad
	Vertex vertices[4];

	// Bottom left
	vertices[0].pos = bottom_left_ndc;
	vertices[0].color = c;
	vertices[0].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[0].uv = bottom_left_uv;

	// Top left
	vertices[1].pos = top_left_ndc;
	vertices[1].color = c;
	vertices[1].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[1].uv = top_left_uv;

	// Top right
	vertices[2].pos = top_right_ndc;
	vertices[2].color = c;
	vertices[2].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[2].uv = top_right_uv;

	// Bottom right
	vertices[3].pos = bottom_right_ndc;
	vertices[3].color = c;
	vertices[3].color.a *= ((float)texture->alpha_mod / 255.0f);
	vertices[3].uv = bottom_right_uv;

	// Vertices for the vbo
	renderer->vertices.push_back(vertices[0]);
	renderer->vertices.push_back(vertices[1]);
	renderer->vertices.push_back(vertices[2]);
	renderer->vertices.push_back(vertices[3]);

	// Define indices for the two triangles that make up the quad
	Uint32 baseIndex = static_cast<Uint32>(renderer->vertices.size()) - 4;
	// Bottom left
	renderer->vertices_indices.push_back(baseIndex + 0);
	// Top left
	renderer->vertices_indices.push_back(baseIndex + 1); 
	// Top right
	renderer->vertices_indices.push_back(baseIndex + 2); 

	// Bottom left
	renderer->vertices_indices.push_back(baseIndex + 0); 
	// Top right
	renderer->vertices_indices.push_back(baseIndex + 2); 
	// Bottom right
	renderer->vertices_indices.push_back(baseIndex + 3); 

	Vertices_Info info = {};
	info.draw_type = GL_TRIANGLES;
	info.total_vertices = ARRAYSIZE(vertices);
	info.starting_index = renderer->vertices.size() - info.total_vertices;
	info.type = SPT_TEXTURE;
	info.texture_handle = texture->handle;
	info.total_indices = 6;
	info.index_buffer_index = (Uint32)renderer->vertices_indices.size() - info.total_indices;
	info.blend_mode = texture->blend_mode;
	renderer->vertices_info.push_back(info);

	return 0;
}

Image create_Image(SDL_Renderer* sdl_renderer, const char* file_Path) {
	Image result = {};
	result.file_Path = file_Path;

	int width, height, channels;
	unsigned char* data = stbi_load(file_Path, &width, &height, &channels, 4);

	if (data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return result;
	}

	result.pixel_Data = data;

	result.width = width;
	result.height = height;

	DEFER{
		// Freed at the end of main
		// stbi_image_free(data);
	};

	SDL_Texture* temp = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, result.width, result.height);

	if (temp == NULL) {
		log("ERROR: SDL_CreateTexture returned NULL");
		return result;
	}

	if (SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND) != 0) {
		log("ERROR: SDL_SsetTextureBlendMode did not succeed: %s");
	}

	result.texture = temp;

	void* pixels;
	int pitch;
	SDL_LockTexture(temp, NULL, &pixels, &pitch);

	my_Memory_Copy(pixels, data, (result.width * result.height) * 4);

	SDL_UnlockTexture(temp);

	return result;
}

void SDL_DestroyRenderer(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	// Delete shader programs
	for (int i = 0; i < SPT_TOTAL; i++) {
		GLuint program = shader_program_types[0];
		glDeleteProgram(program);
	}
	
	delete renderer;
}

int SDL_RenderSetClipRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return -1;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

    int window_width = 0;
    int window_height = 0;
    get_window_size(renderer->window, window_width, window_height);

    if (rect == nullptr) {
        // Disable clipping
        glDisable(GL_SCISSOR_TEST);
		renderer->clip_rect_set = false;
    } else {
        // Enable and set the scissor rectangle
        renderer->clip_rect = *rect;
		renderer->clip_rect_set = true;
		// Convert because SDL coordinates are inverted
		int scissor_x = renderer->clip_rect.x;
		int scissor_y = window_height - renderer->clip_rect.y - renderer->clip_rect.h;
		int scissor_width = renderer->clip_rect.w;
		int scissor_height = renderer->clip_rect.h;
		glEnable(GL_SCISSOR_TEST);
		glScissor(scissor_x, scissor_y, scissor_width, scissor_height);
    }
    
	return 0;
}

int SDL_RenderSetViewport(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return -1;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

    int window_width = 0;
    int window_height = 0;
    get_window_size(renderer->window, window_width, window_height);

    if (rect == NULL) {
        // If rect is NULL, use the entire window as the viewport
        glViewport(0, 0, window_width, window_height);
    } else {
        // Convert SDL rect coordinates to OpenGL coordinates
        int viewport_x = rect->x;
        int viewport_y = window_height - rect->y - rect->h;
        int viewport_width = rect->w;
        int viewport_height = rect->h;

        glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    }

    return 0;
}

void SDL_RenderPresent(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	// This rebind may be pointless if I am only going with one vbo
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferData(GL_ARRAY_BUFFER, renderer->vertices.size() * sizeof(Vertex), renderer->vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->vertices_indices.size() * sizeof(Uint32), renderer->vertices_indices.data(), GL_STATIC_DRAW);

	for (Vertices_Info& info : renderer->vertices_info) {
		// Set the blend mode before I render all the vertices
		if (set_gl_blend_mode(info.blend_mode)) {
			log("ERROR: blend mode not set");
			assert(false);
		}
		if (info.texture_handle) {
			glBindTexture(GL_TEXTURE_2D, info.texture_handle);
		}
		GLuint shader_program = shader_program_types[info.type];
		if (!shader_program) {
			log("ERROR: Shader program not specified");
			assert(false);
		}
		glUseProgram(shader_program);

		if (info.total_vertices == 4) {
			glDrawElements(info.draw_type, info.total_indices, GL_UNSIGNED_INT, (void*)(info.index_buffer_index * sizeof(unsigned int)));
		}
		else {
			glDrawArrays(info.draw_type, (GLuint)info.starting_index, info.total_vertices);
		}
	}
	renderer->vertices_info.clear();

	SwapBuffers(renderer->hdc);
}

void draw_debug_images(SDL_Renderer* sdl_renderer) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return;
    }
    // Renderer* renderer = (Renderer*)sdl_renderer;
	int x = 100;
	int x_off = 150;
	int y = 100;
	int y_off = 150;

	// ROW 1
	SDL_SetRenderDrawColor(sdl_renderer, 155, 0, 0, 255);
	SDL_Rect rect_one_a = { x, y, 100, 100 };
	x += x_off;
	SDL_RenderDrawRect(sdl_renderer, &rect_one_a);
	
	SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
	SDL_Rect rect_one_b = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rect_two_b = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rect_three_b = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rects_group_b[3] = { rect_one_b, rect_two_b, rect_three_b };
	SDL_RenderDrawRects(sdl_renderer, rects_group_b, ARRAYSIZE(rects_group_b));

	SDL_SetRenderDrawColor(sdl_renderer, 0, 155, 0, 255);
	SDL_Rect rect_one_c = { x, y, 100, 100 };
	x += x_off;
	SDL_RenderFillRect(sdl_renderer, &rect_one_c);

	SDL_SetRenderDrawColor(sdl_renderer, 0, 255, 0, 255);
	SDL_Rect rect_one_d = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rect_two_d = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rect_three_d = { x, y, 100, 100 };
	x += x_off;
	SDL_Rect rects_group_d[3] = { rect_one_d, rect_two_d, rect_three_d };
	SDL_RenderFillRects(sdl_renderer, rects_group_d, ARRAYSIZE(rects_group_d));

	// ROW 2
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 155, 255);
	y += y_off;
	x = 100;
	SDL_Point p1_a = { x, y };
	SDL_Point p2_a = { x, y + 100 };
	SDL_RenderDrawLine(sdl_renderer, p1_a.x, p1_a.y, p2_a.x, p2_a.y);
	x += x_off;

	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 255, 255);
	SDL_Point p1_b = { x, y };
	SDL_Point p2_b = { x + 100, y };
	SDL_Point p3_b = { x + 100, y + 100 };
	SDL_Point p4_b = { x, y + 100 };
	SDL_Point p5_b = { x, y };
	SDL_Point points_group_b[5] = { p1_b, p2_b, p3_b, p4_b, p5_b };
	SDL_RenderDrawLines(sdl_renderer, points_group_b, ARRAYSIZE(points_group_b));
	x += x_off;

	SDL_SetRenderDrawColor(sdl_renderer, 0, 155, 155, 255);
	SDL_Point p1_c = { x, y };
	SDL_RenderDrawPoint(sdl_renderer, p1_c.x, p1_c.y);
	x += x_off;

	SDL_Point p1_d = { x, y };
	SDL_Point p2_d = { x + 100, y };
	SDL_Point p3_d = { x + 100, y + 100};
	SDL_Point p4_d = { x, y + 100};
	SDL_Point points_group_d[4] = { p1_d, p2_d, p3_d, p4_d };
	SDL_RenderDrawPoints(sdl_renderer, points_group_d, ARRAYSIZE(points_group_d));
}
