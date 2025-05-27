#ifndef FOOLSENGINE_FRAMEBUFFER_H
#define FOOLSENGINE_FRAMEBUFFER_H

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "core/resource_database.hpp"

namespace ToyMakersEngine {

    class Framebuffer;

    class RBO {
    public:
        inline static std::unique_ptr<RBO> create(const glm::vec2& dimensions) {
            return std::unique_ptr<RBO>{ new RBO{dimensions} };
        }
        ~RBO();
        inline GLuint getID() const { return mID; }
        void resize(const glm::vec2& dimensions);
    private:
        RBO(const glm::vec2& dimensions);
        GLuint mID;
    };

    class Framebuffer : public Resource<Framebuffer> {
    public:
        /* Creates a framebuffer with some number of color attachments and color buffers which it manages, optionally
        allowing the user to create an RBO for it */
        Framebuffer(
            GLuint framebuffer,
            glm::vec2 dimensions,
            GLuint nColorAttachments,
            const std::vector<std::shared_ptr<Texture>>& colorBuffers,
            std::unique_ptr<RBO> rbo
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

        std::size_t addTargetColorBufferHandle(std::shared_ptr<Texture> colorBufferHandle);

        /* returns a vector of handles to this framebuffer's textures */
        std::vector<std::shared_ptr<const Texture>> getTargetColorBufferHandles() const;
        const std::vector<std::shared_ptr<Texture>>& getTargetColorBufferHandles();

        bool hasAttachedRBO() const;
        bool hasOwnRBO() const;
        RBO& getOwnRBO();

        void attachRBO(RBO& rbo);
        void detachRBO();

        /* command to bind this framebuffer */
        void bind();

        glm::u16vec2 getDimensions() const { return mDimensions; } 

        /* command to unbind this framebuffer (or in other words, to bind the 
        default framebuffer) */
        void unbind();

        inline static std::string getResourceTypeName() { return "Framebuffer"; }

    private:
        GLuint mID {};
        std::unique_ptr<RBO> mOwnRBO {};
        GLuint mNColorAttachments {};
        glm::vec2 mDimensions {};
        std::vector<std::shared_ptr<Texture>> mTextureHandles {};
        bool mHasAttachedRBO { false };

        void attachRBO_(RBO& rbo);
        void detachRBO_();

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

}

#endif
