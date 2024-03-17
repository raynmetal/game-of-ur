#include <string>
#include <vector>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_image.h>

#include "texture.hpp"

Texture::Texture(const std::string& filepath, const Usage type) : mID{0}, mFilepath{filepath}, mUsage{type} {
    bool success {
        loadFromFile(filepath)
    };
    if(!success) {
        std::cout << "Could not load texture from " << filepath
            << "!" << std::endl;
    } else std::cout << "Texture at " << filepath << " loaded successfully!"
        << std::endl;
}
Texture::Texture(GLuint textureID, const Usage type):mID{textureID}, mFilepath{""}, mUsage{type}
{}
Texture::Texture(): mID{0}, mFilepath{""}, mUsage{Usage::NA} 
{}

Texture::Texture(const Texture& other): mID{other.mID}, mFilepath{other.mFilepath}, mUsage{other.mUsage}
{}
Texture::Texture(Texture&& other) noexcept: mID{other.mID}, mFilepath{other.mFilepath}, mUsage{other.mUsage} {
    //Prevent other from destroying this texture when its
    //deconstructor is called
    other.mID = 0;
}

Texture& Texture::operator=(const Texture& other) {
    if(&other == this) return *this;

    // free currently held resource
    free();

    // copy the other's resource
    mID = other.mID;
    mFilepath = other.mFilepath;
    mUsage = other.mUsage;

    return *this;
}
Texture& Texture::operator=(Texture&& other) noexcept {
    if(&other == this) return *this;

    free();

    mID = other.mID;
    mFilepath = other.mFilepath;
    mUsage = other.mUsage;

    // Prevent the destruction of the texture we now own
    // when other's deconstructor is called
    other.mID = 0;

    return *this;
}

Texture::~Texture() {
    free();
}


bool Texture::loadFromFile(const std::string& filename) {
    free();

    // Load image from file into a convenient SDL surface, per the image
    SDL_Surface* textureImage { IMG_Load(filename.c_str()) };
    if(!textureImage) {
        std::cout << "Could not load texture!\n"
            << IMG_GetError() << std::endl;
        return false;
    }

    // Convert the image from its present format to RGBA
    SDL_Surface* pretexture { SDL_ConvertSurfaceFormat(textureImage, SDL_PIXELFORMAT_RGBA32, 0) };
    SDL_FreeSurface(textureImage);
    textureImage = nullptr;
    if(!pretexture) {
        std::cout << "Something went wrong: " << SDL_GetError() << std::endl;
        return false;
    }
    
    //TODO: flip surface if necessary

    // Move surface pixels to the graphics card
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0,
        //assume linear space if not an albedo texture
        mUsage == Usage::Albedo? GL_SRGB_ALPHA: GL_RGBA,
        pretexture->w, pretexture->h,
        0, GL_RGBA, GL_UNSIGNED_BYTE,
        reinterpret_cast<void*>(pretexture->pixels)
    );
    SDL_FreeSurface(pretexture);
    pretexture=nullptr;

    // Check for OpenGL errors
    GLuint error { glGetError() };
    if(error != GL_NO_ERROR) {
        std::cout << "Could not convert image to OpenGL texture!\n"
            << glewGetErrorString(error) << std::endl;
        glDeleteTextures(1, &texture);
        return false;
    }

    // Configure a few texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // store the id of loaded texture in mID, and return success
    mID = texture;
    return true;
}

void Texture::free() {
    if(!mID) return;

    std::cout << "Texture " << mID << " is being freed" << std::endl;
    glDeleteTextures(1, &mID);
    mID = 0;
    mUsage = Usage::NA;
    mFilepath = "";
}


void Texture::bind() const {
    if(!mID) return;
    glBindTexture(GL_TEXTURE_2D, mID);
}

GLuint Texture::getTextureID() const { return mID; }
Texture::Usage Texture::getUsage() const { return mUsage; }
