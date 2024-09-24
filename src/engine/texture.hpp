#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "resource_manager.hpp"

struct ColorBufferDefinition {
    glm::vec2 mDimensions {800, 600};
    GLenum mMagFilter { GL_LINEAR };
    GLenum mMinFilter { GL_LINEAR };
    GLenum mWrapS { GL_CLAMP_TO_EDGE };
    GLenum mWrapT { GL_CLAMP_TO_EDGE };
    GLenum mDataType { GL_UNSIGNED_BYTE };
    GLbyte mComponentCount { 4 };
    bool mUsesWebColors { false };
};


class Texture : public IResource {
public:
    /* Load texture directly from file */
    Texture(const std::string& filepath);

    /*
    Generates the texture described by the arguments
    */
    Texture(ColorBufferDefinition definition);

    /* Load an empty texture object, with an mID of 0 indicating
    that there is no texture here */
    Texture();

    /*Copy construction*/
    Texture(const Texture& other);
    /*Copy assignment*/
    Texture& operator=(const Texture& other);

    /*Move construction*/
    Texture(Texture&& other) noexcept;
    /*Move assignment*/
    Texture& operator=(Texture&& other) noexcept;

    /*Destructor belonging to latest subclass*/
    virtual ~Texture();

    /* basic load and allocate function */
    bool loadFromFile(
        const std::string& filepath, 
        const ColorBufferDefinition& colorBufferDefinition = {
            .mDataType { GL_UNSIGNED_BYTE },
            .mUsesWebColors { true }
        }
    );
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
    void destroyResource() override;
    /* removes references to (OpenGL managed) texture tied to this object */
    void releaseResource() override;

friend class ResourceManager<Texture>;
};

#endif
