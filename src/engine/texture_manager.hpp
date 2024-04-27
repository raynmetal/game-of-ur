#ifndef ZOTEXTUREMANAGER_H
#define ZOTEXTUREMANAGER_H

#include <string>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"

class TextureManager {
public:
    class TextureHandle {
    public:
        /* decrements reference count in texture manager */
        ~TextureHandle();

        /* copy constructor, increments reference count */
        TextureHandle(const TextureHandle& other);
        /* move constructor, no reference cont incrementing or decrementing */
        TextureHandle(TextureHandle&& other);
        /* copy assignment, increments reference count */
        TextureHandle& operator=(const TextureHandle& other);
        /* move assignment, no increment or decrement of reference count*/
        TextureHandle& operator=(TextureHandle&& other);

        bool operator==(const TextureHandle& other) const;

        /* returns a copy of the (unique) name string for this texture */
        std::string getName() const;
        /* returns the usage type of the texture being referred to */
        Texture::Usage getUsage() const;

        /* binds texture to specified texture unit */
        void bind(GLuint textureUnit=0) const;

        /* attaches texture at this attachment point, assumes framebuffer is already bound */
        void attachToFramebuffer(GLuint attachmentUnit=0) const;

    private: 
        /* creates a texture handle. can't be called outside outer class*/
        TextureHandle(const std::string& name, GLuint glHandle, Texture::Usage usage);

        std::string mName;
        GLuint mGLHandle;
        Texture::Usage mUsage {Texture::NA};

    friend class TextureManager;
    };

    ~TextureManager();

    /* creates or returns the one instance of this class that exists*/
    static TextureManager& getInstance();

    TextureHandle getTexture(const std::string& nameOrFilepath);

    /* load texture from a file, or otherwise return a handle to it if it's already available */
    TextureHandle getFileTexture(const std::string& filepathOrName, Texture::Usage usage);

    /* create a texture with these attributes, or return a handle to it if it's already available */
    TextureHandle getGeneratedTexture(const std::string& name, glm::vec2 dimensions, GLenum dataType, GLenum magFilter, GLenum minFilter, GLenum wrapS, GLenum wrapT);

    void removeUnusedTextures();

private:
    /* increment the reference count for the texture with this name. Empty strings are 
    null operations. */
    void incrementReferenceCount(const std::string& name);

    /* decrement the reference count for the texture with this name. Empty strings
    are null operations. */
    void decrementReferenceCount(const std::string& name);

    /* Counts the references to a texture with a given name */
    std::map<const std::string, GLuint> mReferenceCounts {};
    /* maps each texture name to the texture it refers to */
    std::map<const std::string, Texture> mTextures {};
};

#endif
