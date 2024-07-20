#pragma once
#include <stdint.h>
#include <Windows.h>
#include <unordered_map>

#include "Math.h"

#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <gl/GL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

typedef void(*glDisableVertexAttribArrayFunc)(GLuint index);
glDisableVertexAttribArrayFunc glDisableVertexAttribArray = {};

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

typedef void (*glUniformMatrix4fvFunc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
glUniformMatrix4fvFunc glUniformMatrix4fv = {};

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
	glDisableVertexAttribArray = (glDisableVertexAttribArrayFunc)wglGetProcAddress("glDisableVertexAttribArray");
	glBindVertexArray = (glBindVertexArrayFunc)wglGetProcAddress("glBindVertexArray");
	glBindBuffer = (glBindBufferFunc)wglGetProcAddress("glBindBuffer");
	glBufferData = (glBufferDataFunc)wglGetProcAddress("glBufferData");

	glGetUniformLocation = (glGetUniformLocationFunc)wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (glUniform1fFunc)wglGetProcAddress("glUniform1f");
	glUniform1i = (glUniform1iFunc)wglGetProcAddress("glUniform1i");
	glUniformMatrix4fv = (glUniformMatrix4fvFunc)wglGetProcAddress("glUniformMatrix4fv");

	wglCreateContextAttribsARB = (wglCreateContextAttribsARBFunc)wglGetProcAddress("wglCreateContextAttribsARB");

	glActiveTexture = (glActiveTextureFunc)wglGetProcAddress("glActiveTexture");
}

 enum Shader_Program_Type {
	SPT_COLOR,
	SPT_TEXTURE,
	SPT_3D,
	SPT_3D_Lines,
	SPT_Cube_Face,
	SPT_TOTAL
};

 GLuint create_shader_program(const char* vertex_shader_file_path, const char* fragment_shader_file_path);
 
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

	const char* three_d_vertex_shader_file_path = "Shaders\\vertex_shader_3d.txt";
	const char* three_d_fragment_shader_file_path = "Shaders\\fragment_shader_3d.txt";
	GLuint three_d_shader = create_shader_program(three_d_vertex_shader_file_path, three_d_fragment_shader_file_path);
	shader_program_types[SPT_3D] = three_d_shader;

	const char* three_d_lines_vertex_shader_file_path = "Shaders\\vertex_shader_3d_lines.txt";
	const char* three_d_lines_fragment_shader_file_path = "Shaders\\fragment_shader_3d_lines.txt";
	GLuint three_d_lines_shader = create_shader_program(three_d_lines_vertex_shader_file_path, three_d_lines_fragment_shader_file_path);
	shader_program_types[SPT_3D_Lines] = three_d_lines_shader;

	const char* v_cube_face_file_path = "Shaders\\v_cube_face.txt";
	const char* f_cube_face_file_path = "Shaders\\f_cube_face.txt";
	GLuint cube_face_shader = create_shader_program(v_cube_face_file_path, f_cube_face_file_path);
	shader_program_types[SPT_Cube_Face] = cube_face_shader;
}

struct SDL_Texture;

struct Image {
	int width;
	int height;
	const char* file_Path;
	SDL_Texture* texture;
	unsigned char* pixel_Data;
};

struct MP_Rect_3D {
    V3 top_left;
    V3 top_right;
    V3 bottom_right;
    V3 bottom_left;
};

enum Color_Selector {
    CS_Red,
    CS_Green,
    CS_Blue,
    CS_Total
};

struct Key_State {
	bool pressed_this_frame;
	bool held_down;
};

extern std::unordered_map<LPARAM, Key_State> key_states;

// Make a function that can return something and expose the variable over globals
V2 get_mouse_delta();

// *********************SDL Information*********************
/**
 *  \name Transparency definitions
 *
 *  These define alpha as the opacity of a surface.
 */
/* @{ */
#define SDL_ALPHA_OPAQUE 255
#define SDL_ALPHA_TRANSPARENT 0
/* @} */

/**
 * \brief A signed 8-bit integer type.
 */
#define SDL_MAX_SINT8   ((Sint8)0x7F)           /* 127 */
#define SDL_MIN_SINT8   ((Sint8)(~0x7F))        /* -128 */
typedef int8_t Sint8;
/**
 * \brief An unsigned 8-bit integer type.
 */
#define SDL_MAX_UINT8   ((Uint8)0xFF)           /* 255 */
#define SDL_MIN_UINT8   ((Uint8)0x00)           /* 0 */
typedef uint8_t Uint8;
/**
 * \brief A signed 16-bit integer type.
 */
#define SDL_MAX_SINT16  ((Sint16)0x7FFF)        /* 32767 */
#define SDL_MIN_SINT16  ((Sint16)(~0x7FFF))     /* -32768 */
typedef int16_t Sint16;
/**
 * \brief An unsigned 16-bit integer type.
 */
#define SDL_MAX_UINT16  ((Uint16)0xFFFF)        /* 65535 */
#define SDL_MIN_UINT16  ((Uint16)0x0000)        /* 0 */
typedef uint16_t Uint16;
/**
 * \brief A signed 32-bit integer type.
 */
#define SDL_MAX_SINT32  ((Sint32)0x7FFFFFFF)    /* 2147483647 */
#define SDL_MIN_SINT32  ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
typedef int32_t Sint32;
/**
 * \brief An unsigned 32-bit integer type.
 */
#define SDL_MAX_UINT32  ((Uint32)0xFFFFFFFFu)   /* 4294967295 */
#define SDL_MIN_UINT32  ((Uint32)0x00000000)    /* 0 */
typedef uint32_t Uint32;

/**
 * \brief A signed 64-bit integer type.
 */
#define SDL_MAX_SINT64  ((Sint64)0x7FFFFFFFFFFFFFFFll)      /* 9223372036854775807 */
#define SDL_MIN_SINT64  ((Sint64)(~0x7FFFFFFFFFFFFFFFll))   /* -9223372036854775808 */
typedef int64_t Sint64;
/**
 * \brief An unsigned 64-bit integer type.
 */
#define SDL_MAX_UINT64  ((Uint64)0xFFFFFFFFFFFFFFFFull)     /* 18446744073709551615 */
#define SDL_MIN_UINT64  ((Uint64)(0x0000000000000000ull))   /* 0 */
typedef uint64_t Uint64;

/**
 *  \name Cast operators
 *
 *  Use proper C++ casts when compiled as C++ to be compatible with the option
 *  -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above).
 */
/* @{ */
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#define SDL_const_cast(type, expression) const_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#define SDL_const_cast(type, expression) ((type)(expression))
#endif
/* @} *//* Cast operators */

/* Define a four character code as a Uint32 */
#define SDL_FOURCC(A, B, C, D) \
    ((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
     (SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

/** Pixel type. */
typedef enum
{
    SDL_PIXELTYPE_UNKNOWN,
    SDL_PIXELTYPE_INDEX1,
    SDL_PIXELTYPE_INDEX4,
    SDL_PIXELTYPE_INDEX8,
    SDL_PIXELTYPE_PACKED8,
    SDL_PIXELTYPE_PACKED16,
    SDL_PIXELTYPE_PACKED32,
    SDL_PIXELTYPE_ARRAYU8,
    SDL_PIXELTYPE_ARRAYU16,
    SDL_PIXELTYPE_ARRAYU32,
    SDL_PIXELTYPE_ARRAYF16,
    SDL_PIXELTYPE_ARRAYF32,

    /* This must be at the end of the list to avoid breaking the existing ABI */
    SDL_PIXELTYPE_INDEX2
} SDL_PixelType;

/** Bitmap pixel order, high bit -> low bit. */
typedef enum
{
    SDL_BITMAPORDER_NONE,
    SDL_BITMAPORDER_4321,
    SDL_BITMAPORDER_1234
} SDL_BitmapOrder;

/** Packed component order, high bit -> low bit. */
typedef enum
{
    SDL_PACKEDORDER_RGBA,
} SDL_PackedOrder;

/** Array component order, low byte -> high byte. */
/* !!! FIXME: in 2.1, make these not overlap differently with
   !!! FIXME:  SDL_PACKEDORDER_*, so we can simplify SDL_ISPIXELFORMAT_ALPHA */
typedef enum
{
    SDL_ARRAYORDER_RGBA,
} SDL_ArrayOrder;

/** Packed component layout. */
typedef enum
{
    SDL_PACKEDLAYOUT_NONE,
    SDL_PACKEDLAYOUT_8888,

} SDL_PackedLayout;

#define SDL_DEFINE_PIXELFOURCC(A, B, C, D) SDL_FOURCC(A, B, C, D)

#define SDL_DEFINE_PIXELFORMAT(type, order, layout, bits, bytes) \
    ((1 << 28) | ((type) << 24) | ((order) << 20) | ((layout) << 16) | \
     ((bits) << 8) | ((bytes) << 0))

#define SDL_PIXELFLAG(X)    (((X) >> 28) & 0x0F)
#define SDL_PIXELTYPE(X)    (((X) >> 24) & 0x0F)
#define SDL_PIXELORDER(X)   (((X) >> 20) & 0x0F)
#define SDL_PIXELLAYOUT(X)  (((X) >> 16) & 0x0F)
#define SDL_BITSPERPIXEL(X) (((X) >> 8) & 0xFF)
#define SDL_BYTESPERPIXEL(X) \
    (SDL_ISPIXELFORMAT_FOURCC(X) ? \
        ((((X) == SDL_PIXELFORMAT_YUY2) || \
          ((X) == SDL_PIXELFORMAT_UYVY) || \
          ((X) == SDL_PIXELFORMAT_YVYU)) ? 2 : 1) : (((X) >> 0) & 0xFF))

#define SDL_ISPIXELFORMAT_INDEXED(format)   \
    (!SDL_ISPIXELFORMAT_FOURCC(format) && \
     ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX1) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX2) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX4) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_INDEX8)))

#define SDL_ISPIXELFORMAT_PACKED(format) \
    (!SDL_ISPIXELFORMAT_FOURCC(format) && \
     ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED8) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED16) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_PACKED32)))

#define SDL_ISPIXELFORMAT_ARRAY(format) \
    (!SDL_ISPIXELFORMAT_FOURCC(format) && \
     ((SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU8) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU16) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYU32) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF16) || \
      (SDL_PIXELTYPE(format) == SDL_PIXELTYPE_ARRAYF32)))

#define SDL_ISPIXELFORMAT_ALPHA(format)   \
    ((SDL_ISPIXELFORMAT_PACKED(format) && \
     ((SDL_PIXELORDER(format) == SDL_PACKEDORDER_RGBA))) || \
    (SDL_ISPIXELFORMAT_ARRAY(format) && \
     ((SDL_PIXELORDER(format) == SDL_ARRAYORDER_RGBA))))

/* The flag is set to 1 because 0x1? is not in the printable ASCII range */
#define SDL_ISPIXELFORMAT_FOURCC(format)    \
    ((format) && (SDL_PIXELFLAG(format) != 1))

/* Note: If you modify this list, update SDL_GetPixelFormatName() */
typedef enum
{
    SDL_PIXELFORMAT_UNKNOWN,
    SDL_PIXELFORMAT_RGBA8888 =
        SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_RGBA,
                               SDL_PACKEDLAYOUT_8888, 32, 4),
    /* Aliases for RGBA byte arrays of color data, for the current platform */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	// We use this one. Don't try to support all of them. Only support what I use.
    SDL_PIXELFORMAT_RGBA32 = SDL_PIXELFORMAT_RGBA8888,
#else
    SDL_PIXELFORMAT_ABGR32 = SDL_PIXELFORMAT_RGBA8888,
#endif
    SDL_PIXELFORMAT_YV12 =      /**< Planar mode: Y + V + U  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', '1', '2'),
    SDL_PIXELFORMAT_IYUV =      /**< Planar mode: Y + U + V  (3 planes) */
        SDL_DEFINE_PIXELFOURCC('I', 'Y', 'U', 'V'),
    SDL_PIXELFORMAT_YUY2 =      /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'U', 'Y', '2'),
    SDL_PIXELFORMAT_UYVY =      /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('U', 'Y', 'V', 'Y'),
    SDL_PIXELFORMAT_YVYU =      /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
        SDL_DEFINE_PIXELFOURCC('Y', 'V', 'Y', 'U'),
    SDL_PIXELFORMAT_NV12 =      /**< Planar mode: Y + U/V interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '1', '2'),
    SDL_PIXELFORMAT_NV21 =      /**< Planar mode: Y + V/U interleaved  (2 planes) */
        SDL_DEFINE_PIXELFOURCC('N', 'V', '2', '1'),
    SDL_PIXELFORMAT_EXTERNAL_OES =      /**< Android video texture format */
        SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ')
} SDL_PixelFormatEnum;

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

/**
 * Flip constants for SDL_RenderCopyEx
 */
typedef enum
{
    SDL_FLIP_NONE = 0x00000000,     /**< Do not flip */
    SDL_FLIP_HORIZONTAL = 0x00000001,    /**< flip horizontally */
    SDL_FLIP_VERTICAL = 0x00000002     /**< flip vertically */
} SDL_RendererFlip;
struct SDL_Rect {
	int x, y;
	int w, h;
};

struct SDL_Point {
	int x, y;
};

struct Color_8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
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

struct Vertex_3D_Temp {
	V3 pos;
};

struct Draw_Call_Info {
	int total_vertices;
	size_t starting_index;
	Uint32 index_buffer_index;
	int total_indices;
	int draw_type;
	Shader_Program_Type type;
	GLuint texture_handle;
	SDL_BlendMode blend_mode;
};

struct Clip_Rect_Info {
	SDL_Rect* rect;
	GLenum setting;
};

struct Viewport_Info {
	SDL_Rect* rect;
};

struct Clear_Screen_Info {
	Color_8 clear_draw_color;
};

struct Draw_Call_3D_Cube {
	V3 pos;
	GLuint texture_handle;
};

struct Draw_Call_3D_Line {
	int total_vertices;
	size_t starting_index;
	Shader_Program_Type shader_type;
};

struct Draw_Call_3D_Cube_Face {
	MP_Rect_3D rect;
	V3 pos_ws;
	GLuint texture_handle;

	int total_vertices;
	size_t starting_index;
	Shader_Program_Type shader_type;
};

enum Command_Type {
	CT_Set_Clip_Rect,
	CT_Set_Viewport,
	CT_Draw_Call,
	CT_Draw_Call_3D,
	CT_Draw_Call_3D_Lines,
	CT_Draw_Call_3D_Cube_Face,
	CT_Clear_Screen
};

struct Command_Packet {
	Command_Type type;

	// Filled based off the command 
	// Chris isn't a huge fan of this naming convention ('info')
	Draw_Call_Info draw_call_info;
	Clip_Rect_Info clip_rect_info;
	Clear_Screen_Info clear_screen_info;
	Viewport_Info viewport_info;

	Draw_Call_3D_Cube draw_call_3d_cube;
	Draw_Call_3D_Line draw_call_3d_line;
	Draw_Call_3D_Cube_Face draw_call_3d_cube_face;

	// Draw color needs to be set for the flush 
	Color_8 draw_color;
};
 
struct Vertex_3D {
	V3 pos;
	Color_f color;
	V2 uv;
	V3 normal;
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
	std::vector<Command_Packet> command_packets;

	std::vector<Vertex_3D_Temp> vertices_3d;

	GLuint vbo_cube_faces;
	std::vector<Vertex_3D> vertices_cube_faces;

	bool clip_rect_set;
	SDL_Rect clip_rect;

	SDL_Rect viewport;

	float time;
	float player_speed;
	V3 player_pos = { 0, 0, 0 };

	// x or z axis (tbd)
	float pitch;
	float roll;
	// y 
	float yaw;

	MX4 perspective_from_view;
	MX4 view_from_world;
};


struct SDL_Renderer;
SDL_Renderer* SDL_CreateRenderer(HWND window, int index, uint32_t flags);
void SDL_DestroyRenderer(SDL_Renderer * renderer);
int SDL_GetRendererOutputSize(SDL_Renderer* sdl_renderer, int* w, int* h);

int SDL_SetRenderDrawColor(SDL_Renderer* sdl_renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
int SDL_GetRenderDrawColor(SDL_Renderer* sdl_renderer, Uint8* r, Uint8* g, Uint8* b, Uint8* a);
int SDL_RenderClear(SDL_Renderer* sdl_renderer);
int SDL_RenderFillRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int SDL_RenderFillRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);
int SDL_RenderDrawLine(SDL_Renderer* sdl_renderer, int x1, int y1, int x2, int y2);
int SDL_RenderDrawLines(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int SDL_RenderDrawPoint(SDL_Renderer* sdl_renderer, int x, int y);
int SDL_RenderDrawPoints(SDL_Renderer* sdl_renderer, const SDL_Point* points, int count);
int SDL_RenderDrawRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
int SDL_RenderDrawRects(SDL_Renderer* sdl_renderer, const SDL_Rect* rects, int count);

int SDL_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blend_mode);
int SDL_GetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode* blendMode);
int SDL_SetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode blendMode);
int SDL_GetRenderDrawBlendMode(SDL_Renderer* sdl_renderer, SDL_BlendMode *blendMode);

int SDL_SetTextureColorMod(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b);
int SDL_GetTextureColorMod(SDL_Texture* texture, Uint8* r, Uint8* g, Uint8* b);
int SDL_SetTextureAlphaMod(SDL_Texture* texture, Uint8 alpha);
int SDL_GetTextureAlphaMod(SDL_Texture* texture, Uint8* alpha);

void SDL_UnlockTexture(SDL_Texture* texture);
int SDL_LockTexture(SDL_Texture* texture, const SDL_Rect* rect, void **pixels, int *pitch);
int SDL_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blendMode);
SDL_Texture* SDL_CreateTexture(SDL_Renderer* sdl_renderer, uint32_t format, int access, int w, int h);
int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void *pixels, int pitch);
void SDL_DestroyTexture(SDL_Texture* texture);
int SDL_RenderCopy(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect);
int SDL_RenderCopyEx(SDL_Renderer* sdl_renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect, const float angle, const SDL_Point* center, const SDL_RendererFlip flip);
void SDL_RenderPresent(SDL_Renderer* sdl_renderer);

int SDL_RenderSetClipRect(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
void SDL_RenderGetClipRect(SDL_Renderer* sdl_renderer, SDL_Rect* rect);
bool SDL_RenderIsClipEnabled(SDL_Renderer* sdl_renderer);

int SDL_RenderSetViewport(SDL_Renderer* sdl_renderer, const SDL_Rect* rect);
void SDL_RenderGetViewport(SDL_Renderer* sdl_renderer, SDL_Rect* rect);

void draw_debug_images(SDL_Renderer* sdl_renderer);
void mp_draw_cube(SDL_Renderer* sdl_renderer, V3 pos, SDL_Texture* texture);
void mp_draw_cube_face(SDL_Renderer* sdl_renderer, MP_Rect_3D rect, SDL_Texture* texture);
void draw_perlin_cube(SDL_Renderer* sdl_renderer, V3 pos, float perlin);

void mp_draw_line_3d(SDL_Renderer* sdl_renderer, V3 pos_1, V3 pos_2);
void mp_draw_cube_wire_frame();

Image create_Image(SDL_Renderer* sdl_renderer, const char* file_Path);

#if 0
SDL_RenderGetWindow
SDL_GetTextureUserData
SDL_QueryTexture
SDL_RenderCopyEx
SDL_RenderSetVSync
SDL_SetTextureScaleMode
SDL_SetTextureUserData
#endif