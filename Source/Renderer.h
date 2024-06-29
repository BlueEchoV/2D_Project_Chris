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

struct SDL_Texture;
struct Image {
	int width;
	int height;
	const char* file_Path;
	SDL_Texture* texture;
	unsigned char* pixel_Data;
};

typedef enum SDL_BlendMode
{
    SDL_BLENDMODE_NONE = 0x00000000,     /**< no blending
                                              dstRGBA = srcRGBA */
    SDL_BLENDMODE_BLEND = 0x00000001,    /**< alpha blending
                                              dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA))
                                              dstA = srcA + (dstA * (1-srcA)) */
    SDL_BLENDMODE_ADD = 0x00000002,      /**< additive blending
                                              dstRGB = (srcRGB * srcA) + dstRGB
                                              dstA = dstA */
    SDL_BLENDMODE_MOD = 0x00000004,      /**< color modulate
                                              dstRGB = srcRGB * dstRGB
                                              dstA = dstA */
    SDL_BLENDMODE_MUL = 0x00000008,      /**< color multiply
                                              dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA))
                                              dstA = dstA */
    SDL_BLENDMODE_INVALID = 0x7FFFFFFF

    /* Additional custom blend modes can be returned by SDL_ComposeCustomBlendMode() */

} SDL_BlendMode;

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
struct SDL_Texture;
int SDL_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blend_mode);
int SDL_LockTexture(SDL_Texture * texture, const SDL_Rect * rect, void **pixels, int *pitch);
SDL_Texture* SDL_CreateTexture(SDL_Renderer* sdl_renderer, uint32_t format, int access, int w, int h);
int SDL_RenderCopy(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
void SDL_RenderPresent(SDL_Renderer* sdl_renderer);

Image create_Image(SDL_Renderer* sdl_renderer, const char* file_Path);
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