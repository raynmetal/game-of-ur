#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <string>

#include <GL/glew.h>

class Texture {
public:
    enum Usage {
        NA,
        Albedo,
        Normal,
        Specular
    };
    /* Load texture directly from file */
    Texture(const std::string& filepath, const Usage usage=NA);
    /* Assume texture was manually loaded elsewhere, and store its 
    properties here */
    Texture(GLuint textureID, const Usage usage);
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
    bool loadFromFile(const std::string& filepath);
    /* basic deallocate function */
    virtual void free();

    /* texture ID getter */
    GLuint getTextureID() const;
    /* texture usage getter */
    Usage getUsage() const;

    /* get texture width */
    GLint getWidth() const;
    /* get texture height */
    GLint getHeight() const;

protected:
    void copyImage(const Texture& other);

    /* This texture's name, as referenced by OpenGL */
    GLuint mID;
    /* The path this texture was loaded from, if any */
    std::string mFilepath;
    /* This texture's type, essentially indicating its usage */
    Usage mUsage;
};

#endif