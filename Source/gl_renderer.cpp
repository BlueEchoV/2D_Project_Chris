#include "gl_renderer.h"

glCreateShaderFunc glCreateShader = nullptr;
glShaderSourceFunc glShaderSource = nullptr;
glCompileShaderFunc glCompileShader = nullptr;
glGetShaderivFunc glGetShaderiv = nullptr;
glGetShaderInfoLogFunc glGetShaderInfoLog = nullptr;
glCreateProgramFunc glCreateProgram = nullptr;
glDeleteProgramFunc glDeleteProgram = nullptr;
glAttachShaderFunc glAttachShader = nullptr;
glLinkProgramFunc glLinkProgram = nullptr;
glGetProgramivFunc glGetProgramiv = nullptr;
glGetProgramInfoLogFunc glGetProgramInfoLog = nullptr;
glDetachShaderFunc glDetachShader = nullptr;
glDeleteShaderFunc glDeleteShader = nullptr;
glUseProgramFunc glUseProgram = nullptr;
glGenVertexArraysFunc glGenVertexArrays = nullptr;
glGenBuffersFunc glGenBuffers = nullptr;
glVertexAttribPointerFunc glVertexAttribPointer = nullptr;
glEnableVertexAttribArrayFunc glEnableVertexAttribArray = nullptr;
glDisableVertexAttribArrayFunc glDisableVertexAttribArray = nullptr;
glBindVertexArrayFunc glBindVertexArray = nullptr;
glBindBufferFunc glBindBuffer = nullptr;
glBufferDataFunc glBufferData = nullptr;
glDeleteBuffersFunc glDeleteBuffers = nullptr;
glGetUniformLocationFunc glGetUniformLocation = nullptr;
glUniform1fFunc glUniform1f = nullptr;
glUniformMatrix4fvFunc glUniformMatrix4fv = nullptr;
wglCreateContextAttribsARBFunc wglCreateContextAttribsARB = nullptr;
glActiveTextureFunc glActiveTexture = nullptr;
glUniform1iFunc glUniform1i = nullptr;
glBlendEquationFunc glBlendEquation = nullptr;
glBufferSubDataFunc glBufferSubData = nullptr;

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
	glBufferSubData = (glBufferSubDataFunc)wglGetProcAddress("glBufferSubData");
	glDeleteBuffers = (glDeleteBuffersFunc)wglGetProcAddress("glDeleteBuffers");

	glGetUniformLocation = (glGetUniformLocationFunc)wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (glUniform1fFunc)wglGetProcAddress("glUniform1f");
	glUniform1i = (glUniform1iFunc)wglGetProcAddress("glUniform1i");
	glUniformMatrix4fv = (glUniformMatrix4fvFunc)wglGetProcAddress("glUniformMatrix4fv");

	wglCreateContextAttribsARB = (wglCreateContextAttribsARBFunc)wglGetProcAddress("wglCreateContextAttribsARB");

	glActiveTexture = (glActiveTextureFunc)wglGetProcAddress("glActiveTexture");
}

GLuint create_shader(const std::string shader_file_path, GLenum shader_type) {
	std::ifstream file(shader_file_path);
	if (!file.is_open()) {
		log("ERROR: shader file did not open");
		assert(false);
	}

	std::stringstream shader_source;
	shader_source << file.rdbuf();
	file.close();

	std::string shader_source_string = shader_source.str();
	const char* shader_source_cstr = shader_source_string.c_str();

	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_source_cstr, NULL);
	glCompileShader(shader);
	GLint success;
	char info_Log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, info_Log);
		log("ERROR: shader compilation failed: %s", info_Log);
		assert(false);
	}

	return shader;
}

GLuint create_shader_program(const char* vertex_shader_file_path, const char* fragment_shader_file_path) {
	GLuint result;
	result = glCreateProgram();
	GLuint vertex_shader = create_shader(vertex_shader_file_path, GL_VERTEX_SHADER);
	GLuint fragment_shader = create_shader(fragment_shader_file_path, GL_FRAGMENT_SHADER);

	glAttachShader(result, vertex_shader);
	glAttachShader(result, fragment_shader);
	glLinkProgram(result);
	GLint success;
	glGetProgramiv(result, GL_LINK_STATUS, &success);
	if (!success) {
		assert(false);
	}

	// No longer needed
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	return result;
}

GLuint shader_program_types[SPT_TOTAL] = { 0 };
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

	const char* v_string_file_path = "Shaders\\v_string.txt";
	const char* f_string_file_path = "Shaders\\f_string.txt";
	GLuint string_shader = create_shader_program(v_string_file_path, f_string_file_path);
	shader_program_types[SPT_String] = string_shader;

	const char* v_fireball_file_path = "Shaders\\v_fireball.txt";
	const char* f_fireball_file_path = "Shaders\\f_fireball.txt";
	GLuint fireball_shader = create_shader_program(v_fireball_file_path, f_fireball_file_path);
	shader_program_types[SPT_Fireball] = fireball_shader;

	const char* v_cube_map_file_path = "Shaders\\v_cube_map.txt";
	const char* f_cube_map_file_path = "Shaders\\f_cube_map.txt";
	GLuint cube_map_shader = create_shader_program(v_cube_map_file_path, f_cube_map_file_path);
	shader_program_types[SPT_Cube_Map] = cube_map_shader;
}

