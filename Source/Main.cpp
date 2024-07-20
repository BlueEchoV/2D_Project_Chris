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

struct Open_GL {
	GLuint vao;
	GLuint vbo;
};

struct GL_Renderer {
	HWND window;
	HDC hdc;

	Open_GL open_gl;
};

void init_open_gl(GL_Renderer* gl_renderer) {
	glGenVertexArrays(1, &gl_renderer->open_gl.vao);
	glBindVertexArray(gl_renderer->open_gl.vao);

	glGenBuffers(1, &gl_renderer->open_gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl_renderer->open_gl.vbo);

}

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

	init_open_gl(result);

	load_shaders();

	return result;
}

void gl_draw_line() {

}

/*
struct GL_Texture {
	GLuint handle;
	uint32_t format;
	int access;
	int w, h;

	int pitch;
	// Locked if null
	void* pixels;
	SDL_Rect portion;
};

GL_Texture gl_create_texture(GL_Renderer* gl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, result.width, result.height) {
	if (gl_renderer == nullptr) {
		log("ERROR: gl_renderer is nullptr");
		assert(false);
	}

	GL_Texture* result = new GL_Texture();
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

enum Image_Type {
	IT_Cobblestone,
	IT_Dirt,
	IT_Grass,
    IT_Air,
	IT_Total
};

struct Image {
	int width;
	int height;
	const char* file_Path;
	GL_Texture* texture;
	unsigned char* pixel_Data;
};

Image images[IT_Total] = {};

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

	GL_Texture* temp = gl_create_texture(gl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, result.width, result.height);

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
void init_images(GL_Renderer* gl_renderer) {
	if (gl_renderer == nullptr) {
		log("ERROR: gl_renderer is nullptr");
		assert(false);
		return;
	}

	images[IT_Cobblestone] = create_Image(sdl_renderer, "assets\\cobblestone.png");
	SDL_SetTextureBlendMode(images[IT_Cobblestone].texture, SDL_BLENDMODE_BLEND);
	images[IT_Dirt] = create_Image(sdl_renderer, "assets\\dirt.png");
	SDL_SetTextureBlendMode(images[IT_Dirt].texture, SDL_BLENDMODE_BLEND);
	images[IT_Grass] = create_Image(sdl_renderer, "assets\\grass.png");
	SDL_SetTextureBlendMode(images[IT_Grass].texture, SDL_BLENDMODE_BLEND);
}

SDL_Texture* get_image(Image_Type type) {
	SDL_Texture* result = nullptr;

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

void draw_perlin_cube(SDL_Renderer* sdl_renderer, V3 pos, float perlin) {
	Image_Type result;

	if (perlin > -0.1) {
		result = IT_Grass;
	}
	else if (perlin > -0.35) {
		result = IT_Dirt;
	}
	// Default
	else {
		result = IT_Cobblestone;
	}

	SDL_Texture* result_texture = images[result].texture;
	mp_draw_cube(sdl_renderer, pos, result_texture);
}

void draw_cube_face_type(SDL_Renderer* sdl_renderer, MP_Rect_3D rect, Image_Type type) {
	SDL_Texture* result = get_image(type);
	
	if (result != nullptr) {
		mp_draw_cube_face(sdl_renderer, rect, result);
	}
}

void draw_cube_type(SDL_Renderer* sdl_renderer, V3 pos, Image_Type type) {
	SDL_Texture* result = get_image(type);

	if (result != nullptr) {
		mp_draw_cube(sdl_renderer, pos, result);
	}
}

struct Cube {
	Image_Type type;
};

const int CHUNK_SIZE = 5;
struct Chunk {
	Cube cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {};
};

const int WORLD_SIZE_WIDTH = 10;
const int WORLD_SIZE_LENGTH = 10;
const int WORLD_SIZE_HEIGHT = 5;
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
	int final_arr_pos_x = clamp(x_arr_pos, 0, WORLD_SIZE_WIDTH);
	int final_arr_pos_y = clamp(y_arr_pos, 0, WORLD_SIZE_HEIGHT);
	int final_arr_pos_z = clamp(z_arr_pos, 0, WORLD_SIZE_LENGTH);

	Chunk new_chunk = {};
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				float world_space_x = x + (float)final_arr_pos_x * CHUNK_SIZE;
				float world_space_y = y + (float)final_arr_pos_y * CHUNK_SIZE;
				float world_space_z = z + (float)final_arr_pos_z * CHUNK_SIZE;
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
					} 
					else {
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

	world_chunks[final_arr_pos_x][final_arr_pos_y][final_arr_pos_z] = new_chunk;
}

void generate_world_chunks(float noise) {
	generate_height_map(noise);
	for (int x = 0; x < WORLD_SIZE_WIDTH; x++) {
		for (int y = 0; y < WORLD_SIZE_HEIGHT; y++) {
			for (int z = 0; z < WORLD_SIZE_LENGTH; z++) {
				generate_world_chunk(x, y, z, noise);
			}
		}
	}
}

void draw_chunk(SDL_Renderer* sdl_renderer, int world_space_x, int world_space_y, int world_space_z) {
	Chunk* chunk = &world_chunks[world_space_x][world_space_y][world_space_z];
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 0; z < CHUNK_SIZE; z++) {
				// Voxel index
				Cube* cube = &chunk->cubes[x][y][z];

				// Loop through that faces 
				for (int face = 0; face < 6; face++) {
					// Air next to the voxels determines if the cubes should be drawn

					// If we renderer, we want to emit 4 vertex and 6 indices to the element buffer

					// NOTE: Generate the whole chunk
					// NOTE: Do texturing later
					// MP_Rect_3D rect;
					// mp_draw_cube_face(sdl_renderer, )
				}

				V3 cube_pos;
				//														Center the world 
				cube_pos.x = (float)(x) - ((float)WORLD_SIZE_WIDTH); // * (float)CHUNK_SIZE) / 2.0f;
				cube_pos.y = (float)(y) - ((float)WORLD_SIZE_HEIGHT); // * (float)CHUNK_SIZE) / 2.0f;
				cube_pos.z = (float)(z) - ((float)WORLD_SIZE_LENGTH); // * (float)CHUNK_SIZE) / 2.0f;

				cube_pos.x += (world_space_x * CHUNK_SIZE);
				cube_pos.y += (world_space_y * CHUNK_SIZE);
				cube_pos.z += (world_space_z * CHUNK_SIZE);

				draw_cube_type(sdl_renderer, cube_pos, cube->type);
			}
		}
	}

}

void draw_chunks(SDL_Renderer* sdl_renderer) {
	for (int x = 0; x < WORLD_SIZE_WIDTH; x++) {
		for (int y = 0; y < WORLD_SIZE_HEIGHT; y++) {
			for (int z = 0; z < WORLD_SIZE_LENGTH; z++) {
				draw_chunk(sdl_renderer, x, y, z);
			}
		}
	}
}

void swap_back_buffer(SDL_Renderer* sdl_renderer) {
	if (sdl_renderer == nullptr) {
		log("Error: SDL_Renderer is nullptr");
		assert(false);
		return;
	}
	Renderer* renderer = 
}
*/
#if 0
void draw_wire_frame(SDL_Renderer* sdl_renderer, V3 pos, float width, float length, float height) {
	REF(length);
	REF(height);

	// Pillars 
	for (int i = 0; i < width; i++) {
		float x = i + pos.x;
		float y = i + pos.y;
		float z = i + pos.z;

		mp_draw_3d_line(sdl_renderer, {x,		 y, z},        0, 0);
		mp_draw_3d_line(sdl_renderer, {x + 1.0f, y, z},        0, 0);
		mp_draw_3d_line(sdl_renderer, {x,		 y, z + 1.0f}, 0, 0);
		mp_draw_3d_line(sdl_renderer, {x + 1.0f, y, z + 1.0f}, 0, 0);
	}

	mp_draw_3d_line(sdl_renderer, {pos.x,		 pos.y - 0.5f, pos.z + 0.5f},  90, 0);
	mp_draw_3d_line(sdl_renderer, {pos.x,		 pos.y + 0.5f, pos.z + 0.5f},  90, 0);
	mp_draw_3d_line(sdl_renderer, {pos.x + 1.0f, pos.y - 0.5f, pos.z + 0.5f},  90, 0);
	mp_draw_3d_line(sdl_renderer, {pos.x + 1.0f, pos.y + 0.5f, pos.z + 0.5f},  90, 0);
	mp_draw_3d_line(sdl_renderer, {pos.x + 0.5f, pos.y - 0.5f, pos.z},  0, 90);
	mp_draw_3d_line(sdl_renderer, {pos.x + 0.5f, pos.y + 0.5f, pos.z},  0, 90);
	mp_draw_3d_line(sdl_renderer, {pos.x + 0.5f, pos.y - 0.5f, pos.z + 1.0f},  0, 90);
	mp_draw_3d_line(sdl_renderer, {pos.x + 0.5f, pos.y + 0.5f, pos.z + 1.0f},  0, 90);
}
#endif

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

	/*
	Image castle_infernal_image = create_Image(renderer, "assets\\castle_Infernal.png");
	SDL_SetTextureBlendMode(castle_infernal_image.texture, SDL_BLENDMODE_BLEND);
	Image azir_image = create_Image(renderer, "assets\\azir.jpg");
	SDL_SetTextureBlendMode(azir_image.texture, SDL_BLENDMODE_BLEND);
	init_images(renderer);

	generate_world_chunks(20);
	*/

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


		SwapBuffers(gl_renderer->hdc);

		Sleep(1);
	}
	// TODO: Clean up shaders
	#if 0
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteProgram();
		glGetProgramInfoLog();
		glUseProgram();
	#endif
	// SDL_DestroyRenderer(renderer);
	return 0;
}

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
