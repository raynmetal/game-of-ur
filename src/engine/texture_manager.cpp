#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "texture_manager.hpp"


TextureManager& TextureManager::getInstance() {
    static TextureManager textureManager{};
    return textureManager;
}
TextureManager::~TextureManager() {
    mReferenceCounts.clear();
    mTextures.clear();
}


TextureManager::TextureHandle TextureManager::getTexture(const std::string& nameOrFilepath) {
    if(mTextures.find(nameOrFilepath) == mTextures.end()) {
        std::cout << "No texture " << nameOrFilepath << " exists." << std::endl;
        throw std::runtime_error("Texture does not exist");
    }

    return {nameOrFilepath, mTextures[nameOrFilepath].getTextureID()};
}

TextureManager::TextureHandle TextureManager::getFileTexture(const std::string& filepath, Texture::Usage usage) {
    if(mTextures.find(filepath) != mTextures.end()) {
        return {filepath, mTextures[filepath].getTextureID()};
    }

    Texture fileTexture { filepath, usage };

    if(!fileTexture.getTextureID()) {
        std::cerr << "no file " << filepath << " exists."  << std::endl;
        throw std::runtime_error("File does not exist");
    }

    mTextures[filepath] = std::move(fileTexture);

    return { filepath, mTextures[filepath].getTextureID() };
}

TextureManager::TextureHandle TextureManager::getGeneratedTexture(const std::string& name, glm::vec2 dimensions, GLenum dataType, GLenum magFilter, GLenum minFilter, GLenum wrapS, GLenum wrapT) {
    if(mTextures.find(name) != mTextures.end()) {
        return {name, mTextures[name].getTextureID()};
    }

    GLenum colorFormat;
    if(dataType == GL_FLOAT) colorFormat = GL_RGBA16F;
    else {
        colorFormat = GL_RGBA;
        dataType = GL_UNSIGNED_BYTE;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, dimensions.x, dimensions.y, 0, GL_RGBA, dataType, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error { glGetError() };
    if(error) {
        std::cerr << "An error occurred while creating texture " << name << "." << std::endl;
        std::cerr << error << ": " << glewGetErrorString(error) << std::endl;
        throw std::runtime_error("Could not create texture");
    }

    mTextures[name] = {textureID, Texture::NA};
    return { name, mTextures[name].getTextureID() };
}

void TextureManager::incrementReferenceCount(const std::string& name) {
    if(name == "") return;

    if(mReferenceCounts.find(name) == mReferenceCounts.end()) {
        mReferenceCounts[name] = 0;
    }

    ++mReferenceCounts[name];
}

void TextureManager::decrementReferenceCount(const std::string& name) {
    if(name == "") return;

    if(mReferenceCounts.find(name) == mReferenceCounts.end()) return;

    --mReferenceCounts[name];
}

void TextureManager::removeUnusedTextures() {
    // Make a list of unused textures
    std::vector<std::string> texturesToRemove {};
    for(std::pair<const std::string&, GLuint> refCountPair : mReferenceCounts) {
        if(!refCountPair.second)  {
            texturesToRemove.push_back(refCountPair.first);
        }
    }

    // Remove them
    for(std::string texture: texturesToRemove) {
        mReferenceCounts.erase(texture);
        mTextures.erase(texture);
    }
}


std::string TextureManager::TextureHandle::getName() const {
    return mName;
}

void TextureManager::TextureHandle::bind(GLuint textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, mGLHandle);
    glActiveTexture(GL_TEXTURE0);
}

void TextureManager::TextureHandle::attachToFramebuffer(GLuint attachmentUnit) const {
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentUnit,
        GL_TEXTURE_2D, mGLHandle, 0
    );
}

TextureManager::TextureHandle::TextureHandle(const std::string& name, GLuint glHandle): mName{name}, mGLHandle{glHandle} {
    TextureManager::getInstance().incrementReferenceCount(mName);
}

TextureManager::TextureHandle::~TextureHandle(){
    TextureManager::getInstance().decrementReferenceCount(mName);
}

TextureManager::TextureHandle::TextureHandle(const TextureHandle& other): TextureHandle{other.mName, other.mGLHandle} {}

TextureManager::TextureHandle::TextureHandle(TextureHandle&& other) {
    mName = other.mName;
    mGLHandle = other.mGLHandle;
    other.mName = "";
    other.mGLHandle = 0;
}

TextureManager::TextureHandle& TextureManager::TextureHandle::operator=(const TextureHandle& other) {
    if(&other == this) return *this;
    mName = other.mName;
    mGLHandle = other.mGLHandle;
    TextureManager::getInstance().incrementReferenceCount(mName);

    return *this;
}

TextureManager::TextureHandle& TextureManager::TextureHandle::operator=(TextureHandle&& other) {
    if(&other == this) return *this;
    mName = other.mName;
    mGLHandle = other.mGLHandle;
    other.mName = "";
    other.mGLHandle = 0;

    return *this;
}

bool TextureManager::TextureHandle::operator==(const TextureHandle& other) const {
    return mName == other.mName;
}
