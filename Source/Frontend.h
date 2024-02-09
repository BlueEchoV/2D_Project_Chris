#pragma once
#include <vector>

struct Vec2 {
	float x, y;
};

enum ShaderType {
	layerOne = 1,
};

struct Command {
	// Position where the image should be drawn
	Vec2 position;
	// Pointer to the texture to use
	void* texture;
	// Size of the image drawn
	Vec2 size;
	// Texture coordinates
	Vec2 uvs[4];
	// Enum for the shaderType to select the shader program (Layers?)
	ShaderType shaderType;
};

void queueCommand(std::vector<Command>& commandQueue, Command& command);