#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <assert.h>

#define STB_PERLIN_IMPLEMENTATION
#include "Perlin.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utility.h"
#include "Math.h"
#include "gl_renderer.h"

struct Key_State {
	bool pressed_this_frame;
	bool held_down;
};

extern std::unordered_map<LPARAM, Key_State> key_states;
std::unordered_map<LPARAM, Key_State> key_states;

V2 last_mouse_position;
V2 current_mouse_position;

V2 get_mouse_delta() {
	V2 frame_mouse_delta;
	frame_mouse_delta.x = current_mouse_position.x - last_mouse_position.x;
	frame_mouse_delta.y = current_mouse_position.y - last_mouse_position.y;
	return frame_mouse_delta;
}

void reset_Pressed_This_Frame() {
	for (auto& key_State : key_states) {
		key_states[key_State.first].pressed_this_frame = false;
	}
}

struct GL_Rect_3D {
    V3 top_left;
    V3 top_right;
    V3 bottom_right;
    V3 bottom_left;
};

struct Chunk_Vbo {
	GLuint vbo;
	GLuint texture_sprite_sheet_handle;
	GLuint total_vertices;
};

struct Open_GL {
	GLuint vao;
	GLuint vbo_lines;
	GLuint vbo_cubes;
	GLuint vbo_cube_faces;
	GLuint vbo_strings;

	std::vector<Chunk_Vbo> chunk_vbos;
};

struct Vertex_3D_Line {
	V3 pos;
};

struct Cube_Draw {
	V3 pos_ws;
	GLuint texture_handle;
};

struct Cube_Face {
	// World space positions
	V3 p1, p2, p3, p4;
};

struct Vertex_String {
	V2 pos;
	V2 uv;
};

struct GL_Renderer {
	HWND window;
	HDC hdc;

	Open_GL open_gl = {};
	std::vector<Vertex_3D_Line> lines_vertices;
	std::vector<Cube_Draw> cubes;
	std::vector<Vertex_String> strings;

	float time;
	float player_speed;
	V3 player_pos = { 0, 0, 0 };

	float pitch;
	float roll;
	float yaw;

	MX4 perspective_from_view;
	MX4 view_from_world;
};

GL_Renderer* create_gl_renderer(HWND window) {
	GL_Renderer* result = new GL_Renderer();

	result->window = window;

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

	result->hdc = GetDC(window);

	int pixelFormatIndex = ChoosePixelFormat(result->hdc, &pfd);

	if (pixelFormatIndex == 0) {
		log("Error: ChoosePixelFormat function returned 0");
		return NULL;
	}

	if (!SetPixelFormat(result->hdc, pixelFormatIndex, &pfd)) {
		log("Error: SetPixelFormat function returned false");
		return NULL;
	}

	HGLRC temp_context = wglCreateContext(result->hdc);
	wglMakeCurrent(result->hdc, temp_context);
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

	HGLRC glRendereringContext = wglCreateContextAttribsARB(result->hdc, 0, attrib_list);
	// Make the new context current
	wglMakeCurrent(result->hdc, glRendereringContext);
	// Delete the garbage context
	wglDeleteContext(temp_context);

	if (glRendereringContext == NULL) {
		log("Error: wglCreateContext function returned false");
		return NULL;
	}

	glGenVertexArrays(1, &result->open_gl.vao);
	glBindVertexArray(result->open_gl.vao);

	load_shaders();

	return result;
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

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44

void gl_update_renderer(GL_Renderer* gl_renderer) {
    if (gl_renderer == nullptr) {
        log("Error: gl_renderer is nullptr");
        assert(false);
		return;
    }

	gl_renderer->time += 0.01f;

	V2 mouse_delta = get_mouse_delta();
	mouse_delta.x *= 0.01f;
	mouse_delta.y *= 0.01f;
	gl_renderer->yaw += mouse_delta.x;
	gl_renderer->pitch += mouse_delta.y;

    // Clamp the pitch to avoid flipping the camera
    if (gl_renderer->pitch > 89.0f) {
        gl_renderer->pitch = 89.0f;
    } else if (gl_renderer->pitch < -89.0f) {
        gl_renderer->pitch = -89.0f;
    }

    // Calculate the forward vector
    V3 forward = calculate_direction_normalized(gl_renderer->yaw, gl_renderer->pitch, 90.0f);    
	V3 right = calculate_direction_normalized(gl_renderer->yaw, gl_renderer->pitch, 180.0f);

	// TODO: There is a bug with holding down the keys simultaneously and 
	// the player moving faster diagonally. 
	if (key_states[VK_SHIFT].pressed_this_frame || key_states[VK_SHIFT].held_down) {
		gl_renderer->player_speed = 0.80f;
	} else {
		gl_renderer->player_speed = 0.20f;
	}
	if (key_states[VK_S].pressed_this_frame || key_states[VK_S].held_down) {
        gl_renderer->player_pos.x -= forward.x * gl_renderer->player_speed;
        gl_renderer->player_pos.y += forward.y * gl_renderer->player_speed;
        gl_renderer->player_pos.z -= forward.z * gl_renderer->player_speed;
	} 
	if (key_states[VK_W].pressed_this_frame || key_states[VK_W].held_down) {
        gl_renderer->player_pos.x += forward.x * gl_renderer->player_speed;
        gl_renderer->player_pos.y -= forward.y * gl_renderer->player_speed;
        gl_renderer->player_pos.z += forward.z * gl_renderer->player_speed;
    } 
	if (key_states[VK_D].pressed_this_frame || key_states[VK_D].held_down) {
		gl_renderer->player_pos.x -= right.x * gl_renderer->player_speed;
        gl_renderer->player_pos.y -= right.y * gl_renderer->player_speed;
        gl_renderer->player_pos.z -= right.z * gl_renderer->player_speed;
	}
	if (key_states[VK_A].pressed_this_frame || key_states[VK_A].held_down) {
		gl_renderer->player_pos.x += right.x * gl_renderer->player_speed;
        gl_renderer->player_pos.y += right.y * gl_renderer->player_speed;
        gl_renderer->player_pos.z += right.z * gl_renderer->player_speed;
	}
	if (key_states[VK_SPACE].pressed_this_frame || key_states[VK_SPACE].held_down) {
		gl_renderer->player_pos.y += gl_renderer->player_speed;
	}
	if (key_states[VK_CONTROL].pressed_this_frame || key_states[VK_CONTROL].held_down) {
		gl_renderer->player_pos.y -= gl_renderer->player_speed;
	}

	int window_width = 0;
	int window_height = 0;
	get_window_size(gl_renderer->window, window_width, window_height);
	glViewport(0, 0, window_width, window_height);

	// The perspective is gotten from the frustum
	gl_renderer->perspective_from_view = mat4_perspective((float)M_PI / 2.0f, (float)window_width / (float)window_height);

	// This is my camera
	// Move the camera by the same amount of the player but do the negation
	// Doing the multiplication before the translation rotates the view first.
	gl_renderer->view_from_world = mat4_rotate_x(gl_renderer->pitch) * mat4_rotate_y(gl_renderer->yaw) * translation_matrix_mx_4(
		-gl_renderer->player_pos.x, 
		-gl_renderer->player_pos.y, 
		-gl_renderer->player_pos.z);
}

void gl_draw_line(GL_Renderer* gl_renderer, V3 p1, V3 p2) {
	if (gl_renderer == nullptr) {
		log("Error: gl_renderer is nullptr");
		assert(false);
		return;
	}

	Vertex_3D_Line vertices[2] = {};
	// World positions
	vertices[0].pos = p1;
	gl_renderer->lines_vertices.push_back(vertices[0]);
	vertices[1].pos = p2;
	gl_renderer->lines_vertices.push_back(vertices[1]);
}

void draw_cube_with_lines(GL_Renderer* gl_renderer, int w, int l, int h, V3 pos) {
    // Front face
    gl_draw_line(gl_renderer, { pos.x, pos.y, pos.z }, { pos.x + w, pos.y, pos.z });				 // Bottom edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y, pos.z }, { pos.x + w, pos.y + h, pos.z });		 // Right edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y + h, pos.z }, { pos.x, pos.y + h, pos.z });		 // Top edge
    gl_draw_line(gl_renderer, { pos.x, pos.y + h, pos.z }, { pos.x, pos.y, pos.z });				 // Left edge

    // Back face
    gl_draw_line(gl_renderer, { pos.x, pos.y, pos.z + l }, { pos.x + w, pos.y, pos.z + l });		 // Bottom edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y, pos.z + l }, { pos.x + w, pos.y + h, pos.z + l }); // Right edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y + h, pos.z + l }, { pos.x, pos.y + h, pos.z + l }); // Top edge
    gl_draw_line(gl_renderer, { pos.x, pos.y + h, pos.z + l }, { pos.x, pos.y, pos.z + l });		 // Left edge

    // Connecting edges
    gl_draw_line(gl_renderer, { pos.x, pos.y, pos.z }, { pos.x, pos.y, pos.z + l });				 // Bottom left edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y, pos.z }, { pos.x + w, pos.y, pos.z + l });		 // Bottom right edge
    gl_draw_line(gl_renderer, { pos.x + w, pos.y + h, pos.z }, { pos.x + w, pos.y + h, pos.z + l }); // Top right edge
    gl_draw_line(gl_renderer, { pos.x, pos.y + h, pos.z }, { pos.x, pos.y + h, pos.z + l });		 // Top left edge
}

void gl_upload_and_draw_lines_vbo(GL_Renderer* gl_renderer) {
	if (gl_renderer->open_gl.vbo_lines == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_lines);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_lines);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_lines);
	glBufferData(
		GL_ARRAY_BUFFER, 
		gl_renderer->lines_vertices.size() * sizeof(Vertex_3D_Line), 
		gl_renderer->lines_vertices.data(), 
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Line), (void*)offsetof(Vertex_3D_Line, pos));
	glEnableVertexAttribArray(0);

    GLuint shader_program = shader_program_types[SPT_3D_Lines];
    if (!shader_program) {
        log("Error: Shader program not specified");
        assert(false);
    }
    glUseProgram(shader_program);

    MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;
	GLuint perspective_from_world_location = glGetUniformLocation(shader_program, "perspective_from_world");
	glUniformMatrix4fv(perspective_from_world_location, 1, GL_FALSE, perspective_from_world.e);

	glDrawArrays(GL_LINES, 0, (GLsizei)gl_renderer->lines_vertices.size());
	gl_renderer->lines_vertices.clear();
}

struct MP_Rect_2D {
	int x, y;
	int w, h;
};

struct Color_8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct GL_Texture {
	GLuint handle;
	int w, h;

	int pitch;
	// Locked if null
	void* pixels;
	MP_Rect_2D portion;

	Color_8 mod = { 255, 255, 255, 255 };
};

GL_Texture* gl_create_texture(GL_Renderer* gl_renderer, int w, int h) {
	if (gl_renderer == nullptr) {
		log("ERROR: gl_renderer is nullptr");
		assert(false);
	}

	GL_Texture* result = new GL_Texture();
	result->w = w;
	result->h = h;
	result->pitch = 0;
	result->pixels = NULL;

	// Default color mod so the texture displays
	// SDL_SetTextureColorMod(result, 255, 255, 255);

	// Default alpha mod
	// SDL_SetTextureAlphaMod(result, 255);

	// This is a STATE SETTING
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenTextures(1, &result->handle);
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

int gl_lock_texture(GL_Texture* texture, const MP_Rect_2D* rect, void **pixels, int *pitch) {
	if (texture == NULL) {
		log("ERROR: Invalid texture");
		return -1;
	}

	if (texture->pixels != NULL) {
		log("ERROR: Texture is already locked");
		return -1;
	}

	MP_Rect_2D full_rect = { 0, 0, texture->w, texture->h };
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

	// RGBA = 8 + 8 + 8 + 8 = 32 bits = 8 bytes
	int bytes_per_pixel = 8;
	int buffer_size = texture->portion.w * texture->portion.h * bytes_per_pixel;
	uint8_t* buffer = new uint8_t[buffer_size];
	if (buffer == NULL) {
		log("ERROR: Memory allocation failed");
		return -1;
	}

	*pixels = buffer;
	*pitch = texture->portion.w * bytes_per_pixel;
	texture->pixels = *pixels;

	return 0;
}

// Uploading the texture to the GPU
// Just changing the pixels
void gl_unlock_texture(GL_Texture* texture) {
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

struct Image {
	int width;
	int height;
	const char* file_Path;
	GL_Texture* texture;
	unsigned char* pixel_Data;
};

Image create_Image(GL_Renderer* gl_renderer, const char* file_Path) {
	if (gl_renderer == nullptr) {
		log("Error: gl_rendererer == nullptr");
		assert(false);
	}

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

	GL_Texture* texture = gl_create_texture(gl_renderer, result.width, result.height);

	if (texture == NULL) {
		log("ERROR: SDL_CreateTexture returned NULL");
		return result;
	}

	result.texture = texture;

	void* pixels;
	int pitch;
	gl_lock_texture(texture, NULL, &pixels, &pitch);
	my_Memory_Copy(pixels, data, (result.width * result.height) * 4);
	gl_unlock_texture(texture);

	return result;
}

enum Image_Type {
    IT_Air,
	IT_Cobblestone,
	IT_Dirt,
	IT_Grass,
	IT_Texture_Sprite_Sheet,

	IT_Total
};

Image images[IT_Total] = {};

void init_images(GL_Renderer* gl_renderer) {
	if (gl_renderer == nullptr) {
		log("ERROR: gl_renderer is nullptr");
		assert(false);
		return;
	}

	images[IT_Cobblestone] = create_Image(gl_renderer, "assets\\cobblestone.png");
	images[IT_Dirt] = create_Image(gl_renderer, "assets\\dirt.png");
	images[IT_Grass] = create_Image(gl_renderer, "assets\\grass.png");
	images[IT_Texture_Sprite_Sheet] = create_Image(gl_renderer, "assets\\texture_sprite_sheet.png");
}

struct Font {
	int image_w;
	int image_h;
	int char_w;
	int char_h;

	GL_Texture* texture;
};

Font load_font(GL_Renderer* gl_renderer, const char* file_name) {
	int width, height, channels;
	unsigned char* data = stbi_load(file_name, &width, &height, &channels, 4);
	if (data == NULL) {
		log("Error: stbi_load() returned null");
		assert(false);
	}
	DEFER{
		stbi_image_free(data);
	};

	Font result;
	result.image_w = width;
	result.image_h = height;
	result.char_w = width / 18;
	result.char_h = width / 7;
	
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int index = 0;
			index = (4 * ((y * width) + x));
			if (data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 0) {
				data[index + 3] = 0;
			}
		}
	}

	GL_Texture* texture = gl_create_texture(gl_renderer, width, height);
	if (texture == NULL) {
		log("Error: gl_create_texture returned null");
		assert(false);
	}

	result.texture = texture;

	void* pixels;
	int pitch;
	gl_lock_texture(texture, NULL, &pixels, &pitch);
	my_Memory_Copy(pixels, data, (width * height) * 4);
	gl_unlock_texture(texture);

	return result;
}

V2 convert_to_ndc(GL_Renderer* gl_renderer, V2 pos) {
	if (gl_renderer == nullptr) {
		log("Error: gl_renderer is nullptr");
		assert(false);
	}

	int screen_w, screen_h;
	get_window_size(gl_renderer->window, screen_w, screen_h);

	V2 ndc;
	ndc.x = ((2.0f * pos.x) / screen_w) - 1.0f;
	ndc.y = (1.0f - ((2.0f * pos.y) / screen_h));
	return ndc;
}

V2 convert_to_ndc(GL_Renderer* gl_renderer, int x, int y) {
	return convert_to_ndc(gl_renderer, { (float)x, (float)y });
}

V2 convert_to_uv_coordinates(V2 pos, int w, int h) {
	V2 uv;
	uv.x = pos.x / w;
	uv.y = pos.y / h;
	return uv;
}

void draw_character(GL_Renderer* gl_renderer, Font* font, char character, int pos_x, int pos_y, int size) {
    int ascii_dec = (int)character - (int)' ';
    int chars_per_row = (font->image_w / font->char_w);

    MP_Rect_2D src = {};
	// Position in the row
    src.x = (ascii_dec % chars_per_row) * font->char_w;
	// Which row we are in
    src.y = (ascii_dec / chars_per_row) * font->char_h;
    src.w = font->char_w;
    src.h = font->char_h;

    // Position of the character on the screen
    MP_Rect_2D dst = {};
    dst.x = pos_x;
    dst.y = pos_y;
    dst.w = (int)(font->char_w * size);
    dst.h = (int)(font->char_h * size);

    V2 bottom_left_src = { (float)src.x, (float)src.y + src.h };
    V2 bottom_right_src = { (float)(src.x + src.w), (float)(src.y + src.h) };
    V2 top_right_src = { (float)(src.x + src.w), (float)src.y };
    V2 top_left_src = { (float)src.x, (float)src.y };

    V2 bottom_left_uv = convert_to_uv_coordinates(bottom_left_src, font->texture->w, font->texture->h);
    V2 bottom_right_uv = convert_to_uv_coordinates(bottom_right_src, font->texture->w, font->texture->h);
    V2 top_right_uv = convert_to_uv_coordinates(top_right_src, font->texture->w, font->texture->h);
    V2 top_left_uv = convert_to_uv_coordinates(top_left_src, font->texture->w, font->texture->h);

    V2 top_left_dst = { (float)dst.x, (float)dst.y };
    V2 top_right_dst = { (float)(dst.x + dst.w), (float)dst.y };
    V2 bottom_right_dst = { (float)(dst.x + dst.w), (float)(dst.y + dst.h) };
    V2 bottom_left_dst = { (float)dst.x, (float)(dst.y + dst.h) };

    V2 top_left_ndc = convert_to_ndc(gl_renderer, top_left_dst);
    V2 top_right_ndc = convert_to_ndc(gl_renderer, top_right_dst);
    V2 bottom_right_ndc = convert_to_ndc(gl_renderer, bottom_right_dst);
    V2 bottom_left_ndc = convert_to_ndc(gl_renderer, bottom_left_dst);

	Vertex_String vertices[6];

	vertices[0].pos = bottom_left_ndc;
	vertices[0].uv = bottom_left_uv;
	gl_renderer->strings.push_back(vertices[0]);

	vertices[1].pos = top_left_ndc;
	vertices[1].uv = top_left_uv;
	gl_renderer->strings.push_back(vertices[1]);

	vertices[2].pos = top_right_ndc;
	vertices[2].uv= top_right_uv;
	gl_renderer->strings.push_back(vertices[2]);

	vertices[3].pos = top_right_ndc;
	vertices[3].uv= top_right_uv;
	gl_renderer->strings.push_back(vertices[3]);

	vertices[4].pos = bottom_right_ndc;
	vertices[4].uv = bottom_right_uv;
	gl_renderer->strings.push_back(vertices[4]);

	vertices[5].pos = bottom_left_ndc;
	vertices[5].uv = bottom_left_uv;
	gl_renderer->strings.push_back(vertices[5]);
}

void draw_string(GL_Renderer* gl_renderer, Font* font, const char* str, int pos_x, int pos_y, int size, bool center) {
    int off_x = 0;
    int index = 0;
    char iterator = str[index];
    size_t len_pixels = strlen(str);
    len_pixels *= font->char_w * size;

    int char_pos_x = 0;
    int char_pos_y = 0;
    if (center) {
        char_pos_x = (pos_x - (int)(len_pixels / 2));
        char_pos_y = (pos_y - ((font->char_h * size) / 2));
    }
    else {
        char_pos_x = pos_x;
        char_pos_y = pos_y;
    }

    while (iterator != '\0') {
        char_pos_x += off_x;
        draw_character(gl_renderer, font, iterator, char_pos_x, char_pos_y, size);
        off_x = font->char_w * size;
        index++;
        iterator = str[index];
    }
}

void gl_upload_and_draw_2d_string(GL_Renderer* gl_renderer, Font* font) {
	if (gl_renderer->open_gl.vbo_strings == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_strings);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_strings);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_String) * gl_renderer->strings.size(), gl_renderer->strings.data(), GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_strings);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_String), (void*)offsetof(Vertex_String, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_String), (void*)offsetof(Vertex_String, uv));
	glEnableVertexAttribArray(1);

	glBindTexture(GL_TEXTURE_2D, font->texture->handle);

	GLuint shader_program = shader_program_types[SPT_String];
	if (!shader_program) {
		log("ERROR: Shader program not specified");
		assert(false);
	}
	glUseProgram(shader_program);

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)gl_renderer->strings.size());
	gl_renderer->strings.clear();
}

struct Vertex_3D_Cube {
	V3 pos;
	Color_f color;
	V2 uv;
	V3 normal;
};

Color_f color_one = { 1.0f, 0.0f, 0.0f, 1.0f };
Color_f color_two = { 0.0f, 1.0f, 0.0f, 1.0f };
Color_f color_three = { 0.0f, 0.0f, 1.0f, 1.0f };

Vertex_3D_Cube cube[36] = {
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

void gl_upload_cube_vbo(GL_Renderer* gl_renderer) {
	if (gl_renderer->open_gl.vbo_cubes == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_cubes);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_cubes);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_cubes);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, normal));
	glEnableVertexAttribArray(3);
}

void draw_cube(GL_Renderer* gl_renderer, V3 pos, GL_Texture* texture) {
	Cube_Draw result;

	result.pos_ws = pos;
	result.texture_handle = texture->handle;

	gl_renderer->cubes.push_back(result);
}

void gl_draw_cubes(GL_Renderer* gl_renderer) {
	for (Cube_Draw current_cube : gl_renderer->cubes) {
		glBindTexture(GL_TEXTURE_2D, current_cube.texture_handle);

		GLuint shader_program = shader_program_types[SPT_3D];
		if (!shader_program) {
			log("ERROR: Shader program not specified");
			assert(false);
		}
		glUseProgram(shader_program);

		// MX4 world_from_model = translation_matrix_mx_4(cos(x) * x_speed, sin(x) * y_speed, z_pos);
		MX4 world_from_model = translation_matrix_mx_4(current_cube.pos_ws.x, current_cube.pos_ws.y, current_cube.pos_ws.z)/* * mat4_rotate_x(renderer->time)*/;
		GLuint world_from_model_loc = glGetUniformLocation(shader_program, "world_from_model");
		glUniformMatrix4fv(world_from_model_loc, 1, GL_FALSE, world_from_model.e);

		MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;
		GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
		glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

		GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
		glUniform1f(time_loc, gl_renderer->time);

		glDrawArrays(GL_TRIANGLES, 0, ARRAYSIZE(cube));
	}
	gl_renderer->cubes.clear();
}

GL_Texture* get_image(Image_Type type) {
	GL_Texture* result = nullptr;

	switch (type) {
	case IT_Grass: {
		result = images[type].texture;
		break;
	}
	case IT_Dirt: {
		result = images[type].texture;
		break;
	}
	case IT_Cobblestone: {
		result = images[type].texture;
		break;
	}
	case IT_Texture_Sprite_Sheet: {
		result = images[type].texture;
		break;
	}
	case IT_Air: {
		result = nullptr;
		break;
	}
	default: { // The default is just air 
		break;
	}
	}

	return result;
}

void draw_cube_type(GL_Renderer* gl_renderer, V3 pos, Image_Type type) {
	GL_Texture* result = get_image(type);

	if (result != nullptr) {
		draw_cube(gl_renderer, pos, result);
	}
}

struct Cube {
	Image_Type type = IT_Air;
};

const int CHUNK_SIZE = 40;
struct Chunk {
	Cube cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {};
};

const int WORLD_SIZE_WIDTH = 5;
const int WORLD_SIZE_LENGTH = 5;
const int WORLD_SIZE_HEIGHT = 2;
Chunk world_chunks[WORLD_SIZE_WIDTH][WORLD_SIZE_HEIGHT][WORLD_SIZE_LENGTH] = {};

int height_map[(WORLD_SIZE_WIDTH * CHUNK_SIZE)][(WORLD_SIZE_LENGTH * CHUNK_SIZE)] = {};

void generate_height_map(float noise) {
	for (int x = 0; x < WORLD_SIZE_WIDTH * CHUNK_SIZE; x++) {
		for (int z = 0; z < WORLD_SIZE_LENGTH * CHUNK_SIZE; z++) {
			float height_value = stb_perlin_noise3((float)x / (float)noise, 0, z / noise, 0, 0, 0);
			// Normalize and convert the perlin value into a height map value
			int height = (int)((height_value + 1.0f) * 0.5f * CHUNK_SIZE);
			height += (WORLD_SIZE_HEIGHT * CHUNK_SIZE) - CHUNK_SIZE;
			height_map[x][z] = height;
		}
	}
}

void generate_world_chunk(int x_arr_pos, int y_arr_pos, int z_arr_pos, float noise) {
	int final_x_arr_pos = clamp(x_arr_pos, 0, WORLD_SIZE_WIDTH);
	int final_y_arr_pos = clamp(y_arr_pos, 0, WORLD_SIZE_HEIGHT);
	int final_z_arr_pos = clamp(z_arr_pos, 0, WORLD_SIZE_LENGTH);

	Chunk new_chunk = {};
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				float world_space_x = x + (float)final_x_arr_pos * CHUNK_SIZE;
				float world_space_y = y + (float)final_y_arr_pos * CHUNK_SIZE;
				float world_space_z = z + (float)final_z_arr_pos * CHUNK_SIZE;
				float perlin_result = stb_perlin_noise3(
					(float)world_space_x / (float)noise, 
					(float)world_space_y / (float)noise,
					(float)world_space_z / (float)noise, 
					0, 0, 0 // wrapping
				);

				int height = height_map[(int)world_space_x][(int)world_space_z];

				Image_Type result = IT_Air;

				// We know this is the top chunk
				// Draw the chunk
				if (world_space_y < height) {
					if (world_space_y == height - 1) {
						result = IT_Grass;
					} else if (world_space_y < height - 1 && world_space_y >= height - 4) {
						result = IT_Dirt;
					} else if (world_space_y < height - 4 && world_space_y >= height - 8) {
						result = IT_Cobblestone;
					} else {
						if (perlin_result > -0.3) {
							result = IT_Cobblestone;
						} else {
							result = IT_Air;
						}
					}
				// Don't draw
				} else {
					result = IT_Air;
				}

				new_chunk.cubes[x][y][z].type = result;
			}
		}
	}

	world_chunks[final_x_arr_pos][final_y_arr_pos][final_z_arr_pos] = new_chunk;
}

struct Vertex_3D_Faces {
	V3 pos;
	V2 uv;
};

// Order of the sprite sheet = Grass | Dirt | Cobblestone
V2 get_texture_sprite_sheet_uv_coordinates(Image_Type type, int pos_x, int pos_y) {
	V2 result = {};
	// Currently only have one row
	result.y = (float)pos_y;

	switch (type) {
	case IT_Grass: {
		// 0 0
		if (pos_x == 0) {
			result.x = 0.0f;
		} else if (pos_x == 1) {
			result.x = lerp(0, 1, 0.25);
		}
		break;
	}
	case IT_Dirt: {
		if (pos_x == 0) {
			result.x = lerp(0, 1, 0.25);
		} else if (pos_x == 1) {
			result.x = lerp(0, 1, 0.50);
		}
		break;
	}
	case IT_Cobblestone: {
		if (pos_x == 0) {
			result.x = lerp(0, 1, 0.50);
		} else if (pos_x == 1) {
			result.x = lerp(0, 1, 0.75);
		}
		break;
	}
	default: {
		log("Error: Invalid image type to pull from texture sprite sheet");
		assert(false);
		break;
	}
	}

	return result;
}

const int CUBE_SIZE = 1;

void generate_chunk_vbo(GL_Renderer* gl_renderer, V3 chunk_arr_pos) {
	V3 chunk_ws_pos = {chunk_arr_pos.x * CHUNK_SIZE, chunk_arr_pos.y * CHUNK_SIZE, chunk_arr_pos.z * CHUNK_SIZE};
	Chunk* chunk = &world_chunks[(int)chunk_arr_pos.x][(int)chunk_arr_pos.y][(int)chunk_arr_pos.z];

	// Generate the data to be sent to the GPU
	std::vector<Vertex_3D_Faces> faces_vertices = {};
	Chunk_Vbo chunk_vbo = {};

	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				// Grab the cube I want
				V3 cube_ws_pos = chunk_ws_pos;
				cube_ws_pos.x += x;
				cube_ws_pos.y += y;
				cube_ws_pos.z += z;

				Cube* current_cube = &chunk->cubes[x][y][z];
				if (current_cube->type != IT_Air) {
					// The cubes around the current cube
					Cube back_cube = {};
					Cube top_cube = {};
					Cube right_cube = {};

					Cube front_cube = {};
					Cube bottom_cube = {};
					Cube left_cube = {};

					// Loop over the faces
					// This currently doesn't take into account adjacent chunks when 
					// drawing the faces
					if (x + 1 < CHUNK_SIZE) {
						back_cube = chunk->cubes[x + 1][y][z];
					}
					if (y + 1 < CHUNK_SIZE) {
						top_cube = chunk->cubes[x][y + 1][z];
					}
					if (z + 1 < CHUNK_SIZE) {
						right_cube = chunk->cubes[x][y][z + 1];
					}
					if (x > 0) {
						front_cube = chunk->cubes[x - 1][y][z];
					}
					if (y > 0) {
						bottom_cube = chunk->cubes[x][y - 1][z];
					}
					if (z > 0) {
						left_cube = chunk->cubes[x][y][z - 1];
					}

					// NOTE: The cube is draw from the center. I might just 
					// have the faces draw from the bottom corner

					// Emit a face
					// The points go in clockwise order
					// NOTE: This offset is wrong
					Image_Type t = current_cube->type;
					if (back_cube.type == IT_Air) {
						// Emit a face
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.x += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.y += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}

					if (top_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.y += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.y += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.y += CUBE_SIZE;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}

					if (right_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.z += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.y += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}

					if (front_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;

						face.p2 = cube_ws_pos;
						face.p2.y += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}

					if (bottom_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;

						face.p2 = cube_ws_pos;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = { 0, 0 };
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}

					if (left_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;

						face.p2 = cube_ws_pos;
						face.p2.y += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						// First Triangle
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						// Second Triangle
						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						chunk_vbo.total_vertices += 6;
					}
				}
			}
		}
	}

	glGenBuffers(1, &chunk_vbo.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, chunk_vbo.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_3D_Faces) * faces_vertices.size(), faces_vertices.data(), GL_STATIC_DRAW);
	gl_renderer->open_gl.chunk_vbos.push_back(chunk_vbo);
	faces_vertices.clear();
};

void gl_draw_cube_faces_vbo(GL_Renderer* gl_renderer, GLuint textures_handle) {
	for (const Chunk_Vbo& chunk_vbo : gl_renderer->open_gl.chunk_vbos) {
		glBindBuffer(GL_ARRAY_BUFFER, chunk_vbo.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, uv));
		glEnableVertexAttribArray(1);

		// Bind the sprite sheet with all the textures
		glBindTexture(GL_TEXTURE_2D, textures_handle);

		GLuint shader_program = shader_program_types[SPT_Cube_Face];
		if (!shader_program) {
			log("ERROR: Shader program not specified");
			assert(false);
		}
		glUseProgram(shader_program);

		MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;
		GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
		glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

		glDrawArrays(GL_TRIANGLES, 0, chunk_vbo.total_vertices);
	}
}

void generate_world_chunks(GL_Renderer* gl_renderer, float noise) {
	generate_height_map(noise);
	for (int x = 0; x < WORLD_SIZE_WIDTH; x++) {
		for (int y = 0; y < WORLD_SIZE_HEIGHT; y++) {
			for (int z = 0; z < WORLD_SIZE_LENGTH; z++) {
				generate_world_chunk(x, y, z, noise);
				generate_chunk_vbo(gl_renderer, { (float)x, (float)y, (float)z });
			}
		}
	}
}

void draw_chunk(GL_Renderer* gl_renderer, int chunk_size, int x_arr_pos, int y_arr_pos, int z_arr_pos) {
	int final_x_arr_pos = clamp(x_arr_pos, 0, WORLD_SIZE_WIDTH);
	int final_y_arr_pos = clamp(y_arr_pos, 0, WORLD_SIZE_HEIGHT);
	int final_z_arr_pos = clamp(z_arr_pos, 0, WORLD_SIZE_LENGTH);
	Chunk* chunk = &world_chunks[final_x_arr_pos][final_y_arr_pos][final_z_arr_pos];

	for (int x = 0; x < chunk_size; x++) {
		for (int y = 0; y < chunk_size; y++) {
			for (int z = 0; z < chunk_size; z++) {
				// Voxel index
				Cube* current_cube = &chunk->cubes[x][y][z];
				float world_space_x = x + (float)final_x_arr_pos * CHUNK_SIZE;
				float world_space_y = y + (float)final_y_arr_pos * CHUNK_SIZE;
				float world_space_z = z + (float)final_z_arr_pos * CHUNK_SIZE;

				V3 cube_ws_pos = { world_space_x, world_space_y, world_space_z };

				draw_cube_type(gl_renderer, cube_ws_pos, current_cube->type);
			}
		}
	}
}

void draw_chunks(GL_Renderer* gl_renderer) {
	for (int x = 0; x < WORLD_SIZE_WIDTH; x++) {
		for (int y = 0; y < WORLD_SIZE_HEIGHT; y++) {
			for (int z = 0; z < WORLD_SIZE_LENGTH; z++) {
				draw_chunk(gl_renderer, CHUNK_SIZE, x, y, z);
			}
		}
	}
}

void draw_wire_frames(GL_Renderer* gl_renderer){
	for (int x = 0; x < WORLD_SIZE_WIDTH; x++) {
		for (int y = 0; y < WORLD_SIZE_HEIGHT; y++) {
			for (int z = 0; z < WORLD_SIZE_LENGTH; z++) {
				// This is the corner position of the chunk. 
				// This is the position I'm drawing from.
				V3 chunk_ws_pos = {(float)x * CHUNK_SIZE, (float)y * CHUNK_SIZE, (float)z * CHUNK_SIZE};

				// The points draw in the center of the cube. Need to offset them.
				V3 p1 = chunk_ws_pos;

				V3 p2 = chunk_ws_pos;

				int w = CHUNK_SIZE;
				int l = CHUNK_SIZE;
				int h = CHUNK_SIZE;

				// Left face
				gl_draw_line(gl_renderer, { p1.x,	  p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z    }); 
				gl_draw_line(gl_renderer, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y + h, p2.z    }); 
				gl_draw_line(gl_renderer, { p1.x + w, p1.y + h, p1.z     }, { p2.x,     p2.y + h, p2.z    }); 
				gl_draw_line(gl_renderer, { p1.x,	  p1.y + h, p1.z     }, { p2.x,     p2.y,     p2.z    }); 
				// Right face
				gl_draw_line(gl_renderer, { p1.x,	  p1.y,     p1.z + l }, { p2.x + w, p2.y,     p2.z + l});
				gl_draw_line(gl_renderer, { p1.x + w, p1.y,     p1.z + l }, { p2.x + w, p2.y + h, p2.z + l}); 
				gl_draw_line(gl_renderer, { p1.x + w, p1.y + h, p1.z + l }, { p2.x,     p2.y + h, p2.z + l}); 
				gl_draw_line(gl_renderer, { p1.x,	  p1.y + h, p1.z + l }, { p2.x,     p2.y,     p2.z + l}); 
				// Connect the squares
				gl_draw_line(gl_renderer, { p1.x,	  p1.y,     p1.z     }, { p2.x,		p2.y,     p2.z + l});
				gl_draw_line(gl_renderer, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z + l}); 
				gl_draw_line(gl_renderer, { p1.x + w, p1.y + h, p1.z     }, { p2.x + w, p2.y + h, p2.z + l}); 
				gl_draw_line(gl_renderer, { p1.x,	  p1.y + h, p1.z     }, { p2.x,     p2.y + h, p2.z + l}); 
			}
		}
	}
}

LRESULT windowProcedure(HWND windowHandle, UINT messageType, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = {};
	switch (messageType) {
	case WM_KEYDOWN: {
		key_states[wParam].pressed_this_frame = true;
		key_states[wParam].held_down = true;
		log("WM_KEYDOWN: %llu", wParam);
	} break;
	case WM_KEYUP: {
		key_states[wParam].held_down = false;
	} break;
	case WM_MOUSEMOVE: {
		current_mouse_position.x = (float)GET_X_LPARAM(lParam);
		current_mouse_position.y = (float)GET_Y_LPARAM(lParam);
	} break;
	case WM_LBUTTONDOWN: {
	} break;
	case WM_CLOSE: {
		DestroyWindow(windowHandle);
	} break;
	case WM_DESTROY: {
		PostQuitMessage(0);
	} break;
	default: {
		result = DefWindowProc(windowHandle, messageType, wParam, lParam);
	} break;
	}

	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	OutputDebugString("Hello World!\n");

	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;

	WNDCLASS wndClass = {};
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc = windowProcedure;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = NULL;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wndClass.hbrBackground = NULL;
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = "MyClassName";

	RegisterClass(&wndClass);

	HWND window = CreateWindowEx(WS_EX_APPWINDOW, "MyClassName", "2D Project Chris", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL
	);

	ShowWindow(window, SW_SHOW);
	
	GL_Renderer* gl_renderer = create_gl_renderer(window);

	Image castle_infernal_image = create_Image(gl_renderer, "assets\\castle_Infernal.png");
	Image azir_image = create_Image(gl_renderer, "assets\\azir.jpg");

	Font font = load_font(gl_renderer, "assets\\font_1.png");

	init_images(gl_renderer);
	GLuint texture_sprite_sheet_handle = get_image(IT_Texture_Sprite_Sheet)->handle;

	generate_world_chunks(gl_renderer, 20);

	bool running = true;
	while (running) {
		reset_Pressed_This_Frame();
		last_mouse_position = current_mouse_position;
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
			
		}

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		// GL_COLOR_BUFFER_BIT: This clears the color buffer, which is responsible for holding the color 
		// information of the pixels. Clearing this buffer sets all the pixels to the color specified by glClearColor.
		// GL_DEPTH_BUFFER_BIT: This clears the depth buffer, which is responsible for holding the depth 
		// information of the pixels. The depth buffer keeps track of the distance from the camera to each pixel
		// to handle occlusion correctly.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		// **************Drawing 3D lines****************
		draw_wire_frames(gl_renderer);
		gl_upload_and_draw_lines_vbo(gl_renderer);
		// **********************************************

		// **************Drawing 3D Cubes****************
		// gl_upload_cube_vbo(gl_renderer);

		// draw_cube(gl_renderer, { 0,0,0 }, images[IT_Dirt].texture);
		// draw_chunks(gl_renderer);

		// gl_draw_cubes(gl_renderer);
		// ***********************************************

		// **************Generating Face VBO Chunks****************
		gl_draw_cube_faces_vbo(gl_renderer, texture_sprite_sheet_handle);
		// ***********************************************
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// **************Drawing Strings****************
		draw_string(gl_renderer, &font, "Testing", 500, 500, 5, true);
		gl_upload_and_draw_2d_string(gl_renderer, &font);
		// **********************************************

		gl_update_renderer(gl_renderer);
		SwapBuffers(gl_renderer->hdc);

		Sleep(1);
	}
	// TODO: Clean up shaders
	//      glDeleteShader(vertex_shader);
	//      glDeleteShader(fragment_shader);
	//      glDeleteProgram();
	//      glGetProgramInfoLog();
	//      glUseProgram();
	// TODO: Destroy the renderer
	return 0;
}
