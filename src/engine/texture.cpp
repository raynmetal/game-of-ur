#include <string>
#include <vector>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
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
Texture::Texture(GLuint textureID, const Usage type): mID{textureID}, mFilepath{""}, mUsage{type}
{}
Texture::Texture(): mID{0}, mFilepath{""}, mUsage{Usage::NA} 
{}

Texture::Texture(glm::vec2 dimensions, GLenum dataType, GLenum magFilter, GLenum minFilter, GLenum wrapS, GLenum wrapT) {
    glGenTextures(1, &mID);
    glBindTexture(GL_TEXTURE_2D, mID);
        glTexImage2D(
            GL_TEXTURE_2D, 0, dataType == GL_FLOAT? GL_RGBA16F: GL_RGBA, 
            dimensions.x, dimensions.y, 0, GL_RGBA, dataType, NULL
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(Texture&& other) noexcept: mID{other.mID}, mFilepath{other.mFilepath}, mUsage{other.mUsage} {
    //Prevent other from destroying this texture when its
    //deconstructor is called
    other.mID = 0;
}

Texture::Texture(const Texture& other): mFilepath {other.mFilepath}, mUsage {other.mUsage} {
    glGenTextures(1, &mID);

    copyImage(other);
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

Texture& Texture::operator=(const Texture& other) {
    if(&other == this) return *this;

    free();

    mUsage = other.mUsage;
    mFilepath = other.mFilepath;
    glGenTextures(1, &mID);

    copyImage(other);

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
    
    //TODO: flip surface along Y axis if necessary

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

    releaseResource();
}

GLuint Texture::getTextureID() const { return mID; }
Texture::Usage Texture::getUsage() const { return mUsage; }

GLint Texture::getWidth() const{
    if(!mID) return 0;
    GLint width;
    glBindTexture(GL_TEXTURE_2D, mID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glBindTexture(GL_TEXTURE_2D, 0);

    return width;
}
GLint Texture::getHeight() const {
    if(!mID) return 0;
    GLint height;
    glBindTexture(GL_TEXTURE_2D, mID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &height);
    glBindTexture(GL_TEXTURE_2D, 0);

    return height;
}

void Texture::copyImage(const Texture& other)  {
    GLint width {other.getWidth()};
    GLint height {other.getHeight()};

    // Allocate memory to our texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mID);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            mUsage == Usage::Albedo? GL_SRGB_ALPHA: GL_RGBA,
            width, height,
            0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            NULL
        );

        // Configure a few texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create temporary framebuffers which we'll use to copy other's texture data
    GLuint tempReadFBO;
    GLuint tempWriteFBO;
    GLuint tempReadRBO;
    GLuint tempWriteRBO;
    glGenFramebuffers(1, &tempReadFBO);
    glGenFramebuffers(1, &tempWriteFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempReadFBO);
        glFramebufferTexture2D(
            GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, other.mID, 0
        );
        glReadBuffer(GL_COLOR_ATTACHMENT1);

        //Create a render buffer for completeness
        glGenRenderbuffers(1, &tempReadRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, tempReadRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tempReadRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        if(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Something went wrong while creating read rbo for texture copy! " << std::endl;
        }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempWriteFBO);
        glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mID, 0
        );
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        //Create a renderbuffer for completeness
        glGenRenderbuffers(1, &tempWriteRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, tempWriteRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tempWriteRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Something went wrong while creating read rbo for texture copy! " << std::endl;
        }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Blit other's data into our colour buffer
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempWriteFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Delete temporary buffers created for this operation
    glDeleteFramebuffers(1, &tempReadFBO);
    glDeleteFramebuffers(1, &tempWriteFBO);
    glDeleteRenderbuffers(1, &tempReadRBO);
    glDeleteRenderbuffers(1, &tempWriteRBO);

    GLuint error {glGetError()};
    if(error != 0)
        std::cout << "Error while copying texture: " << glewGetErrorString(error) << std::endl;
}

void Texture::destroyResource() {
    free();
}

void Texture::releaseResource() {
    mID = 0;
    mUsage = Usage::NA;
    mFilepath = "";
}

void Texture::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, mID);
    glActiveTexture(GL_TEXTURE0);
}

void Texture::attachToFramebuffer(GLuint attachmentUnit) const {
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + attachmentUnit,
        GL_TEXTURE_2D,
        mID,
        0
    );
}
