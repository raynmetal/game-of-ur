#ifndef ZORENDERSTAGE_H
#define ZORENDERSTAGE_H

#include <string>
#include <map>
#include <queue>

#include <SDL2/SDL.h>

#include "texture.hpp"
#include "shader_program.hpp"
#include "framebuffer.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "instance.hpp"
#include "model.hpp"
#include "light.hpp"
#include "util.hpp"

/*
    Creates a simple sort key with the following priority:
        Mesh > Material Texture > Material Everything Else
*/

struct OpaqueRenderUnit {
    OpaqueRenderUnit(std::shared_ptr<StaticMesh> meshHandle,std::shared_ptr<Material> materialHandle, glm::mat4 modelMatrix):
        mMeshHandle{meshHandle}, mMaterialHandle{materialHandle}, mModelMatrix{modelMatrix}
    {
        setSortKey();
    }

    bool operator<(const OpaqueRenderUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    std::shared_ptr<StaticMesh> mMeshHandle;
    std::shared_ptr<Material> mMaterialHandle;
    glm::mat4 mModelMatrix;

    void setSortKey() {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<StaticMesh*>{}(mMeshHandle.get())) };
        std::uint32_t materialHash { static_cast<uint32_t>(std::hash<Material*>{}(mMaterialHandle.get()))};
        mSortKey |= (meshHash << ((sizeof(uint32_t)/2)*8)) & 0xFFFF0000;
        mSortKey |= (materialHash << 0) & 0x0000FFFF;
    }
};

struct LightRenderUnit {
    LightRenderUnit(std::shared_ptr<StaticMesh> meshHandle, const LightEmissionData& lightEmissionData, const glm::mat4& modelMatrix):
    mMeshHandle{ meshHandle },
    mModelMatrix{ modelMatrix },
    mLightAttributes { lightEmissionData }
    { setSortKey(); }

    bool operator<(const LightRenderUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    std::shared_ptr<StaticMesh> mMeshHandle;
    glm::mat4 mModelMatrix;
    LightEmissionData mLightAttributes;

    void setSortKey() {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<StaticMesh*>{}(mMeshHandle.get())) };
        mSortKey = meshHash;
    }
};

class BaseRenderStage {
public:
    BaseRenderStage(const std::string& shaderFilepath);

    BaseRenderStage(const BaseRenderStage& other) = delete;
    BaseRenderStage(BaseRenderStage&& other) = delete;
    BaseRenderStage& operator=(const BaseRenderStage& other) = delete;
    BaseRenderStage& operator=(BaseRenderStage&& other) = delete;

    virtual ~BaseRenderStage();

    virtual void setup(const glm::u16vec2& targetDimensions) = 0;
    virtual void validate() = 0;
    virtual void execute() = 0;

    void attachTexture(const std::string& name, std::shared_ptr<Texture> textureHandle);
    void attachMesh(const std::string& name, std::shared_ptr<StaticMesh> meshHandle);
    void attachMaterial(const std::string& name, std::shared_ptr<Material> materialHandle);

    std::shared_ptr<Texture> getTexture(const std::string& name);
    std::shared_ptr<StaticMesh> getMesh(const std::string& name);
    std::shared_ptr<Material> getMaterial(const std::string& name);

    void useViewport();
    void setTargetViewport(const SDL_Rect& targetViewport);
    void submitToRenderQueue(OpaqueRenderUnit renderUnit);
    void submitToRenderQueue(LightRenderUnit lightRenderUnit);

protected:
    GLuint mVertexArrayObject {};
    std::shared_ptr<ShaderProgram> mShaderHandle;

    std::map<std::string, std::shared_ptr<Texture>> mTextureAttachments {};
    std::map<std::string, std::shared_ptr<StaticMesh>> mMeshAttachments {};
    std::map<std::string, std::shared_ptr<Material>> mMaterialAttachments {};
    std::priority_queue<OpaqueRenderUnit> mOpaqueMeshQueue {};
    std::priority_queue<LightRenderUnit> mLightQueue {};

    SDL_Rect mTargetViewport {0, 0, 800, 600};
};

class BaseOffscreenRenderStage: public BaseRenderStage {
public:
    BaseOffscreenRenderStage(const std::string& shaderFilepath, const nlohmann::json& templateFramebufferDescription);

    std::size_t attachTextureAsTarget(std::shared_ptr<Texture> textureHandle);
    std::size_t attachTextureAsTarget(const std::string& targetName, std::shared_ptr<Texture> textureHandle);
    void declareRenderTarget(const std::string& name, unsigned int index);
    std::shared_ptr<Texture> getRenderTarget(const std::string& name);

protected:
    std::shared_ptr<Framebuffer> mFramebufferHandle;
    nlohmann::json mTemplateFramebufferDescription;
    std::map<std::string, unsigned int> mRenderTargets {};
};

class GeometryRenderStage : public BaseOffscreenRenderStage {
public:
    GeometryRenderStage(const std::string& shaderFilepath) 
    : BaseOffscreenRenderStage{shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 3},
            {"dimensions", nlohmann::json::array({
                800, 600
            })},
            {"useRBO", true },
            {"colorBufferDefinitions",{
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
                ColorBufferDefinition{
                    .mDataType=GL_UNSIGNED_BYTE,
                    .mComponentCount=4
                }
            }},
        }}
    })}
    {}

    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
};

class LightingRenderStage : public BaseOffscreenRenderStage {
public:
    LightingRenderStage(const std::string& shaderFilepath)
    : BaseOffscreenRenderStage{shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 2},
            {"dimensions", nlohmann::json::array({
                800, 600
            })},
            {"useRBO", true },
            {"colorBufferDefinitions",{
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
            }},
        }}
    })}
    {}
    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
};

class BlurRenderStage : public BaseOffscreenRenderStage {
public:
    BlurRenderStage(const std::string& shaderFilepath)
    : BaseOffscreenRenderStage{shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 2},
            {"dimensions", nlohmann::json::array({
                800, 600 
            })},
            {"useRBO", false},
            {"colorBufferDefinitions",{
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
                ColorBufferDefinition{
                    .mDataType=GL_FLOAT,
                    .mComponentCount=4
                },
            }},
        }}
    })}
    {}
    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
};

class TonemappingRenderStage : public BaseOffscreenRenderStage {
public:
    TonemappingRenderStage(const std::string& shaderFilepath)
    : BaseOffscreenRenderStage{shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 1},
            {"dimensions", nlohmann::json::array({
                800, 600
            })},
            {"useRBO", false},
            {"colorBufferDefinitions",{
                ColorBufferDefinition{
                    .mDataType=GL_UNSIGNED_BYTE,
                    .mComponentCount=4
                },
            }},
        }}
    })}
    {}

    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
};

class AdditionRenderStage: public BaseOffscreenRenderStage {
public:
    AdditionRenderStage(const std::string& shaderFilepath):
    BaseOffscreenRenderStage(shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 1},
            {"dimensions", nlohmann::json::array({
                800, 600
            })},
            {"useRBO", false},
            {"colorBufferDefinitions", {
                ColorBufferDefinition {
                    .mDataType=GL_UNSIGNED_BYTE,
                    .mComponentCount=4,
                },
            }},
        }},
    }))
    {}

    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
private:
};

class ScreenRenderStage: public BaseRenderStage {
public:
    ScreenRenderStage(const std::string& shaderFilepath):
    BaseRenderStage(shaderFilepath)
    {}

    virtual void setup(const glm::u16vec2& targetDimensions) override;
    virtual void validate() override;
    virtual void execute() override;
};

class ResizeRenderStage : public BaseOffscreenRenderStage {
public:
    ResizeRenderStage(const std::string& shaderFilepath)
    : BaseOffscreenRenderStage{shaderFilepath, nlohmann::json::object({
        {"type", Framebuffer::getResourceTypeName()},
        {"method", FramebufferFromDescription::getResourceConstructorName()},
        {"parameters", {
            {"nColorAttachments", 1},
            {"dimensions", nlohmann::json::array({
                800, 600
            })},
            {"useRBO", false},
            {"colorBufferDefinitions",{
                ColorBufferDefinition{
                    .mDataType=GL_UNSIGNED_BYTE,
                    .mComponentCount=4,
                },
            }},
        }}
    })}
    {}

    virtual void setup(const glm::u16vec2& textureDimensions) override;
    virtual void validate() override;
    virtual void execute() override;

private:
};

#endif
