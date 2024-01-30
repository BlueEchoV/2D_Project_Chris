#include <stdio.h>
#include <Windows.h>
#include <stdint.h>

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

	MSG msg = {};

	bool running = true;

	loadglFunctions();

	const char* vertexShaderString = "#version 330\n"
		"layout(location = 0) in vec2 v_position;\n"
		"layout(location = 1) in vec4 v_color;\n"
		"out vec4 f_color;\n"
		"void main()\n"
		"{\n"
		"f_color = v_color;\n"
		"gl_Position = vec4(v_position.xy, 0, 1);\n"
		"}\n";

	const char* fragmentShaderString = "#version 330\n"
		"out vec4 o_color;\n"
		"in vec4 f_color;\n"
		"void main()\n"
		"{\n"
		"o_color = f_color;\n"
		"}\n";

	uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderString);

	uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderString);

	if (vertexShader == 0 || fragmentShader == 0) {
		// TODO LOG
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

	Vertex vertices[3];

	vertices[0].position.x = -0.5;
	vertices[0].position.y = 0.5;
	vertices[0].color = { 255, 0, 0, 255 };

	vertices[1].position.x = 0.2;
	vertices[1].position.y = 0;
	vertices[1].color = { 0, 255, 0, 255 };

	vertices[2].position.x = -0.5;
	vertices[2].position.y = -0.5;
	vertices[2].color = { 0, 0, 255, 255 };

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

		glDrawArrays(GL_TRIANGLES, 0, 3);

		if (!SwapBuffers(hdc)) {
			log("Error: SwapBuffers function returned false");
			return 1;
		}
	}

	glDeleteProgram(shaderProgram);

	return 0;
}