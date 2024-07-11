#include <string>
#include <vector>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL_image.h>

#include "texture.hpp"

Texture::Texture(const std::string& filepath) : mID{0}, mFilepath{filepath} {
    bool success {
        loadFromFile(filepath)
    };
    assert(success && ("Could not load texture from " + filepath + "!").c_str());
}
Texture::Texture(): mID{0}, mFilepath{""}
{}

Texture::Texture(ColorBufferDefinition definition)
    : mColorBufferDefinition{ definition } 
{ 
    generateTexture();
}

Texture::Texture(Texture&& other) noexcept: mID{other.mID}, mFilepath{other.mFilepath}, mColorBufferDefinition{other.mColorBufferDefinition} {
    //Prevent other from destroying this texture when its
    //deconstructor is called
    other.releaseResource();
}

Texture::Texture(const Texture& other)
: mFilepath {other.mFilepath}, mColorBufferDefinition {other.mColorBufferDefinition}
{
    copyImage(other);
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if(&other == this) return *this;

    free();

    mID = other.mID;
    mFilepath = other.mFilepath;
    mColorBufferDefinition = other.mColorBufferDefinition;

    // Prevent the destruction of the texture we now own
    // when other's deconstructor is called
    other.releaseResource();

    return *this;
}

Texture& Texture::operator=(const Texture& other) {
    if(&other == this) return *this;

    free();

    mFilepath = other.mFilepath;
    mColorBufferDefinition = other.mColorBufferDefinition;
    copyImage(other);

    return *this;
}

Texture::~Texture() {
    free();
}

bool Texture::loadFromFile(const std::string& filename, const ColorBufferDefinition& colorBufferDefinition) {
    mColorBufferDefinition = colorBufferDefinition ;
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
    mColorBufferDefinition.mDimensions = {pretexture->w, pretexture->h};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0,
        //assume linear space if not an albedo texture
        internalFormat(),
        mColorBufferDefinition.mDimensions.x, mColorBufferDefinition.mDimensions.y,
        0, externalFormat(), mColorBufferDefinition.mDataType,
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mColorBufferDefinition.mWrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mColorBufferDefinition.mWrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mColorBufferDefinition.mMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mColorBufferDefinition.mMagFilter);

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

GLint Texture::getWidth() const{
    if(!mID) return 0;

    return mColorBufferDefinition.mDimensions.x;
}
GLint Texture::getHeight() const {
    if(!mID) return 0;
    return mColorBufferDefinition.mDimensions.y;
}

void Texture::copyImage(const Texture& other)  {
    // Allocate memory to our texture
    glActiveTexture(GL_TEXTURE0);
    generateTexture();

    // Create 2 temporary framebuffers which we'll use to copy other's texture data
    GLuint tempReadFBO;
    GLuint tempWriteFBO;
    glGenFramebuffers(1, &tempReadFBO);
    glGenFramebuffers(1, &tempWriteFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempReadFBO);
        glFramebufferTexture2D(
            GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, other.mID, 0
        );
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        assert(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE 
            && "Something went wrong while creating read FBO for texture copy! "
        );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempWriteFBO);
        glFramebufferTexture2D(
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mID, 0
        );
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        assert(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE 
            && "Something went wrong while creating draw FBO for texture copy! "
        );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Blit other's data into our colour buffer
    glViewport(0, 0, mColorBufferDefinition.mDimensions.x, mColorBufferDefinition.mDimensions.y);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempWriteFBO);
        glBlitFramebuffer(0, 0, 
            mColorBufferDefinition.mDimensions.x, mColorBufferDefinition.mDimensions.y, 
            0, 0,
            mColorBufferDefinition.mDimensions.x, mColorBufferDefinition.mDimensions.y, 
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Delete temporary buffers created for this operation
    glDeleteFramebuffers(1, &tempReadFBO);
    glDeleteFramebuffers(1, &tempWriteFBO);

    GLuint error {glGetError()};
    assert(error == GL_FALSE && "Error while copying texture!");
}

void Texture::destroyResource() {
    free();
}

void Texture::releaseResource() {
    if(mID)
        std::cout << "Texture " << mID << " is being released" << std::endl;
    mID = 0;
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

void Texture::generateTexture() {
    assert(mColorBufferDefinition.mDataType == GL_FLOAT || mColorBufferDefinition.mDataType == GL_UNSIGNED_BYTE);
    assert(mColorBufferDefinition.mComponentCount == 1 || mColorBufferDefinition.mComponentCount == 4);

    if(!mID) glGenTextures(1, &mID);

    glBindTexture(GL_TEXTURE_2D, mID);
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat(), 
            mColorBufferDefinition.mDimensions.x, mColorBufferDefinition.mDimensions.y, 0, externalFormat(), mColorBufferDefinition.mDataType, NULL
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mColorBufferDefinition.mMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mColorBufferDefinition.mMinFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mColorBufferDefinition.mWrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mColorBufferDefinition.mWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum Texture::internalFormat() {
    GLenum internalFormat;

    if(mColorBufferDefinition.mDataType == GL_FLOAT && mColorBufferDefinition.mComponentCount == 1){ 
        internalFormat = GL_R16F;
    } else if (mColorBufferDefinition.mDataType == GL_FLOAT && mColorBufferDefinition.mComponentCount == 4) {
        internalFormat = GL_RGBA16F;
    } else if (mColorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && mColorBufferDefinition.mComponentCount == 1) {
        internalFormat = GL_RED;
    } else if (mColorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && mColorBufferDefinition.mComponentCount == 4) {

        if(mColorBufferDefinition.mUsesWebColors) {
            internalFormat = GL_SRGB_ALPHA;
        } else {
            internalFormat = GL_RGBA;
        }

    } else {
        throw std::invalid_argument("Invalid data type and component count combination provided in texture constructor");
    }

    return internalFormat;
}

GLenum Texture::externalFormat() {
    GLenum externalFormat;

    if(mColorBufferDefinition.mDataType == GL_FLOAT && mColorBufferDefinition.mComponentCount == 1){ 
        externalFormat = GL_RED;
    } else if (mColorBufferDefinition.mDataType == GL_FLOAT && mColorBufferDefinition.mComponentCount == 4) {
        externalFormat = GL_RGBA;
    } else if (mColorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && mColorBufferDefinition.mComponentCount == 1) {
        externalFormat = GL_RED;
    } else if (mColorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && mColorBufferDefinition.mComponentCount == 4) {
        externalFormat = GL_RGBA;
    } else {
        throw std::invalid_argument("Invalid data type and component count combination provided in texture constructor");
    }

    return externalFormat;
}

