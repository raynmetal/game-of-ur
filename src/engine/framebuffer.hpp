#ifndef ZOFRAMEBUFFER_H
#define ZOFRAMEBUFFER_H
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture_manager.hpp"
#include "resource_manager.hpp"

struct ColorBufferDefinition {
    enum class ComponentCount : GLuint {
        One = 1,
        Four = 4
    };
    enum class DataType : GLuint {
        Float = GL_FLOAT,
        Byte = GL_UNSIGNED_BYTE
    };
    ComponentCount mComponentCount {ComponentCount::Four};
    DataType mDataType {DataType::Byte};
};


class Framebuffer : IResource {
public:
    Framebuffer() = default;

    /* Creates a framebuffer with some number of color attachments and color buffers which it manages, optionally 
    allowing the user to create an RBO for it */
    Framebuffer(glm::vec2 dimensions, GLuint nColorAttachments, std::vector<ColorBufferDefinition> colorBufferDefinitions, bool useRBO);

    /*Framebuffer destructor, calls destroyResource */
    ~Framebuffer();

    /* copy constructor */
    Framebuffer(const Framebuffer& other);
    /* move constructor */
    Framebuffer(Framebuffer&& other);
    /* copy assignment operator */
    Framebuffer& operator=(const Framebuffer& other);
    /* move assignment operator */
    Framebuffer& operator=(Framebuffer&& other);

    void initialize(glm::vec2 dimensions, GLuint nColorAttachments, std::vector<ColorBufferDefinition> colorBufferDefinitions, bool useRBO);

    /* returns a vector of handles to this framebuffer's textures */
    std::vector<TextureHandle> getColorBufferHandles();

    /* command to bind this framebuffer */
    void bind();

    /* command to unbind this framebuffer (or in other words, to bind the 
    default framebuffer) */
    void unbind();

    bool hasRBO();

private:
    GLuint mID {};
    GLuint mRBO {};
    GLuint mNColorAttachments {};
    glm::vec2 mDimensions {};
    std::vector<ColorBufferDefinition> mColorBufferDefinitions {};
    std::vector<TextureHandle> mTextureHandles {};

    void destroyResource();
    void releaseResource();

    void copyResource(const Framebuffer& other);
    void stealResource(Framebuffer& other);

friend class ResourceManager<Framebuffer>;
};

#endif
