#ifndef ZORENDERSTAGE_H
#define ZORENDERSTAGE_H

#include <string>
#include <map>

#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "framebuffer_manager.hpp"
#include "mesh_manager.hpp"
#include "material_manager.hpp"
#include "model_manager.hpp"
#include "light.hpp"

class BaseRenderStage {
public:
    BaseRenderStage(const std::string& shaderFilepath);

    virtual ~BaseRenderStage() = default;

    virtual void setup() = 0;
    virtual void validate() = 0;
    virtual void execute() = 0;

    void updateFloatParameter(const std::string& name, float parameter);
    void updateIntParameter(const std::string& name, int parameter);
    void attachTexture(const std::string& name, const TextureHandle& textureHandle);
    void attachMesh(const std::string& name, const MeshHandle& meshHandle);
    void attachModel(const std::string& name, const ModelHandle& modelHandle);
    void attachLightCollection(const std::string& name, const LightCollectionHandle& lightCollectionHandle);

    float getFloatParameter(const std::string& name);
    int getIntParameter(const std::string& name);
    TextureHandle getTexture(const std::string& name);
    MeshHandle getMesh(const std::string& name);
    ModelHandle getModel(const std::string& name);
    LightCollectionHandle getLightCollection(const std::string& name);
protected:
    ShaderProgramHandle mShaderHandle;

    std::map<std::string, TextureHandle> mTextureAttachments {};
    /*
     * TODO: We probably want to replace this later with vertex buffer 
     * attachments, but for now we're working with what we've got
     */
    std::map<std::string, MeshHandle> mMeshAttachments {};
    /*
     * TODO: We probably want to replace this later with vertex buffer 
     * attachments, but for now we're working with what we've got
     */
    std::map<std::string, ModelHandle> mModelAttachments {};
    /*
     * TODO: We probably want to replace this later with vertex buffer 
     * attachments, but for now we're working with what we've got
     */
    std::map<std::string, LightCollectionHandle> mLightCollectionAttachments {};
    std::map<std::string, float> mFloatParameters {};
    std::map<std::string, int> mIntParameters {};
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
