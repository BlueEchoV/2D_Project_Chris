#include <stdio.h>
#include <Windows.h>
#include <stdint.h>

#include <gl/GL.h>
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81

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

// TODO: Review
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
}

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

	float red = 1;
	float blue = 1;
	float increment = 0.01;

	loadglFunctions();

	{
		uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const char* vertexShaderString = "#version 330\n"
			"layout(location = 0) in vec2 v_position;\n"
			"void main()\n"
			"{\n"
			"gl_Position = vec4(v_position.xy, 0, 1);\n"
			"}\n";

		glShaderSource(vertexShader, 1, &vertexShaderString, NULL);

		glCompileShader(vertexShader);
		
		int compileStatus = 0;

		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
		
		if (compileStatus == GL_FALSE) {
			log("Error: compileStatus variable returned false");
			int stringLength = 0;
			char infoLog[1000] = {};
			glGetShaderInfoLog(vertexShader, 1000, &stringLength, infoLog);

			return 1;
		}

		// TODO: Compile the pixel shader (Get as far as I can)
#if 0
		"#version 330\n"
			"out vec4 o_color;\n"
			"void main()\n"
			"{\n"
			"o_color = vec4(1.0, 0.5, 0.5, 1.0);\n"
			"}\n";
#endif
	}

	while (running) {
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				running = false;
			}
		}

		Sleep(1);

		red += increment;
		blue += (increment / 2);

		if (red > 1.0) {
			red = 0.0;
		}
		if (blue > 1.0) {
			blue = 0.0;
		}

		glClearColor(red, 0, blue, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if (!SwapBuffers(hdc)) {
			log("Error: SwapBuffers function returned false");
			return 1;
		}
	}

	return 0;
}