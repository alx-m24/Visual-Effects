#pragma once
#ifndef TEXURE_H
#define TEXURE_H

#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

struct Texture
{
	unsigned int id = 0;
	std::string type;
	std::string path;
};

unsigned int loadTexture(std::string path);
unsigned int TextureFromFile(const char* path, const std::string& directory);
#endif // TEXTURE_H