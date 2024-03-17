#ifndef ZOTEXTURE_H
#define ZOTEXTURE_H

#include <string>

#include <GL/glew.h>

class Texture {
public:
    enum Type {
        TextureNA,
        TextureAlbedo,
        TextureNormal,
        TextureSpecular
    };
    /* Load texture directly from file */
    Texture(const std::string& filepath, const Type type=TextureNA);
    /* Assume texture was manually loaded elsewhere, and store its 
    properties here */
    Texture(GLuint textureID, const Type type);
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
    bool loadTextureFromFile(const std::string& filepath);
    /* basid deallocate function */
    void freeTexture();

    /* Bind/unbind texture */
    virtual void bindTexture() const;

    /* texture ID getter */
    GLuint getTextureID() const;
    /* texture type getter */
    Type getType() const;

protected:
    /* This texture's name, as referenced by OpenGL */
    GLuint mID;
    /* The path this texture was loaded from, if any */
    std::string mFilepath;
    /* This texture's type, essentially indicating its usage */
    Type mType;
};

#endif
