#pragma once
#include <stdint.h>
#include <Windows.h>

struct SDL_Rect {
	int x, y;
	int w, h;
};

struct SDL_Point {
	int x, y;
};

struct SDL_Renderer;
SDL_Renderer* SDL_CreateRenderer(HWND window, int index, uint32_t flags);
int SDL_SetRenderDrawColor(SDL_Renderer* sdl_renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int SDL_RenderClear(SDL_Renderer* sdl_renderer);
int SDL_RenderFillRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int SDL_RenderFillRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);
int SDL_RenderDrawLine(SDL_Renderer* sdl_renderer, int x1, int y1, int x2, int y2);
int SDL_RenderDrawLines(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int SDL_RenderDrawPoint(SDL_Renderer* sdl_renderer, int x, int y);
int SDL_RenderDrawPoints(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int SDL_RenderDrawRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int SDL_RenderDrawRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);
void SDL_RenderPresent(SDL_Renderer* sdl_renderer);

#if 0
SDL_GetRenderDrawBlendMode
SDL_GetRenderDrawColor
SDL_GetRendererOutputSize
SDL_GetTextureAlphaMod
SDL_GetTextureBlendMode
SDL_GetTextureColorMod
SDL_GetTextureUserData
SDL_LockTexture
SDL_QueryTexture
SDL_RenderCopy
SDL_RenderCopyEx
SDL_RenderGetClipRect
SDL_RenderGetViewport
SDL_RenderGetWindow
SDL_RenderIsClipEnabled
SDL_RenderSetClipRect
SDL_RenderSetViewport
SDL_RenderSetVSync
SDL_SetRenderDrawBlendMode
SDL_SetRenderDrawColor
SDL_SetTextureAlphaMod
SDL_SetTextureBlendMode
SDL_SetTextureColorMod
SDL_SetTextureScaleMode
SDL_SetTextureUserData
SDL_UnlockTexture
SDL_UpdateTexture
#endif