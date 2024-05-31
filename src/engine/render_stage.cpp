#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "framebuffer_manager.hpp"
#include "light_manager.hpp"

#include "render_stage.hpp"

BaseRenderStage::BaseRenderStage(
    const ShaderProgramHandle& shaderProgramHandle
):
    mShaderHandle(shaderProgramHandle)
{}

BaseOffscreenRenderStage::BaseOffscreenRenderStage(
    const ShaderProgramHandle& shaderProgramHandle, 
    const FramebufferHandle& framebufferHandle
):
    BaseRenderStage{shaderProgramHandle},
    mFramebufferHandle {framebufferHandle} 
{}

void BaseRenderStage::updateFloatParameter(const std::string& name, const float parameter) {
    mFloatParameters[name] = parameter;
}

void BaseRenderStage::updateIntParameter(const std::string& name, const int parameter) {
    mIntParameters[name] = parameter;
}

void BaseRenderStage::attachTexture(
    const std::string& name,
    const TextureHandle& textureHandle
) {
    mTextureAttachments.insert_or_assign(name, textureHandle);
}

void BaseRenderStage::attachMesh(
    const std::string& name,
    const MeshHandle& meshHandle
) {
    mMeshAttachments.insert_or_assign(name, meshHandle);
    meshHandle.getResource().associateShaderProgram(mShaderHandle);
}

void BaseRenderStage::attachModel(
    const std::string& name,
    const ModelHandle& modelHandle
) {
    mModelAttachments.insert_or_assign(name, modelHandle);
    modelHandle.getResource().associateShaderProgram(mShaderHandle);
}

void BaseRenderStage::attachLightCollection(
    const std::string& name,
    const LightCollectionHandle& lightCollectionHandle
) {
    mLightCollectionAttachments.insert_or_assign(name, lightCollectionHandle);
    lightCollectionHandle.getResource().associateShaderProgram(mShaderHandle);
}

float BaseRenderStage::getFloatParameter(const std::string& name) {
    return mFloatParameters.at(name);
}

int BaseRenderStage::getIntParameter(const std::string& name) {
    return mIntParameters.at(name);
}

TextureHandle BaseRenderStage::getTexture(const std::string& name) {
    return mTextureAttachments.at(name);
}

MeshHandle BaseRenderStage::getMesh(const std::string& name) {
    return mMeshAttachments.at(name);
}

ModelHandle BaseRenderStage::getModel(const std::string& name) {
    return mModelAttachments.at(name);
}

LightCollectionHandle BaseRenderStage::getLightCollection(const std::string& name) {
    return mLightCollectionAttachments.at(name);
}

void GeometryRenderStage::validate() {
    /*
     * Three colour buffers corresponding to position, normal, albedospec (for now)
     */
    assert(3 == mFramebufferHandle.getResource().getColorBufferHandles().size());
    assert(mFramebufferHandle.getResource().hasRBO());
    /*
     * TODO: more geometry pass related assertions
     */
    return;
}

void GeometryRenderStage::execute() {
    mShaderHandle.getResource().use();
    mFramebufferHandle.getResource().bind();
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_BLEND);
        glDisable(GL_FRAMEBUFFER_SRGB);

        for(const auto& modelAttachment: mModelAttachments) {
            modelAttachment.second.getResource().draw(mShaderHandle);
        }

    mFramebufferHandle.getResource().unbind();
}

void LightingRenderStage::validate() {
    assert(mTextureAttachments.find("positionMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("normalMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("albedoSpecularMap") != mTextureAttachments.end());
    assert(mIntParameters.find("screenWidth") != mIntParameters.end());
    assert(mIntParameters.find("screenHeight") != mIntParameters.end());
    assert(mFramebufferHandle.getResource().hasRBO());
    assert(mFramebufferHandle.getResource().getColorBufferHandles().size() >= 1);
    /*
     * TODO: more assertions related to the lighting stage
     */
}

void LightingRenderStage::execute() {
    mShaderHandle.getResource().use();
    mFramebufferHandle.getResource().bind();
        std::vector<std::string> gBufferAliases {
            {"positionMap"}, {"normalMap"}, {"albedoSpecularMap"}
        };
        for(int i{0}; i < 3; ++i) {
            mTextureAttachments.at(gBufferAliases[i]).getResource().bind(i);
        }


        mShaderHandle.getResource().setUInt("uGeometryPositionMap", 0);
        mShaderHandle.getResource().setUInt("uGeometryNormalMap", 1);
        mShaderHandle.getResource().setUInt("uGeometryAlbedoSpecMap", 2);

        mShaderHandle.getResource().setUInt(
            "uScreenWidth",
            mIntParameters.at("screenWidth")
        );
        mShaderHandle.getResource().setUInt(
            "uScreenHeight",
            mIntParameters.at("screenHeight")
        );


        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glClear(GL_COLOR_BUFFER_BIT);
        for(auto& lightCollectionAttachment : mLightCollectionAttachments) {
            lightCollectionAttachment.second.getResource().draw(mShaderHandle);
        }
    mFramebufferHandle.getResource().unbind();
}

void BlurRenderStage::validate() {
    assert(2 == mFramebufferHandle.getResource().getColorBufferHandles().size());
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("unblurredImage") != mTextureAttachments.end());
    assert(mIntParameters.find("nPasses") != mIntParameters.end());
    assert(mIntParameters.at("nPasses")>=2);
    /*
     * TODO: assert various other things related to rendering a blur
     */
}

void BlurRenderStage::execute() {
    mShaderHandle.getResource().use();
    mFramebufferHandle.getResource().bind();
        const std::vector<GLenum> drawbufferEnums {
            {GL_COLOR_ATTACHMENT0},
            {GL_COLOR_ATTACHMENT1}
        };
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);
        glDrawBuffers(2, drawbufferEnums.data()); // clear both buffers
        glClear(GL_COLOR_BUFFER_BIT);

        const int nPasses {  mIntParameters.at("nPasses") };
        mTextureAttachments.at("unblurredImage").getResource().bind(0);
        mShaderHandle.getResource().setUInt("uGenericTexture", 0);
        mShaderHandle.getResource().setUBool("uHorizontal", false);

        // In the first pass, draw to the pong buffer, then 
        // alternate between ping and pong
        std::vector<TextureHandle> pingpongBuffers {
            mFramebufferHandle.getResource().getColorBufferHandles()
        };
        MeshHandle screenMeshHandle { mMeshAttachments.at("screenMesh") };
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        for(int i{0}; i < nPasses; ++i) {
            // Blur our image along one axis
            screenMeshHandle.getResource().draw(mShaderHandle, 1);

            // Prepare for the next pass, flipping color buffers
            // and axis
            pingpongBuffers[(1+i)%2].getResource().bind(0);
            mShaderHandle.getResource().setUBool("uHorizontal", (1+i)%2);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i%2);
        }
    mFramebufferHandle.getResource().unbind();
}

void TonemappingRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("litScene") != mTextureAttachments.end());
    assert(mTextureAttachments.find("bloomEffect") != mTextureAttachments.end());
    assert(mFramebufferHandle.getResource().getColorBufferHandles().size() >= 1);
    assert(mFloatParameters.find("exposure") != mFloatParameters.end());
    assert(mFloatParameters.find("gamma") != mFloatParameters.end());
    assert(mIntParameters.find("combine") != mIntParameters.end());
}

void TonemappingRenderStage::execute() {
    mShaderHandle.getResource().use();
    mFramebufferHandle.getResource().bind();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClear(GL_COLOR_BUFFER_BIT);
        mTextureAttachments.at("litScene").getResource().bind(0);
        mTextureAttachments.at("bloomEffect").getResource().bind(1);
        mShaderHandle.getResource().setUInt("uGenericTexture", 0);
        mShaderHandle.getResource().setUInt("uGenericTexture1", 1);
        mShaderHandle.getResource().setUFloat("uExposure", mFloatParameters.at("exposure"));
        mShaderHandle.getResource().setUFloat("uGamma", mFloatParameters.at("gamma"));
        mShaderHandle.getResource().setUInt("uCombine", mIntParameters.at("combine"));
        mMeshAttachments.at("screenMesh").getResource().draw(mShaderHandle, 1);
    mFramebufferHandle.getResource().unbind();
}

void ScreenRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("renderSource") != mTextureAttachments.end());
}

void ScreenRenderStage::execute() {
    mShaderHandle.getResource().use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    //glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    mTextureAttachments.at("renderSource").getResource().bind(0);
    mShaderHandle.getResource().setUInt("uGenericTexture", 0);
    mMeshAttachments.at("screenMesh").getResource().draw(mShaderHandle, 1);
}
