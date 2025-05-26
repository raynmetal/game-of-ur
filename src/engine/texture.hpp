#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "core/resource_database.hpp"

struct ColorBufferDefinition {
    enum class Type {
        TEXTURE_2D,
        CUBEMAP,
    };

    enum class CubemapLayout: uint8_t {
        NA,
        ROW,
        // COLUMN,
        // ROW_CROSS,
        // COLUMN_CROSS,
    };

    glm::vec2 mDimensions {800, 600};
    CubemapLayout mCubemapLayout { CubemapLayout::NA };
    GLenum mMagFilter { GL_LINEAR };
    GLenum mMinFilter { GL_LINEAR };
    GLenum mWrapS { GL_CLAMP_TO_EDGE };
    GLenum mWrapT { GL_CLAMP_TO_EDGE };
    GLenum mDataType { GL_UNSIGNED_BYTE };
    GLbyte mComponentCount { 4 };
    bool mUsesWebColors { false };
};


inline GLenum deduceInternalFormat(const ColorBufferDefinition& colorBufferDefinition) {
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

inline GLenum deduceExternalFormat(const ColorBufferDefinition& colorBufferDefinition) {
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


void to_json(nlohmann::json& json, const ColorBufferDefinition& colorBufferDefinition);
void from_json(const nlohmann::json& json, ColorBufferDefinition& colorBufferDefinition);
NLOHMANN_JSON_SERIALIZE_ENUM(ColorBufferDefinition::CubemapLayout, {
    {ColorBufferDefinition::CubemapLayout::ROW, "row"},
});

class Texture : public Resource<Texture> {
public:
    Texture(
        GLuint textureID,
        const ColorBufferDefinition& colorBufferDefinition,
        const std::string& filepath=""
    );    /*Copy construction*/

    Texture(const Texture& other);
    /*Copy assignment*/
    Texture& operator=(const Texture& other);

    /*Move construction*/
    Texture(Texture&& other) noexcept;
    /*Move assignment*/
    Texture& operator=(Texture&& other) noexcept;

    /*Destructor belonging to latest subclass*/
    virtual ~Texture();

    /* basic deallocate function */
    virtual void free();

    /* binds this texture to the specified texture unit */
    void bind(GLuint textureUnit) const;

    /* attaches this texture to a (presumably existing and bound) framebuffer*/
    void attachToFramebuffer(GLuint attachmentUnit) const;

    /* texture ID getter */
    GLuint getTextureID() const;

    /* get texture width */
    GLint getWidth() const;

    /* get texture height */
    GLint getHeight() const;
    

    inline static std::string getResourceTypeName() { return "Texture"; }

    ColorBufferDefinition getColorBufferDefinition() { return mColorBufferDefinition; }

protected:
    void copyImage(const Texture& other);

    /* This texture's name, as referenced by OpenGL */
    GLuint mID {0};
    /* The path this texture was loaded from, if any */
    std::string mFilepath {""};
    ColorBufferDefinition mColorBufferDefinition;

    void generateTexture();
    GLenum internalFormat();
    GLenum externalFormat();

    /* destroys (OpenGL managed) texture tied to this object */
    void destroyResource();
    /* removes references to (OpenGL managed) texture tied to this object */
    void releaseResource();
};

class TextureFromFile: public ResourceConstructor<Texture, TextureFromFile> {
public:
    TextureFromFile(): ResourceConstructor<Texture, TextureFromFile> {0} {}
    inline static std::string getResourceConstructorName() { return "fromFile"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

class TextureFromColorBufferDefinition: public ResourceConstructor<Texture, TextureFromColorBufferDefinition> {
public:
    TextureFromColorBufferDefinition(): ResourceConstructor<Texture, TextureFromColorBufferDefinition> {0} {}
    inline static std::string getResourceConstructorName() { return "fromDescription"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

#endif
