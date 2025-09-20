#pragma once
#ifndef TEXURE_H
#define TEXURE_H

#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <unordered_map>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace fs = std::filesystem;

struct Texture
{
	unsigned int id = 0;
	std::string type;
	std::string path;
};

unsigned int loadTexture(std::string path);
unsigned int TextureFromFile(const char* path, const std::string& directory);

inline unsigned int TextureFromMemory(const unsigned char* data, size_t size) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // if needed
    unsigned char* image = stbi_load_from_memory(data, static_cast<int>(size),
        &width, &height, &nrChannels, 0);
    if (!image) {
        std::cerr << "Failed to load texture from memory\n";
        return 0;
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
        format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(image);
    return textureID;
}
inline unsigned int TextureFromRawPixels(aiTexel* pixels, int width, int height) {
    // aiTexel is always 4 bytes: BGRA (0-255)
    std::vector<unsigned char> data(width * height * 4);

    for (int i = 0; i < width * height; i++) {
        data[4 * i + 0] = pixels[i].r;
        data[4 * i + 1] = pixels[i].g;
        data[4 * i + 2] = pixels[i].b;
        data[4 * i + 3] = pixels[i].a;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

#endif // TEXTURE_H