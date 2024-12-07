#include <memory>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "resource_database.hpp"
#include "texture.hpp"

#include "framebuffer.hpp"

Framebuffer::Framebuffer(
    GLuint framebuffer,
    glm::vec2 dimensions,
    GLuint nColorAttachments,
    std::vector<std::shared_ptr<Texture>> colorBuffers,
    GLuint rbo
) :
Resource<Framebuffer>{0},
mID { framebuffer },
mRBO { rbo },
mNColorAttachments { nColorAttachments },
mDimensions { dimensions },
mTextureHandles { colorBuffers }
{}

Framebuffer::Framebuffer(const Framebuffer& other): Resource<Framebuffer>{0}{
    copyResource(other);
}

Framebuffer::Framebuffer(Framebuffer&& other):Resource<Framebuffer>{0} {
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

std::vector<std::shared_ptr<Texture>> Framebuffer::getColorBufferHandles()  {
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
    mTextureHandles.clear();
    mDimensions = {0.f, 0.f};
    mNColorAttachments = 0;
    mID = 0;
    mRBO = 0;
}

void Framebuffer::releaseResource() {
    mTextureHandles.clear();
    mDimensions = {0.f, 0.f};
    mNColorAttachments = 0;
    mID = 0;
    mRBO = 0;
}

void Framebuffer::copyResource(const Framebuffer& other) {
    if(!mID)
        glGenFramebuffers(1, &mID);

    mNColorAttachments = other.mNColorAttachments;
    mDimensions = other.mDimensions;
    mTextureHandles.clear();

    bind();
        GLuint count {0};
        for(const std::shared_ptr<Texture>& textureHandle: other.mTextureHandles) {
            std::string colorBufferName { 
                "framebuffer_" + std::to_string(mID) + "::color_buffer_" + std::to_string(count)
            };
            ResourceDatabase::addResourceDescription(
                nlohmann::json {
                    {"name", colorBufferName},
                    {"type", Texture::getResourceTypeName()},
                    {"method", TextureFromColorBufferDefinition::getResourceConstructorName()},
                    {"params", colorBufferDefinitionToJSON(textureHandle->getColorBufferDefinition())}
                }
            );
            mTextureHandles.push_back(
                ResourceDatabase::getResource<Texture>(colorBufferName)
            );
            if(count < mNColorAttachments){
                mTextureHandles.back()->attachToFramebuffer(count);
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
    mTextureHandles = other.mTextureHandles;

    other.releaseResource();
}

bool Framebuffer::hasRBO() {
    return mRBO != 0;
}

std::shared_ptr<IResource> FramebufferFromDescription::createResource(const nlohmann::json& methodParams) {
    uint32_t nColorAttachments { methodParams.at("nColorAttachments").get<uint32_t>() };
    bool useRBO { methodParams.at("useRBO").get<bool>() };
    glm::vec2 dimensions { 
        methodParams.at("dimensions").at(0).get<float>(),
        methodParams.at("dimensions").at(1).get<float>()
    };
    std::vector<nlohmann::json> colorBufferParams { 
        methodParams.at("colorBufferDefinitions").begin(), methodParams.at("colorBufferDefinitions").end()
    };
    assert((nColorAttachments || useRBO) && "Framebuffer must have at least one color buffer attachment or one depth stencil attachment");
    assert(nColorAttachments <= colorBufferParams.size() && "Not enough color buffer definitions to support number of attachments specified for this framebuffer");


    GLuint framebuffer;
    GLuint rbo { 0 };
    glGenFramebuffers(1, &framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        std::vector<std::shared_ptr<Texture>> textureHandles {};
        // If there are color buffers defined, we'll create and store them
        for(uint16_t i {0}; i < colorBufferParams.size(); ++i){
            std::string colorBufferName {
                "framebuffer_" + std::to_string(framebuffer) + "::color_buffer_" + std::to_string(i)
            };
            nlohmann::json colorBufferDescription { 
                {"name", colorBufferName},
                {"type", Texture::getResourceTypeName()},
                {"method", TextureFromColorBufferDefinition::getResourceConstructorName()},
                {"parameters", colorBufferParams[i]}
            };
            ResourceDatabase::addResourceDescription(
                colorBufferDescription
            );
            textureHandles.push_back(
                ResourceDatabase::getResource<Texture>(colorBufferName)
            );
            //Assume the first n color buffers become the first n
            //framebuffer attachments
            if(i < nColorAttachments) {
                textureHandles.back()->attachToFramebuffer(i);
            }
        }

        // Declare which color attachments this framebuffer will use when rendering
        if(nColorAttachments){
            std::vector<GLenum> colorAttachments(nColorAttachments);
            for(GLuint i{0}; i < nColorAttachments; ++i) {
                colorAttachments[i] = GL_COLOR_ATTACHMENT0+i;
            }
            glDrawBuffers(nColorAttachments, colorAttachments.data());
        }

        if(useRBO) {
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dimensions.x, dimensions.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && ("Could not complete creation of framebuffer with ID" + std::to_string(framebuffer)).c_str());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return std::make_shared<Framebuffer>(framebuffer, dimensions, nColorAttachments, textureHandles, rbo);
}
