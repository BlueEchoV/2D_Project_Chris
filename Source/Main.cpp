#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Utility.h"
#include "Renderer.h"
#include "Math.h"

struct Key_State {
	bool pressed_this_frame;
	bool held_down;
};

std::unordered_map<LPARAM, Key_State> key_states;

void reset_Pressed_This_Frame() {
	for (auto& key_State : key_states) {
		key_states[key_State.first].pressed_this_frame = false;
	}
}

LRESULT windowProcedure(HWND windowHandle, UINT messageType, WPARAM wParam, LPARAM lParam)
{
	reset_Pressed_This_Frame();
	LRESULT result = {};
	switch (messageType) {
	case WM_KEYDOWN: {
		key_states[wParam].pressed_this_frame = true;
		key_states[wParam].held_down = true;
		log("WM_KEYDOWN: %llu", wParam);
		break;
	}
	case WM_KEYUP: {
		key_states[wParam].held_down = false;
	}
	case WM_LBUTTONDOWN: {
		break;
	}
	case WM_CLOSE: {
		DestroyWindow(windowHandle);
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	} 
	default: {
		result = DefWindowProc(windowHandle, messageType, wParam, lParam);
	}
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
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);

	Image castle_infernal_image = create_Image(renderer, "assets\\castle_Infernal.png");
	SDL_SetTextureBlendMode(castle_infernal_image.texture, SDL_BLENDMODE_BLEND);
	Image azir_image = create_Image(renderer, "assets\\azir.jpg");
	SDL_SetTextureBlendMode(azir_image.texture, SDL_BLENDMODE_BLEND);

	Image cobblestone_image = create_Image(renderer, "assets\\cobblestone.png");
	SDL_SetTextureBlendMode(cobblestone_image.texture, SDL_BLENDMODE_BLEND);
	Image dirt_image = create_Image(renderer, "assets\\dirt.png");
	SDL_SetTextureBlendMode(dirt_image.texture, SDL_BLENDMODE_BLEND);

	bool running = true;
	while (running) {
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
			
		}

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
		SDL_RenderClear(renderer);

		SDL_Rect clip_rect = { 100, 10, 500, 500 };
		// SDL_SetRenderDrawColor(renderer, 0, 100, 100, 255);
		// SDL_RenderFillRect(renderer, &clip_rect);
		// SDL_RenderSetClipRect(renderer, &clip_rect);

		SDL_Rect viewport_rect = { 100, 10, 500, 500 };
		// SDL_RenderSetViewport(renderer, &clip_rect);
		
		draw_debug_images(renderer);

		// SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		SDL_Rect castle_rect = { 100,400,200,200 };
		SDL_SetTextureAlphaMod(castle_infernal_image.texture, 155);
		SDL_RenderCopy(renderer, castle_infernal_image.texture, NULL, &castle_rect);

		SDL_Rect azir_rect = { 200,400,200,200 };
		SDL_SetTextureColorMod(azir_image.texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(azir_image.texture, 155);
		SDL_RenderCopy(renderer, azir_image.texture, NULL, &azir_rect);

		// for (float i = 0; i <= 20; i += 2) {
		// 	for (float j = 0; j <= 20; j += 2) {
		// 		float x = i - 10;
		// 		// Invert the drawing so the cubes dont draw on top of eachother
		// 		if (x > 0) {
		// 			x = 10 - x;
		// 		}
		// 		V3 cube_pos = { x, -4, j - 15};
		// 		mp_draw_cube(renderer, cube_pos, cobblestone_image.texture);
		// 	}
		// }

		static float width = 0;
		static float depth = 0;
		if (key_states[VK_INSERT].pressed_this_frame || key_states[VK_INSERT].held_down) {
			width += 1;
		}
		if (key_states[VK_DELETE].pressed_this_frame || key_states[VK_DELETE].held_down) {
			if (width > 0) {
				width -= 1;
			}
		}
		if (key_states[VK_HOME].pressed_this_frame || key_states[VK_HOME].held_down) {
			depth += 1;
		}
		if (key_states[VK_END].pressed_this_frame || key_states[VK_END].held_down) {
			if (depth > 0) {
				depth -= 1;
			}
		}
		for (float i = 0; i <= width; i += 2) {
			for (float j = 0; j <= depth; j += 2) {
				V3 cube_pos = { i - (width / 2), -4, j - (depth / 2)};
				mp_draw_cube(renderer, cube_pos, cobblestone_image.texture);
			}
		}

		static V3 player_pos = { 0, -2, 0 };
		static float player_speed = 0.05f;
		// TODO: There is a bug with holding down the keys simultaneously and 
		// the player moving faster diagonally. 
		if (key_states[VK_DOWN].pressed_this_frame || key_states[VK_DOWN].held_down) {
			player_pos.z += player_speed;
		} 
		if (key_states[VK_UP].pressed_this_frame || key_states[VK_UP].held_down) {
			player_pos.z -= player_speed;
		} 
		if (key_states[VK_RIGHT].pressed_this_frame || key_states[VK_RIGHT].held_down) {
			player_pos.x += player_speed;
		}
		if (key_states[VK_LEFT].pressed_this_frame || key_states[VK_LEFT].held_down) {
			player_pos.x -= player_speed;
		}
		if (key_states[VK_SPACE].pressed_this_frame || key_states[VK_SPACE].held_down) {
			player_pos.y += player_speed;
		}
		if (key_states[VK_CONTROL].pressed_this_frame || key_states[VK_CONTROL].held_down) {
			player_pos.y -= player_speed;
		}
		if (key_states[VK_SHIFT].pressed_this_frame || key_states[VK_SHIFT].held_down) {
			player_speed = 0.1f;
		} else {
			player_speed = 0.05f;
		}
		mp_draw_cube(renderer, player_pos, dirt_image.texture);

		SDL_RenderPresent(renderer);

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
	SDL_DestroyRenderer(renderer);
	return 0;
}

// Archived for now

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
