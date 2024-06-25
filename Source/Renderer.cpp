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

struct Texture {
	int w;
	int h;
	GLuint handle;
};


Texture load_Texture_Data(const char* file_Name) {
	Texture result;

	stbi_set_flip_vertically_on_load(true);

	int x, y, n;
	unsigned char* data = stbi_load(file_Name, &x, &y, &n, 4);

	if (data == nullptr) {
		log("ERROR: stbi_load returned nullptr");
		assert(false);
	}
	DEFER{
		stbi_image_free(data);
	};
	result.w = x;
	result.h = y;

	glGenTextures(1, &result.handle);
	glBindTexture(GL_TEXTURE_2D, result.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, result.w, result.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return result;
}

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

void setup_vertex_vao_vbo(GLuint& vao, GLuint& vbo, Vertex* vertices, size_t vertex_count) {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
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
"{\n"	GLuint vao;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);

"	// o_color = vec4(f_uv, 0, 1);\n"
"	float alpha = texture(texDiffuse, f_uv).a;\n"
"	vec2 uv = f_uv;\n"
"	uv.x += u_uv_Offset_X;\n"
"	uv.y += u_uv_Offset_Y;\n"
"	o_color = vec4(texture(texDiffuse_2, uv).rgb, alpha);\n"
"};\n";
#endif

struct Color_8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};


enum Command_Type {
	CT_FILL_RECT,
	CT_TOTAL
};

struct Render_Command {
	Command_Type type;
	SDL_Rect rect;
};

struct Renderer {
	HWND window;
	HDC hdc;
	Color_8 render_draw_color;
	std::vector<Render_Command> command_queue;

	GLuint vbo;
	GLuint vao;
	std::vector<Vertex> verticies;
};

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
		24,                   // Number of bits for the depthbuffer
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
	
	glGenVertexArrays(1, &renderer->vao);
	glBindVertexArray(renderer->vao);

	glGenBuffers(1, &renderer->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	return (SDL_Renderer*)renderer;
}

int SDL_SetRenderDrawColor(SDL_Renderer* sdl_renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
	renderer->render_draw_color = { r, g, b, a };

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

int SDL_RenderClear(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
	Color_f c = convert_color_8_to_floating_point(renderer->render_draw_color);

	glClearColor(c.r, c.g, c.b, c.a);
	glClear(GL_COLOR_BUFFER_BIT);

	return 0;
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

int SDL_RenderFillRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) { 
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
 
	Render_Command command;
	command.type = CT_FILL_RECT;
	// Null for the entire rendering context 
	if (rect == NULL) {
		int screen_w = 0;
		int screen_h = 0;
		get_window_size(renderer->window, screen_h, screen_h);
		SDL_Rect temp;
		temp.x = screen_w / 2;
		temp.y = screen_h / 2;
		temp.w = screen_w;
		temp.h = screen_h;
		command.rect = temp;
	}
	else {
		command.rect = *rect;
	}
	renderer->command_queue.push_back(command);

	// Returns 0 on success
	return 0;
}

void execute_fill_rect_command(SDL_Renderer* sdl_renderer, Render_Command command) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
  
	Color_f c = convert_color_8_to_floating_point(renderer->render_draw_color);

	int half_w = command.rect.w / 2;
	int half_h = command.rect.h / 2;
	
	V2 top_left =			{ (float)(command.rect.x - half_w) , (float)(command.rect.y + half_h) };
	V2 top_right =			{ (float)(command.rect.x + half_w) , (float)(command.rect.y + half_h) };
	V2 bottom_right =		{ (float)(command.rect.x + half_w) , (float)(command.rect.y - half_h) };
	V2 bottom_left =		{ (float)(command.rect.x - half_w) , (float)(command.rect.y - half_h) };

	V2 top_left_ndc =		convert_to_ndc(sdl_renderer, top_left);
	V2 top_right_ndc =		convert_to_ndc(sdl_renderer, top_right);
	V2 bottom_right_ndc =	convert_to_ndc(sdl_renderer, bottom_right);
	V2 bottom_left_ndc =	convert_to_ndc(sdl_renderer, bottom_left);
	
	Vertex vertices[6] = {};
	// NOTE: Ignore the UV value. No texture.
	// ***First Triangle***
	// Bottom Left
	vertices[0].pos = bottom_left_ndc;
	vertices[0].color = c;
	renderer->verticies.push_back(vertices[0]);
	// Top Left
	vertices[1].pos = top_left_ndc;
	vertices[1].color = c;
	renderer->verticies.push_back(vertices[1]);
	// Top Right
	vertices[2].pos = top_right_ndc;
	vertices[2].color = c;
	renderer->verticies.push_back(vertices[2]);

	// ***Second Triangle***
	// Bottom Left
	vertices[3].pos = bottom_left_ndc;
	vertices[3].color = c;
	renderer->verticies.push_back(vertices[3]);
	// Bottom Right
	vertices[4].pos = bottom_right_ndc;
	vertices[4].color = c;
	renderer->verticies.push_back(vertices[4]);
	// Top Right
	vertices[5].pos = top_right_ndc;
	vertices[5].color = c;
	renderer->verticies.push_back(vertices[5]);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
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

// Put anything I want into the renderer struct. Don't change api
// SDL draw functions don't necessarily emit a draw call immediately
// The draw will happen EVENTUALLY
#if 0

SDL_RenderDrawLine
SDL_RenderDrawLines
SDL_RenderDrawPoint
SDL_RenderDrawPoints
SDL_RenderDrawRect
SDL_RenderDrawRects

SDL_CreateTexture
SDL_RenderCopy
#endif

void SDL_RenderPresent(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	for (Render_Command command : renderer->command_queue) {
		switch (command.type) {
		case CT_FILL_RECT: {
			execute_fill_rect_command(sdl_renderer, command);
			break;
		}
		}
	}

	// Send all the info
	glBufferData(GL_ARRAY_BUFFER, renderer->verticies.size() * sizeof(Vertex), renderer->verticies.data(), GL_STATIC_DRAW);

	const char* vertex_shader_file_path = "Shaders\\vertex_shader.txt";
	const char* fragment_shader_file_path = "Shaders\\fragment_shader.txt";
	GLuint shader_program = create_shader_program(vertex_shader_file_path, fragment_shader_file_path);
	glUseProgram(shader_program);

	for (Vertex vertex : renderer->verticies) {
		glDrawArrays(GL_TRIANGLES, 0, renderer->verticies.size() * 6);
	}
	
	SwapBuffers(renderer->hdc);
}
