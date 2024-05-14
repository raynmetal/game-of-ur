#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture_manager.hpp"
#include "framebuffer.hpp"

Framebuffer::Framebuffer(glm::vec2 dimensions, GLuint nColorAttachments, std::vector<ColorBufferDefinition> colorBufferDefinitions, bool useRBO) {
    initialize(dimensions, nColorAttachments, colorBufferDefinitions, useRBO);
}

Framebuffer::Framebuffer(const Framebuffer& other){
    copyResource(other);
}

Framebuffer::Framebuffer(Framebuffer&& other) {
    stealResource(other);
}

Framebuffer& Framebuffer::operator=(const Framebuffer& other) {
    if(&other == this) return *this;
    destroyResource();
    copyResource(other);
    return *this;
}
Framebuffer& Framebuffer::operator=(Framebuffer&& other) {
    if(&other == this) return *this;
    destroyResource();
    stealResource(other);
    return *this;
}

Framebuffer::~Framebuffer() {
    destroyResource();
}

void Framebuffer::initialize(glm::vec2 dimensions, GLuint nColorAttachments, std::vector<ColorBufferDefinition> colorBufferDefinitions, bool useRBO) {
    if(!nColorAttachments && !useRBO) {
        throw std::invalid_argument("Framebuffer must have at least one color buffer attachment or one depth stencil attachment");
    }
    if(!mID) {
        glGenFramebuffers(1, &mID);
    }
    if(nColorAttachments > colorBufferDefinitions.size()) {
        throw std::invalid_argument("Not enough color buffer definitions to support number of attachments specified for this framebuffer");
    }

    mDimensions = dimensions;
    mColorBufferDefinitions = colorBufferDefinitions;
    mNColorAttachments = nColorAttachments;

    bind();
        // If there are color buffers defined, we'll create and store them

        GLuint count {0};
        for(const ColorBufferDefinition& colorBufferDefinition: mColorBufferDefinitions) {
            mTextureHandles.push_back(
                TextureManager::getInstance().registerResource(
                    "framebuffer_" + std::to_string(mID) + "::color_buffer_" + std::to_string(count),
                    {
                        mDimensions,
                        colorBufferDefinition.mDataType == ColorBufferDefinition::DataType::Float?
                            GL_FLOAT: GL_UNSIGNED_BYTE,
                        GL_LINEAR,
                        GL_LINEAR,
                        GL_CLAMP_TO_EDGE,
                        GL_CLAMP_TO_EDGE,
                        colorBufferDefinition.mComponentCount == ColorBufferDefinition::ComponentCount::Four?
                            4: 1
                    }
                )
            );
            //Assume the first n color buffers become the first n
            //framebuffer attachments
            if(count < mNColorAttachments) {
                mTextureHandles.back().getResource().attachToFramebuffer(count);
            }
            ++count;
        }

        // Declare which color attachments this framebuffer will use when rendering
        if(mNColorAttachments){
            std::vector<GLenum> colorAttachments(mNColorAttachments);
            for(GLuint i{0}; i < mNColorAttachments; ++i) {
                colorAttachments[i] = GL_COLOR_ATTACHMENT0+i;
            }
            glDrawBuffers(mNColorAttachments, colorAttachments.data());
        }

        if(useRBO) {
            if(!mRBO) glGenRenderbuffers(1, &mRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mDimensions.x, mDimensions.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Could not complete creation of framebuffer with ID " + std::to_string(mID));
    }
    unbind();
}

std::vector<TextureHandle> Framebuffer::getColorBufferHandles()  {
    return mTextureHandles;
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
}
void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroyResource() {
    glDeleteRenderbuffers(1, &mRBO);
    glDeleteFramebuffers(1, &mID);
    mColorBufferDefinitions.clear();
    mTextureHandles.clear();
    mDimensions = {0.f, 0.f};
    mNColorAttachments = 0;
    mID = 0;
    mRBO = 0;
}

void Framebuffer::releaseResource() {
    mColorBufferDefinitions.clear();
    mTextureHandles.clear();
    mDimensions = {0.f, 0.f};
    mNColorAttachments = 0;
    mID = 0;
    mRBO = 0;
}

void Framebuffer::copyResource(const Framebuffer& other) {
    if(!mID)
        glGenFramebuffers(1, &mID);

    mColorBufferDefinitions = other.mColorBufferDefinitions;
    mNColorAttachments = other.mNColorAttachments;
    mDimensions = other.mDimensions;
    mTextureHandles.clear();

    bind();
        GLuint count {0};
        for(const TextureHandle textureHandle: other.mTextureHandles) {
            mTextureHandles.push_back(
                TextureManager::getInstance().registerResource(
                    "framebuffer_" + std::to_string(mID) + "::color_buffer_" + std::to_string(count),
                    // makes a copy of the texture pointed to by the texture handle
                    Texture{ textureHandle.getResource() }
                )
            );
            if(count < mNColorAttachments){
                mTextureHandles.back().getResource().attachToFramebuffer(count);
            }
            ++count;
        }

        if(mNColorAttachments) {
            std::vector<GLenum> colorAttachments(mNColorAttachments);
            for(GLuint i{0}; i < mNColorAttachments; ++i) {
                colorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            glDrawBuffers(mNColorAttachments, colorAttachments.data());
        }

        //generate a render buffer object, if necessary
        if(other.mRBO){
            if(!mRBO) {
                glGenRenderbuffers(1, &mRBO);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mDimensions.x, mDimensions.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Could not complete creation of framebuffer with ID " + std::to_string(mID));
        }
    unbind();
}

void Framebuffer::stealResource(Framebuffer& other) {
    mID = other.mID;
    mRBO = other.mRBO;
    mNColorAttachments = other.mNColorAttachments;
    mDimensions = other.mDimensions;
    mColorBufferDefinitions = other.mColorBufferDefinitions;
    mTextureHandles = other.mTextureHandles;

    other.releaseResource();
}
