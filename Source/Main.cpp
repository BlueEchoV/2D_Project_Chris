#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <errno.h>
#include <string.h>
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <assert.h>
#include <format>

#define STB_PERLIN_IMPLEMENTATION
#include "Perlin.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Utility.h"
#include "Math.h"
#include "gl_renderer.h"
#include "Jobs.h"

uint64_t start_time;
LARGE_INTEGER elapsed_global_time;
LARGE_INTEGER frequency_seconds;
void init_clock() {
	if (!QueryPerformanceCounter(&elapsed_global_time)) {
		log("Error: QueryPerformanceCounter() failed");
	}
	// For getting the seconds
	if (!QueryPerformanceFrequency(&frequency_seconds)) {
		log("Error: QueryPerformanceFrequency() failed");
	}
	start_time = elapsed_global_time.QuadPart;
}

// Returns the time since the program started
uint64_t get_clock_milliseconds() {
    if (!QueryPerformanceCounter(&elapsed_global_time)) {
		log("Error: QueryPerformanceCounter() failed");
	}
	return (elapsed_global_time.QuadPart - start_time) / (frequency_seconds.QuadPart / 1000);
}

float get_clock_seconds() {
	uint64_t time_milliseconds = get_clock_milliseconds();
	return (float)time_milliseconds / 1000.0f;
}

float get_clock_difference(float start_time_seconds) {
	uint64_t current_time_milliseconds = get_clock_milliseconds();
	float current_time_seconds = (float)current_time_milliseconds / 1000.0f;
	return current_time_seconds - start_time_seconds;
}

float profiling_time_start_seconds = 0.0f;
float delta_time = 0.0f;
void profile_time_to_execute_start_seconds() {
	profiling_time_start_seconds = get_clock_seconds();
}
void profile_time_to_execute_finish_seconds(std::string name) {
	float time = get_clock_seconds();
	time = time - profiling_time_start_seconds;
	std::string str = name + ": %f sec";
	log(str.c_str(), time);
}

uint64_t profiling_time_start_milliseconds = 0;
void profile_time_to_execute_start_milliseconds() {
	profiling_time_start_milliseconds = get_clock_milliseconds();
}
void profile_time_to_execute_finish_milliseconds(const std::string& name, bool include_less_than_one) {
	uint64_t time = get_clock_milliseconds();
	time = time - profiling_time_start_milliseconds;
	std::string str = name + ": ";
	// Weird condition but it's based on the result of get_clock_milliseconds which 
	// rounds the return value so we can just say it's "less than"
	if (time <= 1) {
		if (include_less_than_one) {
			str += "less than 1";
			log("Less than 1");
		}
		return;
	}
	str = "%i ms";
	log(str.c_str(), time);
}

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
	int chunk_x;
	int chunk_y;
	GLuint total_vertices;
	GLuint vbo;
	bool destroy = false;;
};

// IN CHUNKS
int VIEW_DISTANCE = 4;

struct Open_GL {
	GLuint vao;
	GLuint vbo_lines;
	GLuint vbo_cubes;
	GLuint vbo_fireballs;
	GLuint vbo_cube_faces;
	GLuint vbo_strings;
	GLuint vbo_quads;
};

struct Vertex_3D_Line {
	V3 pos;
	Color_f color;
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

enum Image_Type {
    IT_Air,
	IT_Cobblestone,
	IT_Dirt,
	IT_Grass,
	IT_Fireball,
	IT_Texture_Sprite_Sheet,

	IT_Total
};

struct Fireball {
	V3 pos;
	V3 velocity;
	MX4 mx_world;
	Image_Type type;
};

struct Fireball_To_Draw {
	V3 pos;
	V3 velocity;
	MX4 mx_world;
	Image_Type type;
};

struct Cube {
	Image_Type type = IT_Air;
};

const int CUBE_SIZE = 1;

const int CHUNK_WIDTH = 16;
const int CHUNK_LENGTH = 16;
const int CHUNK_HEIGHT = 256;

struct Chunk {
	bool allocated = false;
	bool buffer_sub_data = false;

	int chunk_x;
	int chunk_y;
	GLuint total_vertices;
	GLuint vbo;
	GLsizeiptr buffer_size;
	GLuint total_indices_to_be_rendered;
	GLuint ebo;

	Cube cubes[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT] = {};
};

struct Vertex_3D_Faces {
	V3 pos;
	V2 uv;
	V3 normal;
};

struct Vertex_3D {
	V3 pos;
	V2 uv;
};

struct GL_Renderer {
	HWND window;
	HDC hdc;

	Open_GL open_gl = {};
	std::vector<Vertex_3D_Line> lines_vertices;
	std::vector<Cube_Draw> cubes;
	std::vector<Vertex_String> strings;
	std::vector<Chunk*> chunks_to_draw;
	std::vector<Vertex_3D> quads_to_draw;

	float time;
	float player_speed;
	V3 player_pos = { 0, 0, 0 };
	int previous_chunk_player_x = 0;
	int previous_chunk_player_y = 0;

	float fireball_speed;
	std::vector<Fireball> fireballs;

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

	init_clock();

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

void shoot_fireball(GL_Renderer* gl_renderer, V3 pos, V3 velocity, MX4 world, Image_Type type) {
	Fireball result;

	result.pos = pos;
	result.velocity = velocity;
	result.mx_world = world;
	result.type = type;

	gl_renderer->fireballs.push_back(result);
}

bool toggle_debug_info = false;
bool force_regenerate = true;
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
	}
	else if (gl_renderer->pitch < -89.0f) {
		gl_renderer->pitch = -89.0f;
	}

	// Calculate the forward vector
	MX4 transposed_view_mx = matrix_transpose(gl_renderer->view_from_world);

	// OpenGL's default coordinate system

	// The third column of the transposed view matrix col[2] corresponds to the 
	// forward direction of the camera. The forward direction is typically the 
	// negative z-axis. By negating this column, you get the actual forward direction 
	// in the camera's coordinate system.
	V3 forward = transposed_view_mx.col[0].xyz;
	V3 right = transposed_view_mx.col[1].xyz;
	V3 up = transposed_view_mx.col[2].xyz;

	// TODO: There is a bug with holding down the keys simultaneously and 
	// the player moving faster diagonally. 
	if (key_states[VK_SHIFT].pressed_this_frame || key_states[VK_SHIFT].held_down) {
		gl_renderer->player_speed = 2.00f;
	}
	else {
		gl_renderer->player_speed = 1.00f;
	}
	if (key_states[VK_S].pressed_this_frame || key_states[VK_S].held_down) {
		gl_renderer->player_pos.x -= forward.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y -= forward.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z -= forward.z * gl_renderer->player_speed;
	}
	if (key_states[VK_W].pressed_this_frame || key_states[VK_W].held_down) {
		gl_renderer->player_pos.x += forward.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y += forward.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z += forward.z * gl_renderer->player_speed;
	}
	if (key_states[VK_D].pressed_this_frame || key_states[VK_D].held_down) {
		gl_renderer->player_pos.x += right.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y += right.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z += right.z * gl_renderer->player_speed;
	}
	if (key_states[VK_A].pressed_this_frame || key_states[VK_A].held_down) {
		gl_renderer->player_pos.x -= right.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y -= right.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z -= right.z * gl_renderer->player_speed;
	}
	if (key_states[VK_SPACE].pressed_this_frame || key_states[VK_SPACE].held_down) {
		gl_renderer->player_pos.x += up.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y += up.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z += up.z * gl_renderer->player_speed;
	}
	if (key_states[VK_CONTROL].pressed_this_frame || key_states[VK_CONTROL].held_down) {
		gl_renderer->player_pos.x -= up.x * gl_renderer->player_speed;
		gl_renderer->player_pos.y -= up.y * gl_renderer->player_speed;
		gl_renderer->player_pos.z -= up.z * gl_renderer->player_speed;
	}
	if (key_states[VK_LBUTTON].pressed_this_frame) {
		MX4 matrix = identity_mx_4();
		matrix.col[0].xyz = forward;
		matrix.col[1].xyz = right;
		matrix.col[2].xyz = up;
		shoot_fireball(gl_renderer, gl_renderer->player_pos, forward, matrix, IT_Fireball);
		log("Shooting fireball");
	}
	if (key_states[VK_UP].pressed_this_frame) {
		VIEW_DISTANCE++;
		force_regenerate = true;
		log("%i", VIEW_DISTANCE);
	}
	if (key_states[VK_DOWN].pressed_this_frame) {
		VIEW_DISTANCE--;
		force_regenerate = true;
		log("%i", VIEW_DISTANCE);
	}
	if (key_states[VK_TAB].pressed_this_frame) {
		toggle_debug_info = !toggle_debug_info;
	}

	int window_width = 0;
	int window_height = 0;
	get_window_size(gl_renderer->window, window_width, window_height);
	glViewport(0, 0, window_width, window_height);

	// The perspective is gotten from the frustum
	gl_renderer->perspective_from_view = mat4_perspective((float)M_PI / 2.0f, (float)window_width / (float)window_height);

	// This is my camera
	// Move the camera by the same amount of the player but do the negation
	gl_renderer->view_from_world = mat4_rotate_y(-gl_renderer->pitch) * mat4_rotate_z(-gl_renderer->yaw) * translation_matrix_mx_4(
		-gl_renderer->player_pos.x, 
		-gl_renderer->player_pos.y, 
		-gl_renderer->player_pos.z
	);
}

void gl_draw_line(GL_Renderer* gl_renderer, Color_f color, V3 p1, V3 p2) {
	if (gl_renderer == nullptr) {
		log("Error: gl_renderer is nullptr");
		assert(false);
		return;
	}

	Vertex_3D_Line vertices[2] = {};
	// World positions
	vertices[0].pos = p1;
	vertices[0].color = color;
	gl_renderer->lines_vertices.push_back(vertices[0]);
	vertices[1].pos = p2;
	vertices[1].color = color;
	gl_renderer->lines_vertices.push_back(vertices[1]);
}

void draw_cube_with_lines(GL_Renderer* gl_renderer, int w, int l, int h, V3 pos) {
    // Front face
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y, pos.z }, { pos.x + w, pos.y, pos.z });				 // Bottom edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y, pos.z }, { pos.x + w, pos.y + h, pos.z });		 // Right edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y + h, pos.z }, { pos.x, pos.y + h, pos.z });		 // Top edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y + h, pos.z }, { pos.x, pos.y, pos.z });				 // Left edge

    // Back face
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y, pos.z + l }, { pos.x + w, pos.y, pos.z + l });		 // Bottom edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y, pos.z + l }, { pos.x + w, pos.y + h, pos.z + l }); // Right edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y + h, pos.z + l }, { pos.x, pos.y + h, pos.z + l }); // Top edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y + h, pos.z + l }, { pos.x, pos.y, pos.z + l });		 // Left edge

    // Connecting edges
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y, pos.z }, { pos.x, pos.y, pos.z + l });				 // Bottom left edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y, pos.z }, { pos.x + w, pos.y, pos.z + l });		 // Bottom right edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x + w, pos.y + h, pos.z }, { pos.x + w, pos.y + h, pos.z + l }); // Top right edge
    gl_draw_line(gl_renderer,{ 1, 0, 0, 1 }, { pos.x, pos.y + h, pos.z }, { pos.x, pos.y + h, pos.z + l });		 // Top left edge
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
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Line), (void*)offsetof(Vertex_3D_Line, color));
	glEnableVertexAttribArray(1);

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
	images[IT_Fireball] = create_Image(gl_renderer, "assets\\fireball.png");
}

#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
GLuint cube_map_texture_handle = 0;
void create_cube_map() {
	const char* image_back_file_path = "assets\\skybox\\space\\back.png";
	int back_w, back_h, back_channels;
	unsigned char* back_data = stbi_load(image_back_file_path, &back_w, &back_h, &back_channels, 4);
	if (back_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}

	const char* image_bottom_file_path = "assets\\skybox\\space\\bottom.png";
	int bottom_w, bottom_h, bottom_channels;
	unsigned char* bottom_data = stbi_load(image_bottom_file_path, &bottom_w, &bottom_h, &bottom_channels, 4);
	if (bottom_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}
	DEFER{
		stbi_image_free(bottom_data);
	};

	const char* image_front_file_path = "assets\\skybox\\space\\front.png";
	int front_w, front_h, front_channels;
	unsigned char* front_data = stbi_load(image_front_file_path, &front_w, &front_h, &front_channels, 4);
	if (front_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}
	DEFER{
		stbi_image_free(front_data);
	};

	const char* image_left_file_path = "assets\\skybox\\space\\left.png";
	int left_w, left_h, left_channels;
	unsigned char* left_data = stbi_load(image_left_file_path, &left_w, &left_h, &left_channels, 4);
	if (left_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}
	DEFER{
		stbi_image_free(left_data);
	};

	const char* image_right_file_path = "assets\\skybox\\space\\right.png";
	int right_w, right_h, right_channels;
	unsigned char* right_data = stbi_load(image_right_file_path, &right_w, &right_h, &right_channels, 4);
	if (right_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}
	DEFER{
		stbi_image_free(right_data);
	};

	const char* image_top_file_path = "assets\\skybox\\space\\top.png";
	int top_w, top_h, top_channels;
	unsigned char* top_data = stbi_load(image_top_file_path, &top_w, &top_h, &top_channels, 4);
	if (top_data == NULL) {
		log("ERROR: stbi_load returned NULL");
		return;
	}
	DEFER{
		stbi_image_free(top_data);
	};

	glGenTextures(1, &cube_map_texture_handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_texture_handle);

	int width = back_w;
	int height = back_h;
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, back_data);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bottom_data);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, front_data);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, left_data);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, right_data);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, top_data);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
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
	case IT_Fireball: {
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
	result.char_h = height / 7;
	
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

V2 convert_to_uv_coordinates(V2 pos, float w, float h) {
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

    V2 top_left_src =	  { (float)src.x,           (float)src.y };
    V2 top_right_src =	  { (float)(src.x + src.w), (float)src.y };
    V2 bottom_right_src = { (float)(src.x + src.w), (float)(src.y + src.h) };
    V2 bottom_left_src =  { (float)src.x,           (float)src.y + src.h };

    V2 top_left_uv =     convert_to_uv_coordinates(top_left_src,     (float)font->texture->w, (float)font->texture->h);
    V2 top_right_uv =    convert_to_uv_coordinates(top_right_src,    (float)font->texture->w, (float)font->texture->h);
    V2 bottom_right_uv = convert_to_uv_coordinates(bottom_right_src, (float)font->texture->w, (float)font->texture->h);
    V2 bottom_left_uv =  convert_to_uv_coordinates(bottom_left_src,  (float)font->texture->w, (float)font->texture->h);

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

V3 get_camera_forward(GL_Renderer* gl_renderer) {
	MX4 transpose_view_mx = matrix_transpose(gl_renderer->view_from_world);
	// Move the camera in the opposite direction
	V3 camera_forward = transpose_view_mx.col[0].xyz;

	return camera_forward;
}

void draw_string_ws(GL_Renderer* gl_renderer, Font* font, const char* str, V3 ws_pos, int size, bool center) {
	V4 ws_pos_converted = { ws_pos.x, ws_pos.y, ws_pos.z, 1.0f };
	// WS position
	V4 ndc = gl_renderer->perspective_from_view * gl_renderer->view_from_world * ws_pos_converted;

	V3 camera_forward = get_camera_forward(gl_renderer);

	V3 camera_pos = gl_renderer->player_pos;
	V3 camera_to_string;
	camera_to_string.x = ws_pos.x - camera_pos.x;
	camera_to_string.y = ws_pos.y - camera_pos.y;
	camera_to_string.z = ws_pos.z - camera_pos.z;

	float dot_product_result = dot_product(camera_forward, camera_to_string);

	if (dot_product_result < 0) {
		return;
	}

	// Perspective divide
	float x = ndc.x / ndc.w;
	float y = ndc.y / ndc.w;

	int w, h;
	get_window_size(gl_renderer->window, w, h);

	// Convert to a range of 0 - 2 from -1 - 1
	x += 1.0f;
	y += 1.0f;

	// 0 - 1
	x /= 2.0f;
	y /= 2.0f;

	float pixel_pos_x = lerp(0, (float)w, x);
	float pixel_pos_y = lerp(0, (float)h, 1 - y);

	draw_string(gl_renderer, font, str, (int)pixel_pos_x, (int)pixel_pos_y, size, center);
}

void gl_upload_and_draw_2d_string(GL_Renderer* gl_renderer, Font* font) {
	if (gl_renderer->open_gl.vbo_strings == 0) {
		glDeleteBuffers(1, &gl_renderer->open_gl.vbo_strings);
	}

	glGenBuffers(1, &gl_renderer->open_gl.vbo_strings);
	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_strings);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_String) * gl_renderer->strings.size(), gl_renderer->strings.data(), GL_STATIC_DRAW);

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

void draw_cube_map(GL_Renderer* gl_renderer) {
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

	glBindTexture(GL_TEXTURE_2D, cube_map_texture_handle);

	GLuint shader_program = shader_program_types[SPT_Cube_Map];
	if (!shader_program) {
		log("ERROR: Shader program not specified");
		assert(false);
	}
	glUseProgram(shader_program);

	// NOTE: This is bad because if I change the way the view matrix is created,
	// this code breaks.
	// Get the view matrix rotation and not the position change
	MX4 view_rotation = mat4_rotate_y(-gl_renderer->pitch) * mat4_rotate_z(-gl_renderer->yaw);

	MX4 perspective_from_view = gl_renderer->perspective_from_view * view_rotation;
	GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_view");
	glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_view.e);

	glDrawArrays(GL_TRIANGLES, 0, ARRAYSIZE(cube));
}

void draw_cube_mx(GL_Renderer* gl_renderer, MX4 mx) {
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

	GLuint handle = get_image(IT_Dirt)->handle;
	glBindTexture(GL_TEXTURE_2D, handle);

	GLuint shader_program = shader_program_types[SPT_3D];
	if (!shader_program) {
		log("ERROR: Shader program not specified");
		assert(false);
	}
	glUseProgram(shader_program);

	MX4 world_from_model = mx;
	GLuint world_from_model_loc = glGetUniformLocation(shader_program, "world_from_model");
	glUniformMatrix4fv(world_from_model_loc, 1, GL_FALSE, world_from_model.e);

	MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;
	GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
	glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

	GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
	glUniform1f(time_loc, gl_renderer->time);

	glDrawArrays(GL_TRIANGLES, 0, ARRAYSIZE(cube));
}

void draw_cube(GL_Renderer* gl_renderer, V3 pos, GL_Texture* texture) {
	Cube_Draw result;

	result.pos_ws = pos;
	result.texture_handle = texture->handle;

	gl_renderer->cubes.push_back(result);
}

void gl_draw_cubes(GL_Renderer* gl_renderer) {
	if (gl_renderer->open_gl.vbo_fireballs == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_fireballs);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_fireballs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_fireballs);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, normal));
	glEnableVertexAttribArray(3);


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

void gl_draw_fireballs(GL_Renderer* gl_renderer) {
	std::vector<Fireball_To_Draw> fireballs_to_draw;
	for (Fireball& fireball : gl_renderer->fireballs) {
		Fireball_To_Draw result;

		result.pos = fireball.pos;
		result.velocity = fireball.velocity;
		result.mx_world = fireball.mx_world;
		result.type = fireball.type;

		fireballs_to_draw.push_back(result);
	}

	if (gl_renderer->open_gl.vbo_fireballs == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_fireballs);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_fireballs);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_fireballs);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Cube), (void*)offsetof(Vertex_3D_Cube, normal));
	glEnableVertexAttribArray(3);

	std::vector<MX4> matrices;
	for (Fireball_To_Draw& fireball_to_draw : fireballs_to_draw) {
		GLuint texture = get_image(fireball_to_draw.type)->handle;
		glBindTexture(GL_TEXTURE_2D, texture);

		GLuint shader_program = shader_program_types[SPT_Fireball];
		if (!shader_program) {
			log("ERROR: Shader program not specified");
			assert(false);
		}
		glUseProgram(shader_program);

		// MX4 world_from_model = translation_matrix_mx_4(cos(x) * x_speed, sin(x) * y_speed, z_pos);
		MX4 world_from_model = translation_matrix_mx_4(
			fireball_to_draw.pos.x,
			fireball_to_draw.pos.y,
			fireball_to_draw.pos.z) * fireball_to_draw.mx_world/* * mat4_rotate_x(renderer->time)*/;

		MX4 child_mx_one = world_from_model * mat4_rotate_z(gl_renderer->time * 2) * translation_matrix_mx_4(0, 2, 0) * scaling_matrix_mx_4(0.5f, 0.5f, 0.5f);
		MX4 child_mx_two = world_from_model * mat4_rotate_z(gl_renderer->time * 2.0f + (float)M_PI / 2.0f) * translation_matrix_mx_4(0, 2, 0) * scaling_matrix_mx_4(0.5f, 0.5f, 0.5f);
		MX4 child_mx_three = world_from_model * mat4_rotate_z(gl_renderer->time * 2.0f + (float)M_PI) * translation_matrix_mx_4(0, 2, 0) * scaling_matrix_mx_4(0.5f, 0.5f, 0.5f);
		MX4 child_mx_four = world_from_model * mat4_rotate_z(gl_renderer->time * 2.0f + (3.0f * (float)M_PI) / 2.0f) * translation_matrix_mx_4(0, 2, 0) * scaling_matrix_mx_4(0.5f, 0.5f, 0.5f);

		matrices.push_back(child_mx_one);
		matrices.push_back(child_mx_two);
		matrices.push_back(child_mx_three);
		matrices.push_back(child_mx_four);
		
		GLuint world_from_model_loc = glGetUniformLocation(shader_program, "world_from_model");
		glUniformMatrix4fv(world_from_model_loc, 1, GL_FALSE, world_from_model.e);

		MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;
		GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
		glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

		glDrawArrays(GL_TRIANGLES, 0, ARRAYSIZE(cube));
	}
	// No need to clear the vector because it's within the scope of this function
	for (MX4 matrix : matrices) {
		draw_cube_mx(gl_renderer, matrix);
	}
}

void draw_cube_type(GL_Renderer* gl_renderer, V3 pos, Image_Type type) {
	GL_Texture* result = get_image(type);

	if (result == nullptr) {
		log("Error: GL_Texture* is nullptr");
		// assert(false);
		return;
	} 
	draw_cube(gl_renderer, pos, result);
}

// Order of the sprite sheet = Grass | Dirt | Cobblestone
V2 get_texture_sprite_sheet_uv_coordinates(Image_Type type, int pos_x, int pos_y) {
	V2 result = {};
	// Currently only have one row
	result.y = (float)pos_y;

	switch (type) {
	case IT_Grass: {
		if (pos_x == 0) {
			result.x = 0.0f;
		} else if (pos_x == 1) {
			result.x = 0.125f;
		}
		if (pos_y == 0) {
			result.y = 0.0f;
		} else if (pos_y == 1) {
			result.y = 0.125f;
		}
		break;
	}
	case IT_Dirt: {
		if (pos_x == 0) {
			result.x = 0.125f;
		} else if (pos_x == 1) {
			result.x = 0.25f;
		}
		if (pos_y == 0) {
			result.y = 0.0f;
		} else if (pos_y == 1) {
			result.y = 0.125f;
		}
		break;
	}
	case IT_Cobblestone: {
		if (pos_x == 0) {
			result.x = 0.25;
		} else if (pos_x == 1) {
			result.x = 0.375;
		}
		if (pos_y == 0) {
			result.y = 0.0f;
		} else if (pos_y == 1) {
			result.y = 0.125f;
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

void generate_world_chunk(GL_Renderer* gl_renderer, int chunk_world_index_x, int chunk_world_index_y, float noise) {
	Chunk* new_chunk = nullptr;

	// See if there is a available spot in the chunks_to_draw vector
	for (Chunk* chunk : gl_renderer->chunks_to_draw) {
		if (chunk->allocated == false) {
			new_chunk = chunk;
			new_chunk->buffer_sub_data = true;
			break;
		}
	}
	// Create the chunk of the spot doesn't exist
	if (new_chunk == nullptr) {
		new_chunk = new Chunk();
		new_chunk->buffer_sub_data = false;
		gl_renderer->chunks_to_draw.push_back(new_chunk);
	}

	new_chunk->allocated = true;

	new_chunk->chunk_x = chunk_world_index_x;
	new_chunk->chunk_y = chunk_world_index_y;
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_LENGTH; y++) {
			float world_space_x = x + (float)new_chunk->chunk_x * CHUNK_WIDTH;
			float world_space_y = y + (float)new_chunk->chunk_y * CHUNK_LENGTH;
			float height_value = stb_perlin_noise3((float)world_space_x / (float)noise, 0, world_space_y / noise, 0, 0, 0);
			// The perlin value is between -1 and 1, and this line normalizes it to a 
			// range of 0 to CHUNK_HEIGHT, determining the height of the terrain
			// at that (x, y) position.
			float max_height_range = 30.0f;
			int column_height = (int)((height_value + 1.0f) * 0.5f * max_height_range);
			column_height += (CHUNK_HEIGHT - (int)max_height_range);

			for (int z = 0; z < CHUNK_HEIGHT; z++) {
				float world_space_z = (float)z;
				float perlin_result = stb_perlin_noise3(
					(float)world_space_x / (float)noise, 
					(float)world_space_y / (float)noise,
					(float)world_space_z / (float)noise, 
					0, 0, 0 // wrapping
				);

				Image_Type result = IT_Air;

				// We know this is the top chunk
				if (world_space_z < column_height) {
					if (world_space_z == column_height - 1) {
						result = IT_Grass;
					} else if (world_space_z < column_height - 1 && world_space_z >= column_height - 4) {
						result = IT_Dirt;
					} else if (world_space_z < column_height - 4 && world_space_z >= column_height - 8) {
						result = IT_Cobblestone;
					} else {
						if (perlin_result > -0.3) {
							result = IT_Cobblestone;
						} else {
							result = IT_Air;
						}
					}
				} else {
					result = IT_Air;
				}

				new_chunk->cubes[x][y][z].type = result;
			}
		}
	}

	new_chunk->total_vertices = 0;
}

Chunk* get_existing_chunk(GL_Renderer* gl_renderer, int x, int y) {
	Chunk* result = nullptr;
	for (Chunk* chunk : gl_renderer->chunks_to_draw) {
		if (x == chunk->chunk_x && y == chunk->chunk_y) {
			result = chunk;
		}
	}

	return result;
}

void buffer_world_chunk(GL_Renderer* gl_renderer, int chunk_world_index_x, int chunk_world_index_y) {
	Chunk* chunk = get_existing_chunk(gl_renderer, chunk_world_index_x, chunk_world_index_y);
	if (chunk == nullptr) {
		return;
	}

	// Generate the data to be sent to the GPU
	std::vector<Vertex_3D_Faces> faces_vertices = {};
	std::vector<UINT32> faces_indices = {};
	V3 chunk_ws_pos = { chunk->chunk_x * (float)CHUNK_WIDTH, chunk->chunk_y * (float)CHUNK_LENGTH, 0 };
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_LENGTH; y++) {
			for (int z = 0; z < CHUNK_HEIGHT; z++) {
				// Grab the cube I want
				V3 cube_ws_pos = chunk_ws_pos;
				cube_ws_pos.x += x;
				cube_ws_pos.y += y;

				cube_ws_pos.z = (float)z;

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
					if (x + 1 < CHUNK_WIDTH) {
						front_cube = chunk->cubes[x + 1][y][z];
					} 
					if (y + 1 < CHUNK_LENGTH) {
						right_cube = chunk->cubes[x][y + 1][z];
					}
					if (z + 1 < CHUNK_HEIGHT) {
						top_cube = chunk->cubes[x][y][z + 1];
					}
					if (x > 0) {
						back_cube = chunk->cubes[x - 1][y][z];
					}
					if (y > 0) {
						left_cube = chunk->cubes[x][y - 1][z];
					}
					if (z > 0) {
						bottom_cube = chunk->cubes[x][y][z - 1];
					}

					// Check if the index is one out of bounds for adjacent chunks
					// NOTE: Ignore the height because I'm not stacking chunks
					if (x + 1 == CHUNK_WIDTH) {
						Chunk* adjacent_chunk = get_existing_chunk(gl_renderer, chunk_world_index_x + 1, chunk_world_index_y);

						// Grab the adjacent cube
						if (adjacent_chunk != nullptr) {
							front_cube = adjacent_chunk->cubes[0][y][z];
						}
					}
					if (y + 1 == CHUNK_LENGTH) {
						Chunk* adjacent_chunk = get_existing_chunk(gl_renderer, chunk_world_index_x, chunk_world_index_y + 1);

						// Grab the adjacent cube
						if (adjacent_chunk != nullptr) {
							right_cube = adjacent_chunk->cubes[x][0][z];
						}
					}
					if (x == 0) {
						Chunk* adjacent_chunk = get_existing_chunk(gl_renderer, chunk_world_index_x - 1, chunk_world_index_y);

						// Grab the adjacent cube
						if (adjacent_chunk != nullptr) {
							back_cube = adjacent_chunk->cubes[CHUNK_WIDTH - 1][y][z];
						}
					}
					if (y == 0) {
						Chunk* adjacent_chunk = get_existing_chunk(gl_renderer, chunk_world_index_x, chunk_world_index_y - 1);

						// Grab the adjacent cube
						if (adjacent_chunk != nullptr) {
							left_cube = adjacent_chunk->cubes[x][CHUNK_LENGTH - 1][z];
						}
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

						face.p2 = cube_ws_pos;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { -1, 0, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						// No need to store starting index becuase I'm going to be doing a massive 
						// glDrawElements call
						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);

						// Second Triangle
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 0);
					}

					if (top_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.z += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.y += CUBE_SIZE;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 0, 0, 1 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (right_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.y += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.y += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 0, 1, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (front_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.x += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 1, 0, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 3);

						// Second Triangle
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 0);
					}

					if (bottom_cube.type == IT_Air) {
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

						vertex.normal = { 0, 0, -1 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (left_cube.type == IT_Air) {
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

						vertex.normal = { 0, -1, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 0);
					}
				}
			}
		}
	}

	chunk->total_indices_to_be_rendered = (GLuint)faces_indices.size();

	bool force_reallocate = false;
	if (chunk->buffer_sub_data) {
		GLsizeiptr new_size = sizeof(Vertex_3D_Faces) * faces_vertices.size();
		// ONLY ALLOCATE IF THERE IS ENOUGH SIZE IN THE CURRENT BUFFER
		if (new_size <= chunk->buffer_size && chunk->vbo > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
			//											  size of the data that is being replaced
			glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, new_size, faces_vertices.data());

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)0, faces_indices.size() * sizeof(UINT32), faces_indices.data());
		}
		else {
			force_reallocate = true;
		}
	}
	if (!chunk->buffer_sub_data || force_reallocate) {
		bool chunk_already_in_chunks_to_draw = true;
		if (chunk->vbo <= 0) {
			glGenBuffers(1, &chunk->vbo);
			chunk_already_in_chunks_to_draw = false;
		}
		glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
		chunk->buffer_size = sizeof(Vertex_3D_Faces) * faces_vertices.size();
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_3D_Faces) * faces_vertices.size(), faces_vertices.data(), GL_STATIC_DRAW);


		if (chunk->ebo <= 0) {
			glGenBuffers(1, &chunk->ebo);
		}
		// Elements buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_indices.size() * sizeof(UINT32), faces_indices.data(), GL_STATIC_DRAW);

		if (!chunk_already_in_chunks_to_draw) {
			gl_renderer->chunks_to_draw.push_back(chunk);
		}
	}
	faces_vertices.clear();
}

#if 0
void generate_world_chunk(GL_Renderer* gl_renderer, int chunk_world_index_x, int chunk_world_index_y, float noise) {
	Chunk* new_chunk = {};
	// See if we can buffer_sub_data
	bool buffer_sub_data = false;
	for (Chunk* chunk : gl_renderer->chunks_to_draw) {
		// Check if there is a open spot
		if (chunk->allocated == false) {
			new_chunk = chunk;
			buffer_sub_data = true;
			break;
		}
	}
	// This means there are no available spots spots 
	if (new_chunk == nullptr) {
		new_chunk = new Chunk();
	}

	new_chunk->allocated = true;

	new_chunk->chunk_x = chunk_world_index_x;
	new_chunk->chunk_y = chunk_world_index_y;
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_LENGTH; y++) {
			float world_space_x = x + (float)new_chunk->chunk_x * CHUNK_WIDTH;
			float world_space_y = y + (float)new_chunk->chunk_y * CHUNK_LENGTH;
			float height_value = stb_perlin_noise3((float)world_space_x / (float)noise, 0, world_space_y / noise, 0, 0, 0);
			// The perlin value is between -1 and 1, and this line normalizes it to a 
			// range of 0 to CHUNK_HEIGHT, determining the height of the terrain
			// at that (x, y) position.
			float max_height_range = 30.0f;
			int column_height = (int)((height_value + 1.0f) * 0.5f * max_height_range);
			column_height += (CHUNK_HEIGHT - (int)max_height_range);

			for (int z = 0; z < CHUNK_HEIGHT; z++) {
				float world_space_z = (float)z;
				float perlin_result = stb_perlin_noise3(
					(float)world_space_x / (float)noise, 
					(float)world_space_y / (float)noise,
					(float)world_space_z / (float)noise, 
					0, 0, 0 // wrapping
				);

				Image_Type result = IT_Air;

				// We know this is the top chunk
				if (world_space_z < column_height) {
					if (world_space_z == column_height - 1) {
						result = IT_Grass;
					} else if (world_space_z < column_height - 1 && world_space_z >= column_height - 4) {
						result = IT_Dirt;
					} else if (world_space_z < column_height - 4 && world_space_z >= column_height - 8) {
						result = IT_Cobblestone;
					} else {
						if (perlin_result > -0.3) {
							result = IT_Cobblestone;
						} else {
							result = IT_Air;
						}
					}
				} else {
					result = IT_Air;
				}

				new_chunk->cubes[x][y][z].type = result;
			}
		}
	}

	new_chunk->total_vertices = 0;
	// Generate the data to be sent to the GPU
	std::vector<Vertex_3D_Faces> faces_vertices = {};
	std::vector<UINT32> faces_indices = {};
	V3 chunk_ws_pos = { new_chunk->chunk_x * (float)CHUNK_WIDTH, new_chunk->chunk_y * (float)CHUNK_LENGTH, 0 };
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_LENGTH; y++) {
			for (int z = 0; z < CHUNK_HEIGHT; z++) {
				// Grab the cube I want
				V3 cube_ws_pos = chunk_ws_pos;
				cube_ws_pos.x += x;
				cube_ws_pos.y += y;

				cube_ws_pos.z = (float)z;

				Cube* current_cube = &new_chunk->cubes[x][y][z];
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
					if (x + 1 < CHUNK_WIDTH) {
						front_cube = new_chunk->cubes[x + 1][y][z];
					} else if (x + 1 == CHUNK_WIDTH) {

					}
					if (y + 1 < CHUNK_LENGTH) {
						right_cube = new_chunk->cubes[x][y + 1][z];
					}
					if (z + 1 < CHUNK_HEIGHT) {
						top_cube = new_chunk->cubes[x][y][z + 1];
					}
					if (x > 0) {
						back_cube = new_chunk->cubes[x - 1][y][z];
					}
					if (y > 0) {
						left_cube = new_chunk->cubes[x][y - 1][z];
					}
					if (z > 0) {
						bottom_cube = new_chunk->cubes[x][y][z - 1];
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

						face.p2 = cube_ws_pos;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { -1, 0, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						// No need to store starting index becuase I'm going to be doing a massive 
						// glDrawElements call
						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);

						// Second Triangle
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 0);
					}

					if (top_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.z += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.y += CUBE_SIZE;
						face.p4.z += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 0, 0, 1 };

						// Square (Indicies)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (right_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.y += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.y += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 0, 1, 0 };

						// Square (Indicies)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (front_cube.type == IT_Air) {
						Cube_Face face = {};

						face.p1 = cube_ws_pos;
						face.p1.x += CUBE_SIZE;

						face.p2 = cube_ws_pos;
						face.p2.x += CUBE_SIZE;
						face.p2.z += CUBE_SIZE;

						face.p3 = cube_ws_pos;
						face.p3.x += CUBE_SIZE;
						face.p3.y += CUBE_SIZE;
						face.p3.z += CUBE_SIZE;

						face.p4 = cube_ws_pos;
						face.p4.x += CUBE_SIZE;
						face.p4.y += CUBE_SIZE;

						Vertex_3D_Faces vertex = {};

						vertex.normal = { 1, 0, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 3);

						// Second Triangle
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 0);
					}

					if (bottom_cube.type == IT_Air) {
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

						vertex.normal = { 0, 0, -1 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 0);
					}

					if (left_cube.type == IT_Air) {
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

						vertex.normal = { 0, -1, 0 };

						// Square (Indices)
						vertex.pos = face.p1;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 0);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p2;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 0, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p3;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 1);
						faces_vertices.push_back(vertex);

						vertex.pos = face.p4;
						vertex.uv = get_texture_sprite_sheet_uv_coordinates(t, 1, 0);
						faces_vertices.push_back(vertex);

						new_chunk->total_vertices += 4;

						UINT32 base_index = (UINT32)(faces_vertices.size()) - 4;
						// First Triangle
						faces_indices.push_back(base_index + 0);
						faces_indices.push_back(base_index + 1);
						faces_indices.push_back(base_index + 2);

						// Second Triangle
						faces_indices.push_back(base_index + 2);
						faces_indices.push_back(base_index + 3);
						faces_indices.push_back(base_index + 0);
					}
				}
			}
		}
	}

	new_chunk->total_indices_to_be_rendered = (GLuint)faces_indices.size();

	bool force_reallocate = false;
	if (buffer_sub_data) {
		GLsizeiptr new_size = sizeof(Vertex_3D_Faces) * faces_vertices.size();
		// ONLY ALLOCATE IF THERE IS ENOUGH SIZE IN THE CURRENT BUFFER
		if (new_size <= new_chunk->buffer_size && new_chunk->vbo > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, new_chunk->vbo);
			//											  size of the data that is being replaced
			glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, new_size, faces_vertices.data());

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_chunk->ebo);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)0, faces_indices.size() * sizeof(UINT32), faces_indices.data());
		}
		else {
			force_reallocate = true;
		}
	}
	if (!buffer_sub_data || force_reallocate) {
		bool chunk_already_in_chunks_to_draw = true;
		if (new_chunk->vbo <= 0) {
			glGenBuffers(1, &new_chunk->vbo);
			chunk_already_in_chunks_to_draw = false;
		}
		glBindBuffer(GL_ARRAY_BUFFER, new_chunk->vbo);
		new_chunk->buffer_size = sizeof(Vertex_3D_Faces) * faces_vertices.size();
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_3D_Faces) * faces_vertices.size(), faces_vertices.data(), GL_STATIC_DRAW);


		if (new_chunk->ebo <= 0) {
			glGenBuffers(1, &new_chunk->ebo);
		}
		// Elements buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_chunk->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_indices.size() * sizeof(UINT32), faces_indices.data(), GL_STATIC_DRAW);

		if (!chunk_already_in_chunks_to_draw) {
			gl_renderer->chunks_to_draw.push_back(new_chunk);
		}
	}
	faces_vertices.clear();
}
#endif

void gl_draw_cube_faces_vbo(GL_Renderer* gl_renderer, GLuint textures_handle) {
	// V3 light_position = {};
	// if (gl_renderer->fireballs.size() > 0) {
	// 	light_position = gl_renderer->fireballs[0].pos;
	// }

	// Bind the sprite sheet with all the textures
	// log("chunks_to_draw_size = %i", gl_renderer->chunks_to_draw.size());
	for (Chunk* chunk: gl_renderer->chunks_to_draw) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
		if (chunk->ebo <= 0) {
			assert(false);
		}
		glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, normal));
		glEnableVertexAttribArray(2);

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

		GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
		glUniform1f(time_loc, gl_renderer->time);

		// GLuint light_fireball_loc = glGetUniformLocation(shader_program, "light_position_fireball");
		// glUniform3fv(time_loc, );

		glDrawElements(GL_TRIANGLES, chunk->total_indices_to_be_rendered, GL_UNSIGNED_INT, (void*)0);
	}
}

#if 0
Vertex_3D quad[4] = {
//   V3  UV
	{{}, {}},
};

void gl_draw_quad(GL_Renderer* gl_renderer, GLuint textures_handle) {
	if (gl_renderer->open_gl.vbo_fireballs == 0) {
		glGenBuffers(1, &gl_renderer->open_gl.vbo_quads);
		glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_quads);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo_fireballs);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), (void*)offsetof(Vertex_3D, uv));
	glEnableVertexAttribArray(1);


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

	// V3 light_position = {};
	// if (gl_renderer->fireballs.size() > 0) {
	// 	light_position = gl_renderer->fireballs[0].pos;
	// }

	// Bind the sprite sheet with all the textures
	// log("chunks_to_draw_size = %i", gl_renderer->chunks_to_draw.size());
	for (Chunk* chunk: gl_renderer->chunks_to_draw) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
		if (chunk->ebo <= 0) {
			assert(false);
		}
		glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, normal));
		glEnableVertexAttribArray(2);

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

		GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
		glUniform1f(time_loc, gl_renderer->time);

		// GLuint light_fireball_loc = glGetUniformLocation(shader_program, "light_position_fireball");
		// glUniform3fv(time_loc, );

		glDrawElements(GL_TRIANGLES, chunk->total_indices_to_be_rendered, GL_UNSIGNED_INT, (void*)0);
	}
}
#endif

#if 0
void gl_draw_cube_faces_vbo(GL_Renderer* gl_renderer, GLuint textures_handle) {
	V3 light_position = {};
	if (gl_renderer->fireballs.size() > 0) {
		light_position = gl_renderer->fireballs[0].pos;
	}

	// Bind the sprite sheet with all the textures
	glBindTexture(GL_TEXTURE_2D, textures_handle);

	for (const Chunk_Vbo& chunk_vbo : gl_renderer->open_gl.chunk_vbos) {
		glBindBuffer(GL_ARRAY_BUFFER, chunk_vbo.vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D_Faces), (void*)offsetof(Vertex_3D_Faces, normal));
		glEnableVertexAttribArray(2);

		GLuint shader_program = shader_program_types[SPT_Cube_Face];
		if (!shader_program) {
			log("ERROR: Shader program not specified");
			assert(false);
		}
		glUseProgram(shader_program);

		MX4 perspective_from_world = gl_renderer->perspective_from_view * gl_renderer->view_from_world;

		GLuint perspective_from_world_loc = glGetUniformLocation(shader_program, "perspective_from_world");
		glUniformMatrix4fv(perspective_from_world_loc, 1, GL_FALSE, perspective_from_world.e);

		GLuint time_loc = glGetUniformLocation(shader_program, "u_time");
		glUniform1f(time_loc, gl_renderer->time);

		// GLuint light_fireball_loc = glGetUniformLocation(shader_program, "light_position_fireball");
		// glUniform3fv(time_loc, );

		glDrawArrays(GL_TRIANGLES, 0, chunk_vbo.total_vertices);
	}
}
#endif

#if 0
void draw_chunk(GL_Renderer* gl_renderer, Chunk_Index chunk_index) {
	Chunk* chunk = &world_chunks[chunk_index];
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_LENGTH; y++) {
			for (int z = 0; z < CHUNK_HEIGHT; z++) {
				Cube* current_cube = &chunk->cubes[x][y][z];
				float world_space_x = x + (float)chunk_index.x * CHUNK_WIDTH;
				float world_space_y = y + (float)chunk_index.y * CHUNK_LENGTH;
				float world_space_z = (float)z;

				V3 cube_ws_pos = { world_space_x, world_space_y, world_space_z };

				draw_cube_type(gl_renderer, cube_ws_pos, current_cube->type);
			}
		}
	}
}

void load_chunks_around_player(GL_Renderer* gl_renderer, float noise) {
	int chunk_player_is_on_x = (int)gl_renderer->player_pos.x / CHUNK_WIDTH;
	int chunk_player_is_on_z = (int)gl_renderer->player_pos.y / CHUNK_LENGTH;

	for (int x = -VIEW_DISTANCE; x < VIEW_DISTANCE; x++) {
		for (int y = -VIEW_DISTANCE; y < VIEW_DISTANCE; y++) {
			Chunk_Index index;
			index.x = chunk_player_is_on_x + x;
			index.y = chunk_player_is_on_y + y;
			generate_chunk(index, noise);
		}
	}
}

void draw_chunks_around_player(GL_Renderer* gl_renderer) {
	int chunk_player_is_on_x = (int)gl_renderer->player_pos.x / CHUNK_WIDTH;
	int chunk_player_is_on_y = (int)gl_renderer->player_pos.y / CHUNK_LENGTH;

	for (int x = -VIEW_DISTANCE; x < VIEW_DISTANCE; x++) {
		for (int y = -VIEW_DISTANCE; y < VIEW_DISTANCE; y++) {
			Chunk_Index index;
			index.x = chunk_player_is_on_x + x;
			index.y = chunk_player_is_on_y + y;
			draw_chunk(gl_renderer, index);
		}
	}

}
#endif

void generate_and_draw_chunks_around_player(GL_Renderer* gl_renderer, GLuint textures_handle, float noise) {
	profile_time_to_execute_start_milliseconds();
	// ONLY overwrite if we are in range of new chunks
	// log("x = %fg, z = %f", gl_renderer->player_pos.x, gl_renderer->player_pos.y);
	int current_chunk_player_x = (int)gl_renderer->player_pos.x / CHUNK_WIDTH;
	int current_chunk_player_y = (int)gl_renderer->player_pos.y / CHUNK_LENGTH;

	int previous_chunk_player_x = gl_renderer->previous_chunk_player_x;
	int previous_chunk_player_y = gl_renderer->previous_chunk_player_y;

	bool player_on_same_chunk = false;
	if (current_chunk_player_x == previous_chunk_player_x
		&& current_chunk_player_y == previous_chunk_player_y) {
		player_on_same_chunk = true;
	}
	std::vector<V2> chunks_to_generate = {};
	if (player_on_same_chunk == false || force_regenerate) {
		for (Chunk* chunk : gl_renderer->chunks_to_draw) {
			chunk->allocated = false;
		}
		gl_renderer->previous_chunk_player_x = current_chunk_player_x;
		gl_renderer->previous_chunk_player_y = current_chunk_player_y;
		for (int x = -VIEW_DISTANCE; x < VIEW_DISTANCE; x++) {
			for (int y = -VIEW_DISTANCE; y < VIEW_DISTANCE; y++) {
				int chunk_x = x + current_chunk_player_x;
				int chunk_y = y + current_chunk_player_y;

				bool already_generated = false;
				Chunk* previous_chunk = nullptr;
				for (Chunk* chunk : gl_renderer->chunks_to_draw) {
					if (chunk->chunk_x == chunk_x &&
						chunk->chunk_y == chunk_y) {
						// Not the greatest fix
						if (already_generated && previous_chunk != nullptr) {
							previous_chunk->allocated = false;
							// Delete any duplicate chunks
							chunk->allocated = true;
							continue;
						}
						already_generated = true;
						chunk->allocated = true;
						previous_chunk = chunk;
					}
				}
				if (already_generated == false) {
					chunks_to_generate.push_back({(float)chunk_x, (float)chunk_y});
				}
			}
		}
	}
	// Do this after the loop so we don't overwrite any currently allocated chunks
	// log("chunks_to_generate_size = %i", chunks_to_generate.size());
	// Generate the chunks first
	for (V2 chunk : chunks_to_generate) {
		generate_world_chunk(gl_renderer, (int)chunk.x, (int)chunk.y, noise);
	}
	// Buffer the data second
	for (V2 chunk : chunks_to_generate) {
		buffer_world_chunk(gl_renderer, (int)chunk.x, (int)chunk.y);
	}
	chunks_to_generate.clear();
	std::erase_if(gl_renderer->chunks_to_draw, [](const Chunk* chunk) {
		if (!chunk->allocated) {
			glDeleteBuffers(1, &chunk->vbo);
			delete chunk;
			return true;
		}
		return false;
	});
	gl_draw_cube_faces_vbo(gl_renderer, textures_handle);
	force_regenerate = false;
	profile_time_to_execute_finish_milliseconds("generate_and_draw_chunks_around_player", false);
}

void draw_wire_frames(GL_Renderer* gl_renderer){
	for (int x = -VIEW_DISTANCE; x < VIEW_DISTANCE; x++) {
		for (int y = -VIEW_DISTANCE; y < VIEW_DISTANCE; y++) {
			int chunk_selected_x = x + gl_renderer->previous_chunk_player_x;
			int chunk_selected_y = y + gl_renderer->previous_chunk_player_y;
			// This is the corner position of the chunk. 
			// This is the position I'm drawing from.
			V3 chunk_ws_pos = {(float)chunk_selected_x * CHUNK_WIDTH, (float)chunk_selected_y * CHUNK_LENGTH, 0};

			// The points draw in the center of the cube. Need to offset them.
			V3 p1 = chunk_ws_pos;
			V3 p2 = chunk_ws_pos;

			int w = CHUNK_WIDTH;
			int l = CHUNK_LENGTH;
			int h = CHUNK_HEIGHT;

#if 0
			// Left face
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z    }); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,		p1.z + h }, { p2.x,     p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,		p1.z + h }, { p2.x,     p2.y,     p2.z    }); 
			// Right face
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y + l,     p1.z},     { p2.x + w, p2.y + l,     p2.z    });
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y + l,     p1.z},	   { p2.x + w, p2.y + l,	 p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y + l,     p1.z + h}, { p2.x,     p2.y + l,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y + l,     p1.z + h}, { p2.x,     p2.y + l,     p2.z    }); 
			// Connect the squares
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z     }, { p2.x,		p2.y + l, p2.z});
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y + l, p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z + h }, { p2.x + w, p2.y + l, p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z + h }, { p2.x,     p2.y + l, p2.z});  
#endif

			// Left face (Y-Z plane)
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z     }, { p2.x,     p2.y + l, p2.z    }); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,     p1.y + l, p1.z     }, { p2.x,     p2.y + l, p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,     p1.y + l, p1.z + h }, { p2.x,     p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z + h }, { p2.x,     p2.y,     p2.z    });

			// Right face (Y-Z plane offset by width)
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y + l, p2.z    }); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y + l, p1.z     }, { p2.x + w, p2.y + l, p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y + l, p1.z + h }, { p2.x + w, p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z + h }, { p2.x + w, p2.y,     p2.z    });

			// Connect the squares (X-Z plane)
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z    });
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z     }, { p2.x + w, p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x + w, p1.y,     p1.z + h }, { p2.x,     p2.y,     p2.z + h}); 
			gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, { p1.x,	  p1.y,     p1.z + h }, { p2.x,     p2.y,     p2.z    });
		}
	}
}

void update_fireballs(GL_Renderer* gl_renderer) {
	for (Fireball& fireball : gl_renderer->fireballs) {
		fireball.pos.x += (fireball.velocity.x * gl_renderer->fireball_speed) * delta_time;
		fireball.pos.y += (fireball.velocity.y * gl_renderer->fireball_speed) * delta_time;
		fireball.pos.z += (fireball.velocity.z * gl_renderer->fireball_speed) * delta_time;
	}
}

LRESULT windowProcedure(HWND windowHandle, UINT messageType, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = {};
	switch (messageType) {
	case WM_KEYDOWN: {
		key_states[wParam].pressed_this_frame = true;
		key_states[wParam].held_down = true;
		// log("WM_KEYDOWN: %llu", wParam);
	} break;
	case WM_KEYUP: {
		key_states[wParam].held_down = false;
	} break;
	case WM_MOUSEMOVE: {
		current_mouse_position.x = (float)GET_X_LPARAM(lParam);
		current_mouse_position.y = (float)GET_Y_LPARAM(lParam);
	} break;
	case WM_LBUTTONDOWN: {
		key_states[wParam].pressed_this_frame = true;
		key_states[wParam].held_down = true;
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

void draw_mx4(GL_Renderer* gl_renderer, std::string mx_name, Font* font, MX4 mx, int pos_x, int pos_y, int size, bool center) {
	int offset_y = 0;
	std::string title = " " + mx_name;
	draw_string(gl_renderer, font, title.c_str(), pos_x, pos_y + offset_y, size, center);
	offset_y += font->char_h * size;

	MX4 mx_transposed = matrix_transpose(mx);

	std::string row_1 = std::format("{:>5.2f}", mx_transposed.e[0]) 
		+ " " + std::format("{:>5.2f}", mx_transposed.e[1])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[2])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[3]);
	draw_string(gl_renderer, font, row_1.c_str(), pos_x, pos_y + offset_y, size, center);
	offset_y += font->char_h * size;

	std::string row_2 = std::format("{:>5.2f}",mx_transposed.e[4]) 
		+ " " + std::format("{:>5.2f}", mx_transposed.e[5])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[6])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[7]);
	draw_string(gl_renderer, font, row_2.c_str(), pos_x, pos_y + offset_y, size, center);
	offset_y += font->char_h * size;

	std::string row_3 = std::format("{:>5.2f}", mx_transposed.e[8]) 
		+ " " + std::format("{:>5.2f}", mx_transposed.e[9])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[10])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[11]);
	draw_string(gl_renderer, font, row_3.c_str(), pos_x, pos_y + offset_y, size, center);
	offset_y += font->char_h * size;

	std::string row_4 = std::format("{:>5.2f}", mx_transposed.e[12]) 
		+ " " + std::format("{:>5.2f}", mx_transposed.e[13])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[14])
		+ " " + std::format("{:>5.2f}", mx_transposed.e[15]);
	draw_string(gl_renderer, font, row_4.c_str(), pos_x, pos_y + offset_y, size, center);
	offset_y += font->char_h * size;
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

	create_cube_map();

	init_clock();
	float previous_frame_time = 0;
	float current_time = 0;

	// generate_world_chunks(gl_renderer, 20);

	gl_renderer->fireball_speed = 10.0f;

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

		uint64_t current_milliseconds = get_clock_milliseconds();
		current_time = ((float)current_milliseconds / 1000.0f);
		delta_time = (current_time - previous_frame_time);
		if (delta_time > 0.5f) {
			delta_time = 0.5f;
		}
		previous_frame_time = current_time;

		update_fireballs(gl_renderer);

		// GL_COLOR_BUFFER_BIT: This clears the color buffer, which is responsible for holding the color 
		// information of the pixels. Clearing this buffer sets all the pixels to the color specified by glClearColor.
		// GL_DEPTH_BUFFER_BIT: This clears the depth buffer, which is responsible for holding the depth 
		// information of the pixels. The depth buffer keeps track of the distance from the camera to each pixel
		// to handle occlusion correctly.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		draw_cube_map(gl_renderer);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		// Drawing 3D Cubes
		// float pos_x = -10.0f;
		// MX4 mx_parent = translation_matrix_mx_4(pos_x, 0, 0);
		// draw_cube_mx(gl_renderer, mx_parent);
		// for (int i = 0; i < 20; i++) {
		// 	mx_parent = mx_parent * mat4_rotate_x(sin(gl_renderer->time * 10) * 0.01f) * translation_matrix_mx_4(0, 1, 0);
		// 	draw_cube_mx(gl_renderer, mx_parent);
		// }

#if 0
		float pos_x = -10.0f + sin(gl_renderer->time);
		MX4 mx_parent = translation_matrix_mx_4(pos_x, 0, 0);
		draw_cube_mx(gl_renderer, mx_parent);
		MX4 mx_child = mx_parent * mat4_rotate_y(sin(gl_renderer->time)) * translation_matrix_mx_4(2, 0, 0) * mat4_rotate_y(sin(gl_renderer->time));
		draw_cube_mx(gl_renderer, mx_child);
		// *******
		MX4 mx_parent = translation_matrix_mx_4(pos_x, 0, 0) * mat4_rotate_x(sin(gl_renderer->time));
		draw_cube_mx(gl_renderer, mx_parent);
		MX4 mx_child = mx_parent * translation_matrix_mx_4(0, 1, 0);
		draw_cube_mx(gl_renderer, mx_child);
		MX4 mx_child_of_child = mx_child * translation_matrix_mx_4(0, 1, 0) * scaling_matrix_mx_4(0.5f, 0.5f, 0.5f);
		draw_cube_mx(gl_renderer, mx_child_of_child);
		MX4 mx_child_of_that_child = mx_child_of_child * translation_matrix_mx_4(0, 2, 0) * scaling_matrix_mx_4(2.0f, 2.0f, 2.0f);
		draw_cube_mx(gl_renderer, mx_child_of_that_child);
#endif

		// draw_chunks(gl_renderer);
		// gl_draw_cubes(gl_renderer);
		// Drawing fireballs
		gl_draw_fireballs(gl_renderer);

		// Generating Face VBO Chunks
		// gl_draw_cube_faces_vbo(gl_renderer, texture_sprite_sheet_handle);
		generate_and_draw_chunks_around_player(gl_renderer, texture_sprite_sheet_handle, 20);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// Drawing 3D lines
		if (toggle_debug_info) {
			draw_wire_frames(gl_renderer);
		}
		V3 p1_x = { 0, 0, 0 } ;
		V3 p2_x = { 5, 0, 0 } ;
		gl_draw_line(gl_renderer, { 1, 0, 0, 1 }, p1_x, p2_x);

		V3 p1_y = { 0, 0, 0 } ;
		V3 p2_y = { 0, 5, 0 } ;
		gl_draw_line(gl_renderer, { 0, 1, 0, 1 }, p1_y, p2_y);

		V3 p1_z = { 0, 0, 0 } ;
		V3 p2_z = { 0, 0, 5 } ;
		gl_draw_line(gl_renderer, { 0, 0, 1, 1 }, p1_z, p2_z);
		gl_upload_and_draw_lines_vbo(gl_renderer);

		// Drawing Strings
		// draw_string_ws(gl_renderer, &font, "First 3D string", { 0,0,0 }, 5, true);
		draw_string(gl_renderer, &font, "Hello world!", 250, 500, 5, true);
		// The transposed view matrix
		MX4 view_mx = matrix_transpose(gl_renderer->view_from_world);
		draw_mx4(gl_renderer, "View_from_world", &font, view_mx, 0, 0, 2, false);
		
		draw_string_ws(gl_renderer, &font, "x", { 5, 0, 0 }, 3, true);
		draw_string_ws(gl_renderer, &font, "y", { 0, 5, 0 }, 3, true);
		draw_string_ws(gl_renderer, &font, "z", { 0, 0, 5 }, 3, true);

		gl_upload_and_draw_2d_string(gl_renderer, &font);

		gl_update_renderer(gl_renderer);
		SwapBuffers(gl_renderer->hdc);

		init_job_system(); 

		add_job(JT_Increment_Number);

		execute_all_jobs();
		
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
