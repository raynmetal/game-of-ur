#include <string>
#include <vector>
#include <iostream>
#include <map>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SDL2/SDL_image.h>

#include "texture.hpp"

const std::map<std::string, GLenum> kStringToFilter {
    {"linear", GL_LINEAR},
    {"nearest", GL_NEAREST},
};

const std::map<std::string, GLenum> kStringToWrap {
    { "clamp-border", GL_CLAMP_TO_BORDER },
    { "clamp-edge", GL_CLAMP_TO_EDGE },
    { "repeat", GL_REPEAT },
    { "repeat-mirrored", GL_MIRRORED_REPEAT },
};

const std::map<GLenum, std::string> kFilterToString {
    {GL_LINEAR, "linear"},
    {GL_NEAREST, "nearest"},
};

const std::map<GLenum, std::string> kWrapToString {
    {GL_CLAMP_TO_BORDER, "clamp-border"},
    {GL_CLAMP_TO_EDGE, "clamp-edge"},
    {GL_REPEAT, "repeat"},
    {GL_MIRRORED_REPEAT, "repeat-mirrored"},
};

GLenum deduceInternalFormat(const ColorBufferDefinition& colorBufferDefinition) {
    GLenum internalFormat;

    if(colorBufferDefinition.mDataType == GL_FLOAT && colorBufferDefinition.mComponentCount == 1){ 
        internalFormat = GL_R16F;
    } else if (colorBufferDefinition.mDataType == GL_FLOAT && colorBufferDefinition.mComponentCount == 4) {
        internalFormat = GL_RGBA16F;
    } else if (colorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && colorBufferDefinition.mComponentCount == 1) {
        internalFormat = GL_RED;
    } else if (colorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && colorBufferDefinition.mComponentCount == 4) {

        if(colorBufferDefinition.mUsesWebColors) {
            internalFormat = GL_SRGB_ALPHA;
        } else {
            internalFormat = GL_RGBA;
        }

    } else {
        throw std::invalid_argument("Invalid data type and component count combination provided in texture constructor");
    }

    return internalFormat;
}

GLenum deduceExternalFormat(const ColorBufferDefinition& colorBufferDefinition) {
    GLenum externalFormat;

    if(colorBufferDefinition.mDataType == GL_FLOAT && colorBufferDefinition.mComponentCount == 1){ 
        externalFormat = GL_RED;
    } else if (colorBufferDefinition.mDataType == GL_FLOAT && colorBufferDefinition.mComponentCount == 4) {
        externalFormat = GL_RGBA;
    } else if (colorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && colorBufferDefinition.mComponentCount == 1) {
        externalFormat = GL_RED;
    } else if (colorBufferDefinition.mDataType == GL_UNSIGNED_BYTE && colorBufferDefinition.mComponentCount == 4) {
        externalFormat = GL_RGBA;
    } else {
        throw std::invalid_argument("Invalid data type and component count combination provided in texture constructor");
    }

    return externalFormat;
}

Texture::Texture(
    GLuint textureID,
    const ColorBufferDefinition& colorBufferDefinition,
    const std::string& filepath
) : 
Resource<Texture>{0},
mID { textureID }, 
mFilepath { filepath },
mColorBufferDefinition{colorBufferDefinition}
{}

Texture::Texture(Texture&& other) noexcept: Resource<Texture>(0), mID{other.mID}, mFilepath{other.mFilepath}, mColorBufferDefinition{other.mColorBufferDefinition} {
    //Prevent other from destroying this texture when its
    //deconstructor is called
    other.releaseResource();
}

Texture::Texture(const Texture& other)
: Resource<Texture>{0}, mFilepath {other.mFilepath}, mColorBufferDefinition {other.mColorBufferDefinition}
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

void Texture::free() {
    if(!mID) return;
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
    return deduceInternalFormat(mColorBufferDefinition);
}

GLenum Texture::externalFormat() {
    return deduceExternalFormat(mColorBufferDefinition);
}

std::shared_ptr<IResource> TextureFromFile::createResource(const nlohmann::json& methodParameters) {
    std::string filepath { methodParameters["path"].get<std::string>() };
    ColorBufferDefinition colorBufferDefinition {
        .mDataType { GL_UNSIGNED_BYTE },
        .mUsesWebColors { true }
    };

    // Load image from file into a convenient SDL surface, per the image
    SDL_Surface* textureImage { IMG_Load(filepath.c_str()) };
    assert(textureImage && "SDL image loading failed");

    // Convert the image from its present format to RGBA
    SDL_Surface* pretexture { SDL_ConvertSurfaceFormat(textureImage, SDL_PIXELFORMAT_RGBA32, 0) };
    SDL_FreeSurface(textureImage);
    textureImage = nullptr;
    assert(pretexture && "Could not convert SDL image to known image format");
  
    //TODO: flip surface along Y axis if necessary

    // Move surface pixels to the graphics card
    GLuint texture;
    
    colorBufferDefinition.mDimensions = {pretexture->w, pretexture->h};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0,
        //assume linear space if not an albedo texture
        deduceInternalFormat(colorBufferDefinition),
        colorBufferDefinition.mDimensions.x, colorBufferDefinition.mDimensions.y,
        0, deduceExternalFormat(colorBufferDefinition), colorBufferDefinition.mDataType,
        reinterpret_cast<void*>(pretexture->pixels)
    );
    SDL_FreeSurface(pretexture);
    pretexture=nullptr;

    // Check for OpenGL errors
    GLuint error { glGetError() };
    assert(error == GL_NO_ERROR && "An error occurred during allocation of openGL texture");

    // Configure a few texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, colorBufferDefinition.mWrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, colorBufferDefinition.mWrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, colorBufferDefinition.mMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, colorBufferDefinition.mMagFilter);

    // Construct and return a shared pointer to this texture to the caller
    return std::make_shared<Texture>(texture, colorBufferDefinition, filepath);
}

std::shared_ptr<IResource> TextureFromColorBufferDefinition::createResource(const nlohmann::json& methodParameters) {
    ColorBufferDefinition colorBufferDefinition {
        jsonToColorBufferDefinition(methodParameters)
    };
    assert(colorBufferDefinition.mComponentCount == 1 || colorBufferDefinition.mComponentCount == 4);

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            deduceInternalFormat(colorBufferDefinition),
            colorBufferDefinition.mDimensions.x,
            colorBufferDefinition.mDimensions.y,
            0, 
            deduceExternalFormat(colorBufferDefinition),
            colorBufferDefinition.mDataType,
            NULL
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, colorBufferDefinition.mMagFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, colorBufferDefinition.mMinFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, colorBufferDefinition.mWrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, colorBufferDefinition.mWrapT);
    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_shared<Texture>(texture, colorBufferDefinition);
};

ColorBufferDefinition jsonToColorBufferDefinition(const nlohmann::json& methodParameters) {
    ColorBufferDefinition colorBufferDefinition {
        .mDimensions {
            {methodParameters.at("dimensions").at(0).get<float>()},
            {methodParameters.at("dimensions").at(1).get<float>()}
        },
        .mMagFilter {
            kStringToFilter.at(methodParameters.at("magFilter").get<std::string>())
        },
        .mMinFilter {
            kStringToFilter.at(methodParameters.at("minFilter").get<std::string>())
        },
        .mWrapS {
            kStringToWrap.at(methodParameters.at("wrapS").get<std::string>())
        },
        .mWrapT {
            kStringToWrap.at(methodParameters.at("wrapT").get<std::string>())
        },
        .mDataType{
            static_cast<GLenum>(methodParameters.at("dataType").get<std::string>() == "float"? 
                GL_FLOAT :
                GL_UNSIGNED_BYTE
            )
        },
        .mComponentCount { 
            static_cast<GLbyte>(methodParameters.at("componentCount").get<uint8_t>())
        },
        .mUsesWebColors {
            methodParameters.at("usesWebColors").get<bool>()
        },
    };
    assert(colorBufferDefinition.mComponentCount == 1 || colorBufferDefinition.mComponentCount == 4);
    return colorBufferDefinition;
}

nlohmann::json colorBufferDefinitionToJSON(const ColorBufferDefinition& colorBufferDefinition) {
    nlohmann::json methodParams {
        {"dimensions", { colorBufferDefinition.mDimensions.x, colorBufferDefinition.mDimensions.y }},
        {"magFilter", kFilterToString.at(colorBufferDefinition.mMagFilter)},
        {"minFilter", kFilterToString.at(colorBufferDefinition.mMinFilter)},
        {"wrapS", kWrapToString.at(colorBufferDefinition.mWrapS)},
        {"wrapT", kWrapToString.at(colorBufferDefinition.mWrapT)},
        {"dataType", colorBufferDefinition.mDataType == GL_FLOAT? "float": "unsigned-byte"},
        {"componentCount", colorBufferDefinition.mComponentCount},
        {"usesWebColors", colorBufferDefinition.mUsesWebColors},
    };
    return methodParams;
}
