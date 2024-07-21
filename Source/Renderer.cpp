#include "Renderer.h"

#include "Utility.h"

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>

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
		log("ERROR: shader compilation failed: %s", info_Log);
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
// I Could submit three packets
int SDL_RenderClear(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		return -1;
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	Command_Packet packet = {};

	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Clear_Screen;

	packet.clear_screen_info.clear_draw_color = renderer->render_draw_color;

	renderer->command_packets.push_back(packet);

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

V3 convert_to_ndc_v3(SDL_Renderer* sdl_renderer, V3 pos) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

    int screen_w, screen_h;
    get_window_size(renderer->window, screen_w, screen_h);

	V3 ndc = {};
    ndc.x = ((2.0f * pos.x) / screen_w) - 1.0f;
    ndc.y = 1.0f - ((2.0f * pos.y) / screen_h);
	ndc.z = ((2.0f * pos.z) / screen_w) - 1.0f;

    return ndc;
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

	Command_Packet packet = {};

	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Draw_Call;

	// Store the number of vertices to be rendered for this group
	packet.draw_call_info.draw_type = GL_TRIANGLES;
	packet.draw_call_info.total_vertices = ARRAYSIZE(vertices);
	packet.draw_call_info.starting_index = renderer->vertices.size() - packet.draw_call_info.total_vertices;
	packet.draw_call_info.type = SPT_COLOR;
	packet.draw_call_info.blend_mode = renderer->blend_mode;
	// Store the number of vertices to be rendered for this group
	renderer->command_packets.push_back(packet);

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

	Command_Packet packet = {};
	
	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Draw_Call;

	packet.draw_call_info.draw_type = GL_LINES;
	packet.draw_call_info.total_vertices = ARRAYSIZE(vertices);
	// Subtract the already added vertices to get the starting index
	packet.draw_call_info.starting_index = renderer->vertices.size() - packet.draw_call_info.total_vertices;
	packet.draw_call_info.type = SPT_COLOR;
	packet.draw_call_info.blend_mode = renderer->blend_mode;

	// Store the number of vertices to be rendered for this group
	renderer->command_packets.push_back(packet);
	
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

	Color_8 mod;

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

	// Default color mod so the texture displays
	SDL_SetTextureColorMod(result, 255, 255, 255);

	// Default alpha mod
	SDL_SetTextureAlphaMod(result, 255);

	glBindTexture(GL_TEXTURE_2D, result->handle);
	// Set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

int SDL_SetTextureColorMod(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b) {
	if (texture == nullptr) {
		log("ERROR: Invalid texture pointer");
		return -1;
	}

	if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
		log("ERROR: r || g || b");
		return -1;
	}

	texture->mod.r = r;
	texture->mod.g = g;
	texture->mod.b = b;

	return 0;
}

int SDL_GetTextureColorMod(SDL_Texture* texture, Uint8* r, Uint8* g, Uint8* b) {
	if (texture == nullptr) {
		log("ERROR: Invalid texture pointer");
		return -1;
	}
	if (r == nullptr || g == nullptr || b == nullptr) {
		log("ERROR: Invalid r || g || b pointer");
		return -1;
	}

	*r = texture->mod.r; 
	*g = texture->mod.g; 
	*b = texture->mod.b; 
	
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
	if (texture == nullptr) {
		log("ERROR: Invalid texture pointer");
		return -1;
	}
	if (alpha < 0 || alpha > 255) {
		log("ERROR: Invalid alpha mod");
		return -1;
	}

	texture->mod.a = alpha;

	return 0;
}

int SDL_GetTextureAlphaMod(SDL_Texture* texture, Uint8* alpha) {
	if (texture == nullptr) {
		log("ERROR: Invalid texture pointer");
		return -1;
	}
	if (alpha == nullptr) {
		log("ERROR: Invalid alpha pointer");
		return -1;
	}

	*alpha = texture->mod.a; 
	
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
	Color_f c = { 1, 1, 1, 1 };

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
	vertices[0].color.r *= ((float)texture->mod.r / 255.0f);
	vertices[0].color.g *= ((float)texture->mod.g / 255.0f);
	vertices[0].color.b *= ((float)texture->mod.b / 255.0f);
	vertices[0].color.a *= ((float)texture->mod.a / 255.0f);
	vertices[0].uv = bottom_left_uv;

	// Top left
	vertices[1].pos = top_left_ndc;
	vertices[1].color = c;
	vertices[1].color.r *= ((float)texture->mod.r / 255.0f);
	vertices[1].color.g *= ((float)texture->mod.g / 255.0f);
	vertices[1].color.b *= ((float)texture->mod.b / 255.0f);
	vertices[1].color.a *= ((float)texture->mod.a / 255.0f);
	vertices[1].uv = top_left_uv;

	// Top right
	vertices[2].pos = top_right_ndc;
	vertices[2].color = c;
	vertices[2].color.r *= ((float)texture->mod.r / 255.0f);
	vertices[2].color.g *= ((float)texture->mod.g / 255.0f);
	vertices[2].color.b *= ((float)texture->mod.b / 255.0f);
	vertices[2].color.a *= ((float)texture->mod.a / 255.0f);
	vertices[2].uv = top_right_uv;

	// Bottom right
	vertices[3].pos = bottom_right_ndc;
	vertices[3].color = c;
	vertices[3].color.r *= ((float)texture->mod.r / 255.0f);
	vertices[3].color.g *= ((float)texture->mod.g / 255.0f);
	vertices[3].color.b *= ((float)texture->mod.b / 255.0f);
	vertices[3].color.a *= ((float)texture->mod.a / 255.0f);
	vertices[3].uv = bottom_right_uv;

	// Vertices for the vbo
	renderer->vertices.push_back(vertices[0]);
	renderer->vertices.push_back(vertices[1]);
	renderer->vertices.push_back(vertices[2]);
	renderer->vertices.push_back(vertices[3]);

	// Define indices for the two triangles that make up the quad
	Uint32 base_index = static_cast<Uint32>(renderer->vertices.size()) - 4;
	// Bottom left
	renderer->vertices_indices.push_back(base_index + 0);
	// Top left
	renderer->vertices_indices.push_back(base_index + 1); 
	// Top right
	renderer->vertices_indices.push_back(base_index + 2); 

	// Bottom left
	renderer->vertices_indices.push_back(base_index + 0); 
	// Top right
	renderer->vertices_indices.push_back(base_index + 2); 
	// Bottom right
	renderer->vertices_indices.push_back(base_index + 3); 

	Command_Packet packet = {};

	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Draw_Call;

	packet.draw_call_info.draw_type = GL_TRIANGLES;
	packet.draw_call_info.total_vertices = ARRAYSIZE(vertices);
	packet.draw_call_info.starting_index = renderer->vertices.size() - packet.draw_call_info.total_vertices;
	packet.draw_call_info.type = SPT_TEXTURE;
	packet.draw_call_info.texture_handle = texture->handle;
	packet.draw_call_info.total_indices = 6;
	packet.draw_call_info.index_buffer_index = (Uint32)renderer->vertices_indices.size() - packet.draw_call_info.total_indices;
	packet.draw_call_info.blend_mode = texture->blend_mode;
	renderer->command_packets.push_back(packet);

	return 0;
}

V2 rotate_point(const V2 point_ws, const V2 center_ws, float angle) {
    float radians = angle * (float)M_PI / 180.0f;
    float s = (float)sin(radians);
    float c = (float)cos(radians);

    // Translate point back to origin
    V2 translated_point = { point_ws.x - center_ws.x, point_ws.y - center_ws.y };

    // Rotate point
    V2 rotated_point = {
        translated_point.x * c - translated_point.y * s,
        translated_point.x * s + translated_point.y * c
    };

    // Translate point back
    rotated_point.x += center_ws.x;
    rotated_point.y += center_ws.y;

    return rotated_point;
}

int SDL_RenderCopyEx(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, 
                    const float angle, const SDL_Point* center, const SDL_RendererFlip flip) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return -1;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

    if (texture == nullptr) {
        log("ERROR: texture is nullptr");
        return -1;
    }

    SDL_Rect srcrect_final;
    if (srcrect == NULL) {
        srcrect_final = { 0, 0, texture->w, texture->h };
    } else {
        srcrect_final = *srcrect;
    }

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

    V2 bottom_left_src = { (float)srcrect_final.x, (float)srcrect_final.y + srcrect_final.h };
    V2 bottom_right_src = { (float)(srcrect_final.x + srcrect_final.w), (float)(srcrect_final.y + srcrect_final.h) };
    V2 top_right_src = { (float)(srcrect_final.x + srcrect_final.w), (float)srcrect_final.y };
    V2 top_left_src = { (float)srcrect_final.x, (float)srcrect_final.y };

    V2 bottom_left_uv = convert_to_uv_coordinates(bottom_left_src, texture->w, texture->h);
    V2 bottom_right_uv = convert_to_uv_coordinates(bottom_right_src, texture->w, texture->h);
    V2 top_right_uv = convert_to_uv_coordinates(top_right_src, texture->w, texture->h);
    V2 top_left_uv = convert_to_uv_coordinates(top_left_src, texture->w, texture->h);

    Color_f c = { 1.0f, 1.0f, 1.0f, 1.0f };

	V2 center_point_ws = {};
	if (center != NULL) {
		center_point_ws.x = (float)center->x;
		center_point_ws.y = (float)center->y;
	}
	else {
		center_point_ws.x = (float)dstrect_final.x + (float)dstrect_final.w / 2.0f;
		center_point_ws.y = (float)dstrect_final.y + (float)dstrect_final.h / 2.0f;
	}

    V2 dst_points[4] = {
        { (float)(dstrect_final.x + dstrect_final.w), (float)dstrect_final.y },
        { (float)(dstrect_final.x + dstrect_final.w), (float)(dstrect_final.y + dstrect_final.h) },
        { (float)dstrect_final.x, (float)(dstrect_final.y + dstrect_final.h) },
        { (float)dstrect_final.x, (float)dstrect_final.y }
    };

	V2 dst_points_rotated[4] = {};
    for (int i = 0; i < 4; ++i) {
		dst_points_rotated[i] = rotate_point(dst_points[i], center_point_ws, angle);
    }

    if (flip & SDL_FLIP_HORIZONTAL) {
        std::swap(dst_points_rotated[0], dst_points_rotated[1]);
        std::swap(dst_points_rotated[2], dst_points_rotated[3]);
    }
    if (flip & SDL_FLIP_VERTICAL) {
        std::swap(dst_points_rotated[0], dst_points_rotated[3]);
        std::swap(dst_points_rotated[1], dst_points_rotated[2]);
    }

    V2 ndc_points[4];
    for (int i = 0; i < 4; ++i) {
        ndc_points[i] = convert_to_ndc(sdl_renderer, dst_points_rotated[i]);
    }

    Vertex vertices[4];
    vertices[0] = { ndc_points[0], c, bottom_left_uv };
    vertices[1] = { ndc_points[1], c, top_left_uv };
    vertices[2] = { ndc_points[2], c, top_right_uv };
    vertices[3] = { ndc_points[3], c, bottom_right_uv };

    renderer->vertices.push_back(vertices[0]);
    renderer->vertices.push_back(vertices[1]);
    renderer->vertices.push_back(vertices[2]);
    renderer->vertices.push_back(vertices[3]);

    Uint32 base_index = (Uint32)(renderer->vertices.size()) - 4;
    renderer->vertices_indices.push_back(base_index + 0);
    renderer->vertices_indices.push_back(base_index + 1);
    renderer->vertices_indices.push_back(base_index + 2);
    renderer->vertices_indices.push_back(base_index + 0);
    renderer->vertices_indices.push_back(base_index + 2);
    renderer->vertices_indices.push_back(base_index + 3);

    Command_Packet packet = {};
    packet.draw_color = renderer->render_draw_color;
    packet.type = CT_Draw_Call;
    packet.draw_call_info.draw_type = GL_TRIANGLES;
    packet.draw_call_info.total_vertices = ARRAYSIZE(vertices);
    packet.draw_call_info.starting_index = renderer->vertices.size() - packet.draw_call_info.total_vertices;
    packet.draw_call_info.type = SPT_TEXTURE;
    packet.draw_call_info.texture_handle = texture->handle;
    packet.draw_call_info.total_indices = 6;
    packet.draw_call_info.index_buffer_index = (Uint32)renderer->vertices_indices.size() - packet.draw_call_info.total_indices;
    packet.draw_call_info.blend_mode = texture->blend_mode;
    renderer->command_packets.push_back(packet);

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

	Command_Packet packet = {};

	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Set_Clip_Rect;

	packet.clip_rect_info.rect = (SDL_Rect*)rect;
	packet.clip_rect_info.setting = GL_SCISSOR_TEST;

	renderer->command_packets.push_back(packet);

	return 0;
}

void SDL_RenderGetClipRect(SDL_Renderer* sdl_renderer, SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
		return;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

	// Should never reach this but lets guard anyways
	if (rect == nullptr) {
		log("ERROR: rect* is nullptr");
		assert(false);
		return;
	}

	rect->x = renderer->clip_rect.x;
	rect->y = renderer->clip_rect.y;
	rect->w = renderer->clip_rect.w;
	rect->h = renderer->clip_rect.h;
}

bool SDL_RenderIsClipEnabled(SDL_Renderer* sdl_renderer) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return false;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

	return renderer->clip_rect_set;
}

int SDL_RenderSetViewport(SDL_Renderer* sdl_renderer, const SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
        return -1;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

	Command_Packet packet = {};

	packet.type = CT_Set_Viewport;

	packet.viewport_info.rect = (SDL_Rect*)rect;

	renderer->command_packets.push_back(packet);

    return 0;
}

void SDL_RenderGetViewport(SDL_Renderer* sdl_renderer, SDL_Rect* rect) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
		return;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

	// Should never reach this but lets guard anyways
	if (rect == nullptr) {
		log("ERROR: rect* is nullptr");
		assert(false);
		return;
	}

	rect->x = renderer->viewport.x;
	rect->y = renderer->viewport.y;
	rect->w = renderer->viewport.w;
	rect->h = renderer->viewport.h;
}

Color_f color_one = { 1.0f, 0.0f, 0.0f, 1.0f };
Color_f color_two = { 0.0f, 1.0f, 0.0f, 1.0f };
Color_f color_three = { 0.0f, 0.0f, 1.0f, 1.0f };

Vertex_3D cube[36] = {
    // Front face
	{{-0.5, -0.5,  0.5}, color_one, {0, 0}, {0, 0, 1}}, 
	{{ 0.5, -0.5,  0.5}, color_one, {1, 0}, {0, 0, 1}},
	{{ 0.5,  0.5,  0.5}, color_one, {1, 1}, {0, 0, 1}},
    {{-0.5, -0.5,  0.5}, color_one, {0, 0}, {0, 0, 1}},
	{{ 0.5,  0.5,  0.5}, color_one, {1, 1}, {0, 0, 1}},
	{{-0.5,  0.5,  0.5}, color_one, {0, 1}, {0, 0, 1}},

    // Back face
	{{-0.5, -0.5, -0.5}, color_one, {1, 0}, {0, 0, -1}},
	{{-0.5,  0.5, -0.5}, color_one, {1, 1}, {0, 0, -1}},
	{{ 0.5,  0.5, -0.5}, color_one, {0, 1}, {0, 0, -1}},
    {{-0.5, -0.5, -0.5}, color_one, {1, 0}, {0, 0, -1}}, 
	{{ 0.5,  0.5, -0.5}, color_one, {0, 1}, {0, 0, -1}}, 
	{{ 0.5, -0.5, -0.5}, color_one, {0, 0}, {0, 0, -1}},

    // Left face
	{{-0.5, -0.5, -0.5}, color_two, {0, 0}, {-1, 0, 0}},
	{{-0.5, -0.5,  0.5}, color_two, {1, 0}, {-1, 0, 0}}, 
	{{-0.5,  0.5,  0.5}, color_two, {1, 1}, {-1, 0, 0}},
    {{-0.5, -0.5, -0.5}, color_two, {0, 0}, {-1, 0, 0}}, 
	{{-0.5,  0.5,  0.5}, color_two, {1, 1}, {-1, 0, 0}}, 
	{{-0.5,  0.5, -0.5}, color_two, {0, 1}, {-1, 0, 0}},

    // Right face
	{{0.5, -0.5, -0.5}, color_two, {1, 0}, {1, 0, 0}},
	{{0.5,  0.5, -0.5}, color_two, {1, 1}, {1, 0, 0}}, 
	{{0.5,  0.5,  0.5}, color_two, {0, 1}, {1, 0, 0}},
    {{0.5, -0.5, -0.5}, color_two, {1, 0}, {1, 0, 0}}, 
	{{0.5,  0.5,  0.5}, color_two, {0, 1}, {1, 0, 0}}, 
	{{0.5, -0.5,  0.5}, color_two, {0, 0}, {1, 0, 0}},

    // Top face
	{{-0.5,  0.5, -0.5}, color_three, {0, 1}, {0, 1, 0}},
	{{-0.5,  0.5,  0.5}, color_three, {0, 0}, {0, 1, 0}}, 
	{{ 0.5,  0.5,  0.5}, color_three, {1, 0}, {0, 1, 0}},
    {{-0.5,  0.5, -0.5}, color_three, {0, 1}, {0, 1, 0}}, 
	{{ 0.5,  0.5,  0.5}, color_three, {1, 0}, {0, 1, 0}}, 
	{{ 0.5,  0.5, -0.5}, color_three, {1, 1}, {0, 1, 0}},

    // Bottom face
	{{-0.5, -0.5, -0.5}, color_three, {0, 0}, {0, -1, 0}},
	{{ 0.5, -0.5, -0.5}, color_three, {1, 0}, {0, -1, 0}}, 
	{{ 0.5, -0.5,  0.5}, color_three, {1, 1}, {0, -1, 0}},
    {{-0.5, -0.5, -0.5}, color_three, {0, 0}, {0, -1, 0}}, 
	{{ 0.5, -0.5,  0.5}, color_three, {1, 1}, {0, -1, 0}}, 
	{{-0.5, -0.5,  0.5}, color_three, {0, 1}, {0, -1, 0}}
};

// Frustum
MX4 mat4_perspective(float fovy, float aspect)
{
    float z_near = 0.01f;
    float z_far  = 1000.0f;

    float tan_half_fovy = tanf(0.5f * fovy);
    MX4 out = {};
    out.col[0].x = 1.0f / (aspect * tan_half_fovy);
    out.col[1].y = 1.0f / (tan_half_fovy);
    out.col[2].z = -(z_far + z_near) / (z_far - z_near);
    out.col[2].w = -1.0f;
    out.col[3].z = -2.0f * z_far * z_near / (z_far - z_near);
    return out;
}

void prepare_to_draw_cube_faces(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	if (renderer->vbo_cube_faces == 0) {
		glGenBuffers(1, &renderer->vbo_cube_faces);
		glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_cube_faces);
		glBufferData(GL_ARRAY_BUFFER, renderer->vertices_cube_faces.size() * sizeof(Vertex_3D), renderer->vertices_cube_faces.data(), GL_STATIC_DRAW);
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, normal));
	glEnableVertexAttribArray(3);
}

Vertex_3D vertices_face[6] = {
	{ {-0.5, -0.5, 0}, {}, {}, {} }, // Bottom left 
	{ {-0.5,  0.5, 0}, {}, {}, {} }, // Top left
	{ { 0.5,  0.5, 0}, {}, {}, {} }, // Top right

	{ {-0.5, -0.5, 0}, {}, {}, {} }, // Bottom left
	{ { 0.5, -0.5, 0}, {}, {}, {} }, // Bottom right
	{ { 0.5,  0.5, 0}, {}, {}, {} }  // Top right
};


void mp_draw_cube_face(SDL_Renderer* sdl_renderer, MP_Rect_3D rect_ws, SDL_Texture* texture) {
	REF(texture);
	REF(rect_ws);

	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	// I could store the scaling value here when I convert
	// The model / local space of the vertices
	V3 ndc_top_left = convert_to_ndc_v3(sdl_renderer, rect_ws.top_left);
	V3 ndc_top_right = convert_to_ndc_v3(sdl_renderer, rect_ws.top_right);
	V3 ndc_bottom_right = convert_to_ndc_v3(sdl_renderer, rect_ws.bottom_right);
	V3 ndc_bottom_left = convert_to_ndc_v3(sdl_renderer, rect_ws.bottom_left);

	// The model / local space of the vertices
	Vertex_3D vertices[6] = {};
	vertices[0].pos = vertices_face[0].pos;
	renderer->vertices_cube_faces.push_back(vertices[0]);
	vertices[1].pos = vertices_face[1].pos;
	renderer->vertices_cube_faces.push_back(vertices[1]);
	vertices[2].pos = vertices_face[2].pos;
	renderer->vertices_cube_faces.push_back(vertices[2]);
	vertices[3].pos = vertices_face[3].pos;
	renderer->vertices_cube_faces.push_back(vertices[3]);
	vertices[4].pos = vertices_face[4].pos;
	renderer->vertices_cube_faces.push_back(vertices[4]);
	vertices[5].pos = vertices_face[5].pos;
	renderer->vertices_cube_faces.push_back(vertices[5]);

	// Calculate the position of the face

	Command_Packet packet = {};

	packet.type = CT_Draw_Call_3D_Cube_Face;

	packet.draw_call_3d_cube_face.rect = rect_ws;
	// packet.draw_call_3d_cube_face.pos_ws = {
	// 	rect_ws.top_right.x / 2,
	// 	rect_ws.top_right.y / 2,
	// 	rect_ws.top_right.z / 2,
	// };
	packet.draw_call_3d_cube_face.pos_ws = { 0, 0, 0 };
	// packet.draw_call_3d_cube_face.texture_handle = texture->handle;
	
	packet.draw_call_3d_cube_face.total_vertices = ARRAYSIZE(vertices);
	packet.draw_call_3d_cube_face.starting_index = renderer->vertices_cube_faces.size() - ARRAYSIZE(vertices);
	packet.draw_call_3d_cube_face.shader_type = SPT_Cube_Face;

	renderer->command_packets.push_back(packet);
}

// I don't need to upload the data every time I draw a cube
void prepare_to_draw_cube() {
	static GLuint vbo;
	if (vbo == 0) {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, normal));
	glEnableVertexAttribArray(3);
}

void mp_draw_cube(SDL_Renderer* sdl_renderer, V3 pos, SDL_Texture* texture) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	Command_Packet command_packet = {};

	command_packet.type = CT_Draw_Call_3D;

	command_packet.draw_call_3d_cube.pos = pos;
	command_packet.draw_call_3d_cube.texture_handle = texture->handle;

	renderer->command_packets.push_back(command_packet);
}

typedef MX4 Matrix4;
typedef V3 Vector3;
Matrix4 mat4_rotate_x(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[1].y = c;
    rot.col[1].z = s;
    rot.col[2].y = -s;
    rot.col[2].z = c;
    return rot;
}

Matrix4 mat4_rotate_y(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[0].x = c;
    rot.col[0].z = -s;
    rot.col[2].x = s;
    rot.col[2].z = c;
    return rot;
}

Matrix4 mat4_rotate_z(float angle_radians)
{
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    Matrix4 rot = identity_mx_4();
    rot.col[0].x = c;
    rot.col[0].y = s;
    rot.col[1].x = -s;
    rot.col[1].y = c;
    return rot;
}

void draw_cube_face(SDL_Renderer* sdl_renderer, Draw_Call_3D_Cube_Face packet) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	GLuint shader_program = shader_program_types[SPT_Cube_Face];
	if (!shader_program) {
		log("ERROR: Shader program not specified");
		assert(false);
	}
	glUseProgram(shader_program);

	// MX4 world_from_model = translation_matrix_mx_4(cos(x) * x_speed, sin(x) * y_speed, z_pos);
	MX4 world_from_model = translation_matrix_mx_4(packet.pos_ws.x, packet.pos_ws.y, packet.pos_ws.z)/* * mat4_rotate_x(renderer->time)*/;

	// When you take these three matrices and multiple them all together, 
	// you get one matrix that has one transformation.
	MX4 perspective_from_model = renderer->perspective_from_view * renderer->view_from_world * world_from_model;

	GLuint perspective_from_model_loc = glGetUniformLocation(shader_program, "perspective_from_model");
	glUniformMatrix4fv(perspective_from_model_loc, 1, GL_FALSE, perspective_from_model.e);

	glDrawArrays(GL_TRIANGLES, (GLint)packet.starting_index, packet.total_vertices);
}

void draw_cube(SDL_Renderer* sdl_renderer, V3 pos) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;

	GLuint shader_program = shader_program_types[SPT_3D];
	if (!shader_program) {
		log("ERROR: Shader program not specified");
		assert(false);
	}

	// MX4 world_from_model = translation_matrix_mx_4(cos(x) * x_speed, sin(x) * y_speed, z_pos);
	MX4 world_from_model = translation_matrix_mx_4(pos.x, pos.y, pos.z)/* * mat4_rotate_x(renderer->time)*/;

	// When you take these three matrices and multiple them all together, 
	// you get one matrix that has one transformation.
	MX4 perspective_from_model = renderer->perspective_from_view * renderer->view_from_world * world_from_model;

	GLuint world_from_model_loc = glGetUniformLocation(shader_program, "world_from_model");
	GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
	glUseProgram(shader_program);
	glUniformMatrix4fv(world_from_model_loc, 1, GL_FALSE, world_from_model.e);
	MX4 perspective_from_world = renderer->perspective_from_view * renderer->view_from_world;
	glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

	GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
	glUniform1f(time_loc, renderer->time);

	glDrawArrays(GL_TRIANGLES, 0, ARRAYSIZE(cube));
}

Vertex_3D_Temp line[2] = { {0,-0.5,0}, {0,0.5,0} };

// I don't need to upload the data every time I draw a cube
void prepare_to_draw_lines() {
	static GLuint vbo;
	if (vbo == 0) {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Temp), (void*)offsetof(Vertex_3D_Temp, pos));
	glEnableVertexAttribArray(0);
}
#if 0
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

	Command_Packet packet = {};
	
	packet.draw_color = renderer->render_draw_color;
	packet.type = CT_Draw_Call;

	packet.draw_call_info.draw_type = GL_LINES;
	packet.draw_call_info.total_vertices = ARRAYSIZE(vertices);
	// Subtract the already added vertices to get the starting index
	packet.draw_call_info.starting_index = renderer->vertices.size() - packet.draw_call_info.total_vertices;
	packet.draw_call_info.type = SPT_COLOR;
	packet.draw_call_info.blend_mode = renderer->blend_mode;

	// Store the number of vertices to be rendered for this group
	renderer->command_packets.push_back(packet);
	
	return 0;
}
#endif

void mp_draw_line_3d(SDL_Renderer* sdl_renderer, V3 pos_1, V3 pos_2) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
    }
	Renderer* renderer = (Renderer*)sdl_renderer;
	
	// Add a color here

	// NOTE: Confirm this works as intended
	V3 p1 = convert_to_ndc_v3(sdl_renderer, pos_1);
	V3 p2 = convert_to_ndc_v3(sdl_renderer, pos_2);

	Vertex_3D_Temp vertices[2];
	// 10, 0, 0
	vertices[0].pos = p1;
	renderer->vertices_3d.push_back(vertices[0]);
	// 15, 0, 0
	vertices[1].pos = p2;
	renderer->vertices_3d.push_back(vertices[1]);

	Command_Packet packet = {};

	packet.type = CT_Draw_Call_3D_Lines;

	packet.draw_call_3d_line.total_vertices = ARRAYSIZE(vertices);
	packet.draw_call_3d_line.starting_index = renderer->vertices_3d.size() - ARRAYSIZE(vertices);
	packet.draw_call_3d_line.shader_type = SPT_3D_Lines;

	renderer->command_packets.push_back(packet);
}

void execute_draw_line_3d(SDL_Renderer* sdl_renderer, Draw_Call_3D_Line draw_call) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
    }
	Renderer* renderer = (Renderer*)sdl_renderer;

    GLuint shader_program = shader_program_types[draw_call.shader_type];
    if (!shader_program) {
        log("ERROR: Shader program not specified");
        assert(false);
    }
    glUseProgram(shader_program);

	float scale = 10;
	MX4 world_from_model = translation_matrix_mx_4(10, 0, 0) * scaling_matrix_mx_4(scale, scale, scale);
    MX4 perspective_from_model = renderer->perspective_from_view * renderer->view_from_world * world_from_model;
	GLuint perspective_from_model_location = glGetUniformLocation(shader_program, "perspective_from_model");
	glUniformMatrix4fv(perspective_from_model_location, 1, GL_FALSE, perspective_from_model.e);

    glDrawArrays(GL_LINES, (GLint)draw_call.starting_index, draw_call.total_vertices);
}

void execute_set_clip_rect_command(SDL_Renderer* sdl_renderer, Clip_Rect_Info info) {
    if (sdl_renderer == nullptr) {
        log("ERROR: sdl_renderer is nullptr");
        assert(false);
		return;
    }
    Renderer* renderer = (Renderer*)sdl_renderer;

	if (info.rect == nullptr) {
		// Disable clipping
		// GL_SCISSOR_TEST
		glDisable(info.setting);
		renderer->clip_rect_set = false;
	} else {
		int window_width = 0;
		int window_height = 0;
		get_window_size(renderer->window, window_width, window_height);

		// Enable and set the scissor rectangle
		renderer->clip_rect = *info.rect;
		renderer->clip_rect_set = true;
		// Convert because SDL coordinates are inverted
		int scissor_x = renderer->clip_rect.x;
		int scissor_y = window_height - renderer->clip_rect.y - renderer->clip_rect.h;
		int scissor_width = renderer->clip_rect.w;
		int scissor_height = renderer->clip_rect.h;
		glEnable(info.setting);
		glScissor(scissor_x, scissor_y, scissor_width, scissor_height);
	}
}

// Function to calculate forward vector based on yaw angle
V3 calculate_forward(float yaw, float rotation_offset) {
    V3 forward;
	float yaw_temp = yaw - (rotation_offset * ((float)M_PI / 180.0f));
    forward.x = (float)cos(yaw_temp);
    forward.y = 0;
	forward.z = (float)sin(yaw_temp);
    return forward;
}

V3 normalize(const V3& v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	V3 result = {};
    if (length != 0.0f) {
        result = {v.x / length, v.y / length, v.z / length};
    }
	return result;
}

void SDL_RenderPresent(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("ERROR: sdl_renderer is nullptr");
		assert(false);
	}
	Renderer* renderer = (Renderer*)sdl_renderer;
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	renderer->time += 0.01f;

	V2 mouse_delta = get_mouse_delta();
	mouse_delta.x *= 0.01f;
	mouse_delta.y *= 0.01f;
	renderer->yaw += mouse_delta.x;
	renderer->pitch += mouse_delta.y;

    // Calculate the forward vector
    V3 forward = calculate_forward(renderer->yaw, 90.0f);    
	// Normalize the vectors
    forward = normalize(forward);

	V3 right = calculate_forward(renderer->yaw, 180.0f);

	// TODO: There is a bug with holding down the keys simultaneously and 
	// the player moving faster diagonally. 
	if (key_states[VK_SHIFT].pressed_this_frame || key_states[VK_SHIFT].held_down) {
		renderer->player_speed = 0.40f;
	} else {
		renderer->player_speed = 0.20f;
	}
	if (key_states[VK_S].pressed_this_frame || key_states[VK_S].held_down) {
        renderer->player_pos.x -= forward.x * renderer->player_speed;
        renderer->player_pos.y -= forward.y * renderer->player_speed;
        renderer->player_pos.z -= forward.z * renderer->player_speed;
	} 
	if (key_states[VK_W].pressed_this_frame || key_states[VK_W].held_down) {
        renderer->player_pos.x += forward.x * renderer->player_speed;
        renderer->player_pos.y += forward.y * renderer->player_speed;
        renderer->player_pos.z += forward.z * renderer->player_speed;
    } 
	if (key_states[VK_D].pressed_this_frame || key_states[VK_D].held_down) {
		renderer->player_pos.x -= right.x * renderer->player_speed;
        renderer->player_pos.y -= right.y * renderer->player_speed;
        renderer->player_pos.z -= right.z * renderer->player_speed;
	}
	if (key_states[VK_A].pressed_this_frame || key_states[VK_A].held_down) {
		renderer->player_pos.x += right.x * renderer->player_speed;
        renderer->player_pos.y += right.y * renderer->player_speed;
        renderer->player_pos.z += right.z * renderer->player_speed;
	}
	if (key_states[VK_SPACE].pressed_this_frame || key_states[VK_SPACE].held_down) {
		renderer->player_pos.y += renderer->player_speed;
	}
	if (key_states[VK_CONTROL].pressed_this_frame || key_states[VK_CONTROL].held_down) {
		renderer->player_pos.y -= renderer->player_speed;
	}

	// GL_COLOR_BUFFER_BIT: This clears the color buffer, which is responsible for holding the color 
	// information of the pixels. Clearing this buffer sets all the pixels to the color specified by glClearColor.
	// GL_DEPTH_BUFFER_BIT: This clears the depth buffer, which is responsible for holding the depth 
	// information of the pixels. The depth buffer keeps track of the distance from the camera to each pixel
	// to handle occlusion correctly.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int window_width = 0;
	int window_height = 0;
	get_window_size(renderer->window, window_width, window_height);
	SDL_Rect new_viewport = { 0, 0, window_width, window_height };
	glViewport(new_viewport.x, new_viewport.y, new_viewport.w, new_viewport.h);

	// The perspective is gotten from the frustum
	renderer->perspective_from_view = mat4_perspective((float)M_PI / 2.0f, (float)window_width / (float)window_height);

	// This is my camera
	// Move the camera by the same amount of the player but do the negation
	// Doing the multiplication before the translation rotates the view first.
	renderer->view_from_world = mat4_rotate_y(renderer->yaw) /** mat4_rotate_x(renderer->pitch)*/ * translation_matrix_mx_4(
		-renderer->player_pos.x, 
		-renderer->player_pos.y, 
		-renderer->player_pos.z);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	prepare_to_draw_cube();
	for (Command_Packet packet : renderer->command_packets) {
		switch (packet.type) {
		case CT_Draw_Call_3D: {
			if (packet.draw_call_3d_cube.texture_handle) {
				glBindTexture(GL_TEXTURE_2D, packet.draw_call_3d_cube.texture_handle);
			}
			draw_cube(sdl_renderer, packet.draw_call_3d_cube.pos);
			break;
		}
		default: {
			break;
		}
		}
	}

	prepare_to_draw_cube_faces(sdl_renderer);
	for (Command_Packet packet : renderer->command_packets) {
		switch (packet.type) {
		case CT_Draw_Call_3D_Cube_Face: {
			if (packet.draw_call_3d_cube_face.texture_handle) {
				glBindTexture(GL_TEXTURE_2D, packet.draw_call_3d_cube_face.texture_handle);
			}
			draw_cube_face(sdl_renderer, packet.draw_call_3d_cube_face);
			break;
		}
		default: {
			break;
		}
		}
	}
	renderer->vertices_cube_faces.clear();
	glDisable(GL_DEPTH_TEST);

	prepare_to_draw_lines();
	for (Command_Packet packet : renderer->command_packets) {
		switch (packet.type) {
		case CT_Draw_Call_3D_Lines: {
			execute_draw_line_3d(sdl_renderer, packet.draw_call_3d_line);
			break;
		}
		default: {
			break;
		}
		}
	}
	renderer->vertices_3d.clear();

	// This rebind may be pointless if I am only going with one vbo
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferData(GL_ARRAY_BUFFER, renderer->vertices.size() * sizeof(Vertex), renderer->vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->vertices_indices.size() * sizeof(Uint32), renderer->vertices_indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	// This could become the flush function
	for (Command_Packet command_packet : renderer->command_packets) {
		switch (command_packet.type) {
		case CT_Draw_Call: {
			Draw_Call_Info info = command_packet.draw_call_info;
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

			if (info.total_indices > 0) {
				glDrawElements(info.draw_type, info.total_indices, GL_UNSIGNED_INT, (void*)(info.index_buffer_index * sizeof(unsigned int)));
			}
			else {
				glDrawArrays(info.draw_type, (GLuint)info.starting_index, info.total_vertices);
			}
			break;
		}
		case CT_Set_Clip_Rect: {
			Clip_Rect_Info info = command_packet.clip_rect_info;
			execute_set_clip_rect_command(sdl_renderer, command_packet.clip_rect_info);
			break;
		}
		case CT_Set_Viewport: {
			get_window_size(renderer->window, window_width, window_height);

			Viewport_Info info = command_packet.viewport_info;
			if (info.rect == NULL) {
				// If rect is NULL, use the entire window as the viewport
				renderer->viewport = { 0, 0, window_width, window_height };
			}
			else {
				// Convert SDL rect coordinates to OpenGL coordinates
				renderer->viewport = { info.rect->x, window_height - info.rect->y - info.rect->h, info.rect->w, info.rect->h };
			}
			glViewport(info.rect->x, info.rect->y, info.rect->w, info.rect->h);
			break;
		}
		case CT_Clear_Screen: {
			Clear_Screen_Info info = command_packet.clear_screen_info;

			Color_f c = convert_color_8_to_floating_point(info.clear_draw_color);

			// Create functions that execute the commands in here
			Clip_Rect_Info clip_rect_info = {};
			clip_rect_info.setting = GL_SCISSOR_TEST;
			execute_set_clip_rect_command(sdl_renderer, clip_rect_info);

			glClearColor(c.r, c.g, c.b, c.a);
			glClear(GL_COLOR_BUFFER_BIT);

			if (renderer->clip_rect_set) {
				clip_rect_info.setting = GL_SCISSOR_TEST;
				clip_rect_info.rect = &renderer->clip_rect;
				execute_set_clip_rect_command(sdl_renderer, clip_rect_info);
			}
			break;
		}
		}
	}
	renderer->command_packets.clear();
	renderer->vertices.clear();
	renderer->vertices_indices.clear();

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
