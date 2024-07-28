#ifndef ZORENDERSTAGE_H
#define ZORENDERSTAGE_H

#include <string>
#include <map>
#include <queue>

#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "framebuffer_manager.hpp"
#include "mesh_manager.hpp"
#include "material_manager.hpp"
#include "instance.hpp"
#include "model_manager.hpp"
#include "light.hpp"
#include "util.hpp"


/*
    Creates a simple sort key with the following priority:
        Mesh > Material Texture > Material Everything Else
*/

struct RenderUnit {
    RenderUnit(MeshHandle meshHandle, MaterialHandle materialHandle, Placement placement)
    :
        mMeshHandle{meshHandle}, mMaterialHandle{materialHandle}, 
        mModelMatrix{buildModelMatrix(placement.mPosition, placement.mOrientation, placement.mScale)},
        mPlacement { placement }
    {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<std::string>{}(meshHandle.getName())) };
        std::uint32_t materialHash { static_cast<uint32_t>(std::hash<std::string>{}(materialHandle.getName()))};
        mSortKey |= (meshHash << ((sizeof(uint32_t)/2)*8)) & 0xFFFF0000;
        mSortKey |= (materialHash << 0) & 0x0000FFFF;
    }

    bool operator<(const RenderUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    MeshHandle mMeshHandle;
    MaterialHandle mMaterialHandle;
    glm::mat4 mModelMatrix;
    Placement mPlacement;
};

struct RenderLightUnit {
    RenderLightUnit(const MeshHandle& meshHandle, const MaterialHandle& materialHandle, const Placement& placement, const LightEmissionData& lightEmissionData):
        mMeshHandle{meshHandle}, mMaterialHandle{materialHandle},
        mModelMatrix{buildModelMatrix(placement.mPosition, {}, placement.mScale)},
        mPlacement {placement},
        mLightAttributes {lightEmissionData}
    {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<std::string>{}(meshHandle.getName())) };
        std::uint32_t materialHash {static_cast<uint32_t>(std::hash<std::string>{}(materialHandle.getName()))};
        mSortKey |= (meshHash << ((sizeof(unsigned int)/2)*8)) & 0xFFFF0000;
        mSortKey |= (materialHash << 0) & 0x0000FFFF;
    }

    bool operator<(const RenderLightUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    MeshHandle mMeshHandle;
    MaterialHandle mMaterialHandle;
    glm::mat4 mModelMatrix;
    Placement mPlacement;
    LightEmissionData mLightAttributes;
};

class BaseRenderStage {
public:
    BaseRenderStage(const std::string& shaderFilepath);

    BaseRenderStage(const BaseRenderStage& other) = delete;
    BaseRenderStage(BaseRenderStage&& other) = delete;
    BaseRenderStage& operator=(const BaseRenderStage& other) = delete;
    BaseRenderStage& operator=(BaseRenderStage&& other) = delete;

    virtual ~BaseRenderStage();

    virtual void setup() = 0;
    virtual void validate() = 0;
    virtual void execute() = 0;

    void attachTexture(const std::string& name, const TextureHandle& textureHandle);
    void attachMesh(const std::string& name, const MeshHandle& meshHandle);
    void attachMaterial(const std::string& name, const MaterialHandle& materialHandle);

    TextureHandle getTexture(const std::string& name);
    MeshHandle getMesh(const std::string& name);
    MaterialHandle getMaterial(const std::string& name);

    void submitToRenderQueue(RenderUnit renderUnit);
    void submitToRenderQueue(RenderLightUnit lightRenderUnit);
protected:
    GLuint mVertexArrayObject {};
    ShaderProgramHandle mShaderHandle;

    std::map<std::string, TextureHandle> mTextureAttachments {};
    std::map<std::string, MeshHandle> mMeshAttachments {};
    std::map<std::string, MaterialHandle> mMaterialAttachments {};

    std::priority_queue<RenderUnit> mOpaqueMeshQueue {};
    std::priority_queue<RenderLightUnit> mLightQueue {};
};

class BaseOffscreenRenderStage: public BaseRenderStage {
public:
    BaseOffscreenRenderStage(const std::string& shaderFilepath);

    virtual void setup() = 0;
    virtual void validate() = 0;
    virtual void execute() = 0;

    void declareRenderTarget(const std::string& name, unsigned int index);
    TextureHandle getRenderTarget(const std::string& name);

protected:
    FramebufferHandle mFramebufferHandle;
    std::map<std::string, unsigned int> mRenderTargets {};
};

class GeometryRenderStage : public BaseOffscreenRenderStage {
public:
    GeometryRenderStage(const std::string& shaderFilepath) 
        : BaseOffscreenRenderStage(shaderFilepath)
    {}

    virtual void setup() override;
    virtual void validate() override;
    virtual void execute() override;
};

class LightingRenderStage : public BaseOffscreenRenderStage {
public:
    LightingRenderStage(const std::string& shaderFilepath)
        : BaseOffscreenRenderStage(shaderFilepath)
    {}
    virtual void setup() override;
    virtual void validate() override;
    virtual void execute() override;
};

class BlurRenderStage : public BaseOffscreenRenderStage {
public:
    BlurRenderStage(const std::string& shaderFilepath)
        : BaseOffscreenRenderStage{shaderFilepath}
    {}
    virtual void setup() override;
    virtual void validate() override;
    virtual void execute() override;
};

class TonemappingRenderStage : public BaseOffscreenRenderStage {
public:
    TonemappingRenderStage(const std::string& shaderFilepath)
        : BaseOffscreenRenderStage{shaderFilepath}
    {}

    virtual void setup() override;
    virtual void validate() override;
    virtual void execute() override;
};

class ScreenRenderStage: public BaseRenderStage {
public:
    ScreenRenderStage(const std::string& shaderFilepath)
        : BaseRenderStage(shaderFilepath)
    {}

    virtual void setup() override;
    virtual void validate() override;
    virtual void execute() override;
};

#endif
