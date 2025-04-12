#ifndef ZOFRAMEBUFFER_H
#define ZOFRAMEBUFFER_H

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "resource_database.hpp"

class Framebuffer : public Resource<Framebuffer> {
public:
    /* Creates a framebuffer with some number of color attachments and color buffers which it manages, optionally
    allowing the user to create an RBO for it */
    Framebuffer(
        GLuint framebuffer,
        glm::vec2 dimensions,
        GLuint nColorAttachments,
        const std::vector<std::shared_ptr<Texture>>& colorBuffers,
        GLuint rbo
    );

    /*Framebuffer destructor, calls destroyResource */
    ~Framebuffer() override;

    /* copy constructor */
    Framebuffer(const Framebuffer& other);
    /* move constructor */
    Framebuffer(Framebuffer&& other);
    /* copy assignment operator */
    Framebuffer& operator=(const Framebuffer& other);
    /* move assignment operator */
    Framebuffer& operator=(Framebuffer&& other);

    /* returns a vector of handles to this framebuffer's textures */
    std::vector<std::shared_ptr<const Texture>> getColorBufferHandles() const;
    const std::vector<std::shared_ptr<Texture>>& getColorBufferHandles();

    /* command to bind this framebuffer */
    void bind();

    /* command to unbind this framebuffer (or in other words, to bind the 
    default framebuffer) */
    void unbind();

    bool hasRBO() const;

    inline static std::string getResourceTypeName() { return "Framebuffer"; }

private:
    GLuint mID {};
    GLuint mRBO {};
    GLuint mNColorAttachments {};
    glm::vec2 mDimensions {};
    std::vector<std::shared_ptr<Texture>> mTextureHandles {};

    void destroyResource();
    void releaseResource();

    void copyResource(const Framebuffer& other);
    void stealResource(Framebuffer& other);
};

class FramebufferFromDescription: public ResourceConstructor<Framebuffer, FramebufferFromDescription> {
public:
    FramebufferFromDescription(): 
        ResourceConstructor<Framebuffer,FramebufferFromDescription>{0} 
    {}

    static std::string getResourceConstructorName() { return "fromDescription"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParams) override;
};

#endif
