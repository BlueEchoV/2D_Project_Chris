#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>

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

typedef int64_t GLsizeiptr;

void log(const char* format, ...) {
	va_list arguments;
	const int size = 1000;

	va_start(arguments, format);

	char myArray[size] = {};

	vsnprintf(myArray, size, format, arguments);

	va_end(arguments);

	OutputDebugString(myArray);
	OutputDebugString("\n");
}

LRESULT windowProcedure(HWND windowHandle, UINT messageType, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = {};
	switch (messageType) {
	case WM_KEYDOWN: {
		log("WM_KEYDOWN: %llu", wParam);
		break;
	}

	default: {
		result = DefWindowProc(windowHandle, messageType, wParam, lParam);
	}
	}

	return result;
}

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

void loadglFunctions() {
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
}

const char* readShaderSource(const char* filePath) {
	FILE* file;
	errno_t err = fopen_s(&file, filePath, "rb");

	if (err != 0 || file == NULL) {
		log("ERROR: Shader source file did not open correctly");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = (char*)malloc(length + 1);

	if (buffer == NULL) {
		log("ERROR: Unable to allocate memory for file contents");
		fclose(file);
		return NULL;
	}

	if (length < 0) {
		log("ERROR: length is negative when reading shader source file");
		return NULL;
	}

	size_t readSize = fread(buffer, 1, length, file);
	if (readSize < (size_t)length) {
		log("ERROR: Could not read entire file.");
		free(buffer);
		fclose(file);
		return NULL;
	}

	buffer[length] = '\0';

	fclose(file);

	return buffer;
}

uint32_t compileShader(GLenum shaderType, const char* shaderSource)
{
	uint32_t shader = glCreateShader(shaderType);

	glShaderSource(shader, 1, &shaderSource, NULL);

	glCompileShader(shader);

	int compileStatus = 0;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

	if (compileStatus == GL_FALSE) {
		log("Error: compileStatus variable returned false (compileShader)");
		int stringLength = 0;
		char infoLog[1000] = {};
		glGetShaderInfoLog(shader, 1000, &stringLength, infoLog);

		glDeleteShader(shader);

		return 0;
	}

	return shader;
}

struct Vec2 {
	float x, y;
};

struct Color {
	uint8_t r, g, b, a;
};

struct Vertex {
	Vec2 position;
	Color color;
};

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

	HDC hdc = GetDC(window);
	int pixelFormatIndex = ChoosePixelFormat(hdc, &pfd);

	if (pixelFormatIndex == 0) {
		log("Error: ChoosePixelFormat function returned 0");
		return 1;
	}

	if (!SetPixelFormat(hdc, pixelFormatIndex, &pfd)) {
		log("Error: SetPixelFormat function returned false");
		return 1;
	}

	HGLRC glRendereringContext = wglCreateContext(hdc);

	if (glRendereringContext == NULL) {
		log("Error: wglCreateContext function returned false");
		return 1;
	}

	if (!wglMakeCurrent(hdc, glRendereringContext)) {
		log("Error: wglMakeCurrent function returned false");
		return 1;
	}

	loadglFunctions();

	const char* vertexShaderString = readShaderSource("C:\\Projects\\2D_Project_Chris\\Shaders\\vertexShader.txt");

	const char* fragmentShaderString = readShaderSource("C:\\Projects\\2D_Project_Chris\\Shaders\\fragmentShader.txt");

	if (vertexShaderString == NULL || fragmentShaderString == NULL) {
		log("ERROR: shaderString returned NULL");
		return 1;
	}

	uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderString);

	uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderString);

	if (vertexShader == 0 || fragmentShader == 0) {
		log("Error: vertexShader or fragmentShader result == 0");
		return 1;
	}

	uint32_t shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int linkStatus = 0;

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE)
	{
		log("Error: linkStatus variable returned false");
		int stringLength = 0;
		char infoLog[1000] = {};
		glGetProgramInfoLog(shaderProgram, 1000, &stringLength, infoLog);

		return 1;
	}

	// Attached shaders will only be deleted when they are no longer attached
	// to a object. They will be flagged for deletion and deleted when they 
	// are no longer attached.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);

	// The buffer must be bound before we call this
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	// Have the square share the same points
	Vertex vertices[6];

	// First triangle
	vertices[0].position = {-0.5f, 0.5f};
	vertices[0].color = { 255, 0, 0, 255 };

	vertices[1].position = { 0.5f, -0.5f };
	vertices[1].color = { 0, 0, 255, 255 };

	vertices[2].position = { -0.5f, -0.5f };
	vertices[2].color = { 255, 0, 0, 255 };
	
	// Second triangle
	vertices[3].position = { -0.5f, 0.5f };
	vertices[3].color = { 255, 0, 0, 255 };

	vertices[4].position = { 0.5f, -0.5f };
	vertices[4].color = { 0, 0, 255, 255 };

	vertices[5].position = { 0.5f, 0.5f };
	vertices[5].color = { 0, 0, 255, 255 };

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	MSG msg = {};

	bool running = true;

	while (running) {
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
		}

		Sleep(1);

		RECT rect = {};
		GetClientRect(window, &rect);
		glViewport(0, 0, rect.right, rect.bottom);

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glUseProgram(shaderProgram);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (!SwapBuffers(hdc)) {
			log("Error: SwapBuffers function returned false");
			return 1;
		}
	}

	free((void*)vertexShaderString);
	free((void*)fragmentShaderString);
	glDeleteProgram(shaderProgram);

	return 0;
}