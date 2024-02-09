#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <errno.h>
#include <string.h>

#include "Backend.h"

#define arraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

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
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

struct MouseClick {
	bool clicked = false;
	Vec2 position = {};
};

MouseClick lastMouseClick;

std::vector<Command> commandQueue;

void processCommands() {
	// Sort commands based of the layer being drawn
	// Sort(commandQueue);

	/*
	for (const auto& command : commandQueue) {
		glUseProgram(command.shaderType);
	}
	*/

	// comandQueue.clear();
}

GLuint screenWidthLocation = {};
GLuint screenHeightLocation = {};

typedef int64_t GLsizeiptr;

/**********************************************
 *
 * Defer
 *
 ***************/

template <typename T>
struct ExitScope
{
	T lambda;
	ExitScope(T lambda) : lambda(lambda) { }
	~ExitScope() { lambda(); }
	// NO_COPY(ExitScope);
};

struct ExitScopeHelp
{
	template<typename T>
	ExitScope<T> operator+(T t) { return t; }
};

#define _SG_CONCAT(a, b) a ## b
#define SG_CONCAT(a, b) _SG_CONCAT(a, b)
#define DEFER auto SG_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

struct ShaderProgramData {
	uint32_t shaderProgramID;
	time_t vertexShaderLastModTime;
	time_t fragmentShaderLastModTime;
};

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
	case WM_LBUTTONDOWN: {
		lastMouseClick.position.x = static_cast<float>LOWORD(lParam);
		lastMouseClick.position.y = static_cast<float>HIWORD(lParam);
		lastMouseClick.clicked = true;
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

typedef GLuint(*glGetUniformLocationFunc)(GLuint program, const GLchar* name);
glGetUniformLocationFunc glGetUniformLocation = {};

typedef void(*glUniform1fFunc)(GLint location,GLfloat v0);
glUniform1fFunc glUniform1f = {};

typedef HGLRC(WINAPI* wglCreateContextAttribsARBFunc) (HDC hDC, HGLRC hShareContext, const int* attribList);
wglCreateContextAttribsARBFunc wglCreateContextAttribsARB = {};

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
	glBindVertexArray = (glBindVertexArrayFunc)wglGetProcAddress("glBindVertexArray");
	glBindBuffer = (glBindBufferFunc)wglGetProcAddress("glBindBuffer");
	glBufferData = (glBufferDataFunc)wglGetProcAddress("glBufferData");

	glGetUniformLocation = (glGetUniformLocationFunc)wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (glUniform1fFunc)wglGetProcAddress("glUniform1f");

	wglCreateContextAttribsARB = (wglCreateContextAttribsARBFunc)wglGetProcAddress("wglCreateContextAttribsARB");
}

const char* readShaderSource(const char* filePath) {
	FILE* file;
	errno_t err = fopen_s(&file, filePath, "rb");

	if (err != 0 || file == NULL) {
		log("ERROR: Shader source file did not open correctly");
		return NULL;
	}

	DEFER{
		fclose(file);
	};

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (length < 0) {
		log("ERROR: length is negative when reading shader source file");
		return NULL;
	}

	char* buffer = (char*)malloc(length + 1);

	if (buffer == NULL) {
		log("ERROR: Unable to allocate memory for file contents");
		return NULL;
	}

	size_t readSize = fread(buffer, 1, length, file);
	if (readSize < (size_t)length) {
		log("ERROR: Could not read entire file.");
		free(buffer);
		return NULL;
	}

	buffer[length] = '\0';

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
		log("ERROR: compileStatus variable returned false (compileShader)");
		int stringLength = 0;
		char infoLog[1000] = {};
		glGetShaderInfoLog(shader, 1000, &stringLength, infoLog);
		
		const char* shaderTypeName = "Shader Type Unknown";
		if (shaderType == GL_VERTEX_SHADER) {
			shaderTypeName = "Vertex Shader";
		}
		else if (shaderType == GL_FRAGMENT_SHADER) {
			shaderTypeName = "Fragment Shader";
		}

		MessageBox(NULL, infoLog, shaderTypeName, MB_OK);

		glDeleteShader(shader);

		return 0;
	}

	return shader;
}

struct Color {
	uint8_t r, g, b, a;
};

struct Vertex {
	Vec2 position;
	Color color;
	Vec2 uv;
};

uint32_t loadShaderProgram(const char* vertexShaderFilePath, const char* fragmentShaderFilePath) {
	const char* vertexShaderString = readShaderSource(vertexShaderFilePath);
	DEFER{
		free((void*)vertexShaderString);
	};
	const char* fragmentShaderString = readShaderSource(fragmentShaderFilePath);
	DEFER{
		free((void*)fragmentShaderString);
	};

	if (vertexShaderString == NULL || fragmentShaderString == NULL) {
		log("ERROR: vertexShaderString or fragmentShaderString == NULL");
		return 0;
	}

	uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderString);
	uint32_t fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderString);

	DEFER{
		if (vertexShader != 0) {
			glDeleteShader(vertexShader);
		}
		if (fragmentShader != 0) {
			glDeleteShader(fragmentShader);
		}
	};

	if (vertexShader == 0 || fragmentShader == 0) {
		log("ERROR: vertexShader or fragmentShader didn't compile");
		return 0;
	}

	uint32_t shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int linkStatus = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
	{
		log("ERROR: linkStatus variable returned false");
		int stringLength = 0;
		char infoLog[1000] = {};
		glGetProgramInfoLog(shaderProgram, 1000, &stringLength, infoLog);
		glDeleteProgram(shaderProgram);
		return 0;
	}

	return shaderProgram;
}

time_t getLastChangedFileTime(const char* filePath) {
	struct stat fileAttrib;
	if (stat(filePath, &fileAttrib) < 0) {
		char errorMessage[256];

		strerror_s(errorMessage, sizeof(errorMessage), errno);
		log("ERROR: stat failed for %s, Error: %s", filePath, errorMessage);
	}

	return fileAttrib.st_mtime;
}

GLuint loadTexture(const char* filePath) {
	stbi_set_flip_vertically_on_load(true);

	int width, height, channels;
	unsigned char* imageData = stbi_load(filePath, &width, &height, &channels, 4);
	if (!imageData) {
		log("ERROR: stbi_load failed");
		return 0;
	}
	DEFER{
		stbi_image_free(imageData);
	};

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
		height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	return textureID;
}

bool haveShadersChanged(const char* vertexFilePath, const char* fragmentFilePath, ShaderProgramData& shaderProgramData) {
	time_t currentVertexTime = getLastChangedFileTime(vertexFilePath);
	time_t currentFragmentTime = getLastChangedFileTime(fragmentFilePath);
	
	if (currentVertexTime != shaderProgramData.vertexShaderLastModTime 
		|| currentFragmentTime != shaderProgramData.fragmentShaderLastModTime) {
		shaderProgramData.vertexShaderLastModTime = currentVertexTime;
		shaderProgramData.fragmentShaderLastModTime = currentFragmentTime;
		return true;
	}
	
	return false;
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

	HGLRC temp_context = wglCreateContext(hdc);
	wglMakeCurrent(hdc, temp_context);
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

	HGLRC glRendereringContext = wglCreateContextAttribsARB(hdc, 0, attrib_list);
	wglMakeCurrent(hdc, glRendereringContext);
	wglDeleteContext(temp_context);

	if (glRendereringContext == NULL) {
		log("Error: wglCreateContext function returned false");
		return 1;
	}

	const char* vertexShaderFilePath = "Shaders\\vertexShader.txt";

	const char* fragmentShaderFilePath = "Shaders\\fragmentShader.txt";
	
	ShaderProgramData shaderProgramData = {};

	GLuint vaoOne;
	glGenVertexArrays(1, &vaoOne);
	glBindVertexArray(vaoOne);

	GLuint vboOne;
	glGenBuffers(1, &vboOne);
	glBindBuffer(GL_ARRAY_BUFFER, vboOne);

	Vertex verticesImage1[6];

	// Center of the window
	float centerX = 500;
	float centerY = 500;

	float halfSize = 200.0f;

	// Origin is the top left

	// First triangle
	verticesImage1[0].position = { centerX - halfSize, centerY + halfSize };
	verticesImage1[0].color = { 255, 0, 0, 255 };
	verticesImage1[0].uv = { 0.0f, 0.0f };

	verticesImage1[1].position = { centerX + halfSize, centerY - halfSize };
	verticesImage1[1].color = { 0, 0, 255, 255 };
	verticesImage1[1].uv = { 1.0f, 1.0f };

	verticesImage1[2].position = { centerX - halfSize, centerY - halfSize };
	verticesImage1[2].color = { 255, 0, 0, 255 };
	verticesImage1[2].uv = { 0.0f, 1.0f };

	// Second triangle
	verticesImage1[3].position = { centerX - halfSize, centerY + halfSize };
	verticesImage1[3].color = { 255, 0, 0, 255 };
	verticesImage1[3].uv = { 0.0f, 0.0f };

	verticesImage1[4].position = { centerX + halfSize, centerY - halfSize };
	verticesImage1[4].color = { 0, 0, 255, 255 };
	verticesImage1[4].uv = { 1.0f, 1.0f };

	verticesImage1[5].position = { centerX + halfSize, centerY + halfSize };
	verticesImage1[5].color = { 0, 0, 255, 255 };
	verticesImage1[5].uv = { 1.0f, 0.0f };

	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesImage1), verticesImage1, GL_STATIC_DRAW);

	GLuint vboTwo;
	glGenBuffers(1, &vboTwo);
	glBindBuffer(GL_ARRAY_BUFFER, vboTwo);

	Vertex verticesImage2[6];

	// First triangle
	verticesImage2[0].position = { centerX - halfSize, centerY + halfSize };
	verticesImage2[0].color = { 255, 0, 0, 255 };
	verticesImage2[0].uv = { 0.0f, 0.0f };
				 
	verticesImage2[1].position = { centerX + halfSize, centerY - halfSize };
	verticesImage2[1].color = { 0, 0, 255, 255 };
	verticesImage2[1].uv = { 1.0f, 1.0f };
				 
	verticesImage2[2].position = { centerX - halfSize, centerY - halfSize };
	verticesImage2[2].color = { 255, 0, 0, 255 };
	verticesImage2[2].uv = { 0.0f, 1.0f };
				 
	// Second triangle
	verticesImage2[3].position = { centerX - halfSize, centerY + halfSize };
	verticesImage2[3].color = { 255, 0, 0, 255 };
	verticesImage2[3].uv = { 0.0f, 0.0f };
				 
	verticesImage2[4].position = { centerX + halfSize, centerY - halfSize };
	verticesImage2[4].color = { 0, 0, 255, 255 };
	verticesImage2[4].uv = { 1.0f, 1.0f };
				 
	verticesImage2[5].position = { centerX + halfSize, centerY + halfSize };
	verticesImage2[5].color = { 0, 0, 255, 255 };
	verticesImage2[5].uv = { 1.0f, 0.0f };

	float offsetX = halfSize * 2;

	for (int i = 0; i < arraySize(verticesImage2); i++) {
		verticesImage2[i].position.x += offsetX;
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesImage2), verticesImage2, GL_STATIC_DRAW);

	MSG msg = {};

	bool running = true;

	GLuint textureAzir = loadTexture("assets/azir.jpg");
	GLuint textureSmolder = loadTexture("assets/smolder.jpg");

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

		if (haveShadersChanged(vertexShaderFilePath, fragmentShaderFilePath, shaderProgramData)) {
			const int maxAttempts = 5;
			const int attemptDelay = 1000;
			int currentAttempts = 0;

			while (currentAttempts < maxAttempts) {
				uint32_t newShaderProgram = loadShaderProgram(vertexShaderFilePath, fragmentShaderFilePath);

				if (newShaderProgram == 0) {
					log("ERROR: shaderProgram returned 0. Potential lock on file");
					currentAttempts++;
					Sleep(attemptDelay);
					continue;
				}
				else {
					if (shaderProgramData.shaderProgramID != 0) {
						glDeleteProgram(shaderProgramData.shaderProgramID);
					}
					shaderProgramData.shaderProgramID = newShaderProgram;
					glUseProgram(shaderProgramData.shaderProgramID);

					screenWidthLocation = glGetUniformLocation(shaderProgramData.shaderProgramID, "screenWidth");
					screenHeightLocation = glGetUniformLocation(shaderProgramData.shaderProgramID, "screenHeight");
					break;
				}
			}

			if (currentAttempts >= maxAttempts) {
				log("Error: currentAttempts >= maxAttempts. Issue with hotloading.");
				return 1;
			}
		}

		if (screenWidthLocation >= 0) {
			glUniform1f(screenWidthLocation, (GLfloat)rect.right);
		}
		if (screenHeightLocation >= 0) {
			glUniform1f(screenHeightLocation, (GLfloat)rect.bottom);
		}

		if (lastMouseClick.clicked) {
			log("Left mouse button was clicked at: { %f, %f }", 
				lastMouseClick.position.x, lastMouseClick.position.y);

			Command command = {};
			command.position = lastMouseClick.position;
			command.texture = &textureAzir;
			command.size = { halfSize * 2.0f, halfSize * 2.0f };
			
			command.uvs[0] = { 0.0f, 0.0f };
			command.uvs[1] = { 1.0f, 0.0f };
			command.uvs[2] = { 0.0f, 1.0f };
			command.uvs[3] = { 1.0f, 1.0f };
			
			command.shaderType = layerOne;
			
			commandQueue.push_back(command);

			lastMouseClick.clicked = false;
		}

		glViewport(0, 0, rect.right, rect.bottom);

		glBindBuffer(GL_ARRAY_BUFFER, vboOne);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindTexture(GL_TEXTURE_2D, textureAzir);
		glDrawArrays(GL_TRIANGLES, 0, arraySize(verticesImage1));

		glBindBuffer(GL_ARRAY_BUFFER, vboTwo);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindTexture(GL_TEXTURE_2D, textureSmolder);
		glDrawArrays(GL_TRIANGLES, 0, arraySize(verticesImage2));

		if (!SwapBuffers(hdc)) {
			log("Error: SwapBuffers function returned false");
			return 1;
		}
	}

	glDeleteProgram(shaderProgramData.shaderProgramID);

	return 0;
}