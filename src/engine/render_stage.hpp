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

struct OpaqueRenderUnit {
    OpaqueRenderUnit(MeshHandle meshHandle, MaterialHandle materialHandle, glm::mat4 modelMatrix):
        mMeshHandle{meshHandle}, mMaterialHandle{materialHandle}, mModelMatrix{modelMatrix}
    {
        setSortKey();
    }

    bool operator<(const OpaqueRenderUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    MeshHandle mMeshHandle;
    MaterialHandle mMaterialHandle;
    glm::mat4 mModelMatrix;

    void setSortKey() {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<std::string>{}(mMeshHandle.getName())) };
        std::uint32_t materialHash { static_cast<uint32_t>(std::hash<std::string>{}(mMaterialHandle.getName()))};
        mSortKey |= (meshHash << ((sizeof(uint32_t)/2)*8)) & 0xFFFF0000;
        mSortKey |= (materialHash << 0) & 0x0000FFFF;
    }
};

struct LightRenderUnit {
    LightRenderUnit(const MeshHandle& meshHandle, const MaterialHandle& materialHandle, const LightEmissionData& lightEmissionData, const glm::mat4& modelMatrix):
        mMeshHandle{ meshHandle }, mMaterialHandle{ materialHandle },
        mModelMatrix{ modelMatrix },
        mLightAttributes { lightEmissionData }
    {
        setSortKey();
    }

    bool operator<(const LightRenderUnit& other) const {
        return mSortKey < other.mSortKey;
    }

    std::uint32_t mSortKey {};
    MeshHandle mMeshHandle;
    MaterialHandle mMaterialHandle;
    glm::mat4 mModelMatrix;
    LightEmissionData mLightAttributes;

    void setSortKey() {
        std::uint32_t meshHash { static_cast<uint32_t>(std::hash<std::string>{}(mMeshHandle.getName())) };
        std::uint32_t materialHash { static_cast<uint32_t>(std::hash<std::string>{}(mMaterialHandle.getName()))};
        mSortKey |= (meshHash << ((sizeof(uint32_t)/2)*8)) & 0xFFFF0000;
        mSortKey |= (materialHash << 0) & 0x0000FFFF;
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

    virtual void setup() = 0;
    virtual void validate() = 0;
    virtual void execute() = 0;

    void attachTexture(const std::string& name, const TextureHandle& textureHandle);
    void attachMesh(const std::string& name, const MeshHandle& meshHandle);
    void attachMaterial(const std::string& name, const MaterialHandle& materialHandle);

    TextureHandle getTexture(const std::string& name);
    MeshHandle getMesh(const std::string& name);
    MaterialHandle getMaterial(const std::string& name);

    void submitToRenderQueue(OpaqueRenderUnit renderUnit);
    void submitToRenderQueue(LightRenderUnit lightRenderUnit);
protected:
    GLuint mVertexArrayObject {};
    ShaderProgramHandle mShaderHandle;

    std::map<std::string, TextureHandle> mTextureAttachments {};
    std::map<std::string, MeshHandle> mMeshAttachments {};
    std::map<std::string, MaterialHandle> mMaterialAttachments {};

    std::priority_queue<OpaqueRenderUnit> mOpaqueMeshQueue {};
    std::priority_queue<LightRenderUnit> mLightQueue {};
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
