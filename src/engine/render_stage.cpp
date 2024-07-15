#include <iostream>
#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "framebuffer_manager.hpp"
#include "light.hpp"

#include "render_stage.hpp"

BaseRenderStage::BaseRenderStage(
    const std::string& shaderFilepath
) : mShaderHandle {
        ShaderProgramManager::getInstance().registerResource(
            shaderFilepath, {shaderFilepath}
        )
    }
{
    glGenVertexArrays(1, &mVertexArrayObject);
}

BaseRenderStage::~BaseRenderStage() {
    if(mVertexArrayObject){
        glDeleteVertexArrays(1, &mVertexArrayObject);
    }
}

BaseOffscreenRenderStage::BaseOffscreenRenderStage(
    const std::string& shaderFilepath
):
    BaseRenderStage{shaderFilepath}
{}

void BaseRenderStage::attachMesh(
    const std::string& name,
    const MeshHandle& meshHandle
) {
    mMeshAttachments.insert_or_assign(name, meshHandle);
}

MeshHandle BaseRenderStage::getMesh(const std::string& name) {
    return mMeshAttachments.at(name);
}

void BaseRenderStage::attachTexture(
    const std::string& name,
    const TextureHandle& textureHandle
) {
    mTextureAttachments.insert_or_assign(name, textureHandle);
}

TextureHandle BaseRenderStage::getTexture(const std::string& name) {
    return mTextureAttachments.at(name);
}

void BaseRenderStage::attachMaterial(
    const std::string& name,
    const MaterialHandle& materialHandle
) {
    mMaterialAttachments.insert_or_assign(name, materialHandle);
}

MaterialHandle BaseRenderStage::getMaterial(const std::string& name) {
    return mMaterialAttachments.at(name);
}

void BaseRenderStage::submitToRenderQueue(RenderUnit renderUnit) {
    mOpaqueMeshQueue.push(renderUnit);
}

void BaseRenderStage::submitToRenderQueue(RenderLightUnit renderLightUnit) {
    mLightQueue.push(renderLightUnit);
}

void BaseOffscreenRenderStage::declareRenderTarget(const std::string& name, unsigned int index) {
    assert(index < mFramebufferHandle.getResource().getColorBufferHandles().size());
    mRenderTargets.insert_or_assign(name, index);
}

TextureHandle BaseOffscreenRenderStage::getRenderTarget(const std::string& name) {
    return mFramebufferHandle.getResource().getColorBufferHandles()[mRenderTargets.at(name)];
}

void GeometryRenderStage::setup() {
    mFramebufferHandle = FramebufferManager::getInstance().registerResource(
        "geometryFramebuffer",
        {
            // TODO: Where do we get window dimensions from? Hardcoded for now
            {800, 600},
            3,
            {
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4},
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4},
                ColorBufferDefinition{.mDataType=GL_UNSIGNED_BYTE, .mComponentCount=4},
            },
            true
        }
    );
    declareRenderTarget("geometryPosition", 0);
    declareRenderTarget("geometryNormal", 1);
    declareRenderTarget("geometryAlbedoSpecular", 2);
    mShaderHandle.getResource().use();
    mShaderHandle.getResource().setUniformBlock("Matrices", 0);
    TextureHandle plainWhite {TextureManager::getInstance().registerResource(
        "PlainWhite", {}
    )};
    Material::RegisterTextureHandleProperty("textureAlbedo", plainWhite);
    Material::RegisterTextureHandleProperty("textureSpecular", plainWhite);
    Material::RegisterTextureHandleProperty("textureNormal", plainWhite);
    Material::RegisterFloatProperty("specularExponent", 14.f);
    Material::RegisterIntProperty("usesTextureAlbedo", false);
    Material::RegisterIntProperty("usesTextureSpecular", false);
    Material::RegisterIntProperty("usesTextureNormal", false);
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

        const VertexLayout vertexLayout {{
            { "position", LOCATION_POSITION, 4, GL_FLOAT },
            { "normal", LOCATION_NORMAL, 4, GL_FLOAT },
            { "tangent", LOCATION_TANGENT, 4, GL_FLOAT },
            { "color", LOCATION_COLOR, 4, GL_FLOAT },
            { "UV1", LOCATION_UV1, 2, GL_FLOAT }
        }};

        while(!mOpaqueMeshQueue.empty()) {
            std::vector<glm::mat4> instanceData{};

            RenderUnit first { mOpaqueMeshQueue.top() };
            instanceData.push_back(mOpaqueMeshQueue.top().mModelMatrix);
            mOpaqueMeshQueue.pop();

            while(
                !mOpaqueMeshQueue.empty() 
                && mOpaqueMeshQueue.top().mMeshHandle == first.mMeshHandle
                && mOpaqueMeshQueue.top().mMaterialHandle == first.mMaterialHandle
            ) {
                instanceData.push_back(mOpaqueMeshQueue.top().mModelMatrix);
                mOpaqueMeshQueue.pop();
            }

            // TODO: what is this mVertexArrayObject good for anyway, now?
            mShaderHandle.getResource().use();
            glBindVertexArray(mVertexArrayObject);
                first.mMeshHandle.getResource().bind(vertexLayout);
                BuiltinModelMatrixAllocator modelMatrixAllocator{instanceData};
                modelMatrixAllocator.bind(BuiltinModelMatrixLayout);


                mShaderHandle.getResource().setUFloat(
                    "uMaterial.mSpecularExponent",
                    first.mMaterialHandle.getResource().getFloatProperty("specularExponent")
                );

                first.mMaterialHandle.getResource().getTextureProperty("textureAlbedo").getResource().bind(0);
                mShaderHandle.getResource().setUInt("uMaterial.mTextureAlbedo", 0);
                mShaderHandle.getResource().setUBool(
                    "uMaterial.mUsingAlbedoMap",
                    first.mMaterialHandle.getResource().getIntProperty("usesTextureAlbedo")
                );

                first.mMaterialHandle.getResource().getTextureProperty("textureSpecular").getResource().bind(1);
                mShaderHandle.getResource().setUInt("uMaterial.mTextureSpecular", 1);
                mShaderHandle.getResource().setUBool(
                    "uMaterial.mUsingSpecularMap",
                    first.mMaterialHandle.getResource().getIntProperty("usesTextureSpecular")
                );

                first.mMaterialHandle.getResource().getTextureProperty("textureNormal").getResource().bind(2);
                mShaderHandle.getResource().setUInt("uMaterial.mTextureNormal", 2);
                mShaderHandle.getResource().setUBool(
                    "uMaterial.mUsingNormalMap", 
                    first.mMaterialHandle.getResource().getIntProperty("usesTextureNormal")
                );

                // glDrawElementsInstanced(
                //     GL_TRIANGLES, first.mMeshHandle.getResource().getElementCount(),
                //     GL_UNSIGNED_INT, nullptr, instanceData.size()
                // );

                glDrawElementsInstanced(
                    GL_TRIANGLES, first.mMeshHandle.getResource().getElementCount(),
                    GL_UNSIGNED_INT, nullptr, 1
                );
            glBindVertexArray(0);
        }

    mFramebufferHandle.getResource().unbind();
}

void LightingRenderStage::setup() {
    mFramebufferHandle = FramebufferManager::getInstance().registerResource(
        "lightingFramebuffer",
        {
            {800, 600},
            2,
            {
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4},
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4}
            },
            true
        }
    );

    declareRenderTarget("litScene", 0);
    declareRenderTarget("brightCutoff", 1);
    mShaderHandle.getResource().use();
    mShaderHandle.getResource().setUniformBlock("Matrices", 0);

    Material::RegisterFloatProperty("brightCutoff", 1.f);
    Material::RegisterIntProperty("screenWidth", 800);
    Material::RegisterIntProperty("screenHeight", 600);
}

void LightingRenderStage::validate() {
    assert(mTextureAttachments.find("positionMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("normalMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("albedoSpecularMap") != mTextureAttachments.end());
    assert(mFramebufferHandle.getResource().hasRBO());
    assert(mFramebufferHandle.getResource().getColorBufferHandles().size() >= 1);
    /*
     * TODO: more assertions related to the lighting stage
     */
}

void LightingRenderStage::execute() {
    mShaderHandle.getResource().use();
    mFramebufferHandle.getResource().bind();
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glClear(GL_COLOR_BUFFER_BIT);

        while(!mLightQueue.empty()) {
            std::vector<glm::mat4> modelMatrices {};
            std::vector<LightData> lightDataList {};

            RenderLightUnit first {mLightQueue.top()};
            mLightQueue.pop();

            modelMatrices.push_back(first.mModelMatrix);
            lightDataList.push_back(first.mLightAttributes);

            while(
                !mLightQueue.empty() 
                && mLightQueue.top().mMeshHandle == first.mMeshHandle
                && mLightQueue.top().mMaterialHandle == first.mMaterialHandle
            ) {
                const RenderLightUnit renderLightUnit {mLightQueue.top()};
                mLightQueue.pop();

                modelMatrices.push_back(renderLightUnit.mModelMatrix);
                lightDataList.push_back(renderLightUnit.mLightAttributes);
            }


            mShaderHandle.getResource().use();
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
                first.mMaterialHandle.getResource().getIntProperty("screenWidth")
            );
            mShaderHandle.getResource().setUInt(
                "uScreenHeight",
                first.mMaterialHandle.getResource().getIntProperty("screenHeight")
            );

            glBindVertexArray(mVertexArrayObject);
                first.mMeshHandle.getResource().bind(VertexLayout{{
                    {"position", LOCATION_POSITION, 4, GL_FLOAT}
                }});
                BuiltinModelMatrixAllocator modelMatrixAllocator{ modelMatrices };
                LightInstanceAllocator lightInstanceAllocator{ lightDataList };
                modelMatrixAllocator.bind(BuiltinModelMatrixLayout);
                lightInstanceAllocator.bind({{
                    {"attrLightPlacement.mPosition", RUNTIME, 4, GL_FLOAT},
                    {"attrLightPlacement.mDirection", RUNTIME, 4, GL_FLOAT},

                    {"attrLightEmission.mType", RUNTIME, 1, GL_INT},
                    {"attrLightEmission.mDiffuseColor", RUNTIME, 4, GL_FLOAT},
                    {"attrLightEmission.mSpecularColor", RUNTIME, 4, GL_FLOAT},
                    {"attrLightEmission.mAmbientColor", RUNTIME, 4, GL_FLOAT},
                    {"attrLightEmission.mDecayLinear", RUNTIME, 1, GL_FLOAT},
                    {"attrLightEmission.mDecayQuadratic", RUNTIME, 1, GL_FLOAT},
                    {"attrLightEmission.mCosCutoffInner", RUNTIME, 1, GL_FLOAT},
                    {"attrLightEmission.mCosCutoffOuter", RUNTIME, 1, GL_FLOAT}
                }});
                glDrawElementsInstanced(
                    GL_TRIANGLES, first.mMeshHandle.getResource().getElementCount(), 
                    GL_UNSIGNED_INT, nullptr, lightDataList.size()
                );
            glBindVertexArray(0);
        }
    mFramebufferHandle.getResource().unbind();
}

void BlurRenderStage::setup() {
    mFramebufferHandle = FramebufferManager::getInstance().registerResource(
        "bloomFramebuffer",
        {
            {800, 600},
            2,
            {
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4},
                ColorBufferDefinition{.mDataType=GL_FLOAT, .mComponentCount=4}
            },
            false
        }
    );
    declareRenderTarget("pingBuffer", 0);
    declareRenderTarget("pongBuffer", 1);
    attachMesh("screenMesh", generateRectangleMesh());
    attachMaterial("screenMaterial",
        MaterialManager::getInstance().registerResource("", {})
    );
    Material::RegisterIntProperty("nBlurPasses", 12);
}

void BlurRenderStage::validate() {
    assert(2 == mFramebufferHandle.getResource().getColorBufferHandles().size());
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("unblurredImage") != mTextureAttachments.end());
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

        const int nPasses {  mMaterialAttachments
            .at("screenMaterial").getResource().getIntProperty("nBlurPasses")
        };
        mTextureAttachments.at("unblurredImage").getResource().bind(0);
        mShaderHandle.getResource().setUInt("uGenericTexture", 0);
        mShaderHandle.getResource().setUBool("uHorizontal", false);
        MeshHandle screenMeshHandle { mMeshAttachments.at("screenMesh") };

        // In the first pass, draw to the pong buffer, then 
        // alternate between ping and pong
        std::vector<TextureHandle> pingpongBuffers {
            mFramebufferHandle.getResource().getColorBufferHandles()
        };

        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        // Blur our image along one axis
        glBindVertexArray(mVertexArrayObject);
            screenMeshHandle.getResource().bind({{
                {"position", LOCATION_POSITION, 4, GL_FLOAT},
                {"UV1", LOCATION_UV1, 2, GL_FLOAT}
            }});
            for(int i{0}; i < nPasses; ++i) {
                glDrawElements(
                    GL_TRIANGLES, screenMeshHandle.getResource().getElementCount(), 
                    GL_UNSIGNED_INT, nullptr
                );

                // Prepare for the next pass, flipping color buffers
                // and axis
                pingpongBuffers[(1+i)%2].getResource().bind(0);
                mShaderHandle.getResource().setUBool("uHorizontal", (1+i)%2);
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + i%2);
            }
        glBindVertexArray(0);
    mFramebufferHandle.getResource().unbind();
}

void TonemappingRenderStage::setup() {
    mFramebufferHandle = FramebufferManager::getInstance().registerResource(
        "tonemappingFramebuffer",
        {
            {800, 600},
            1,
            {
                {.mDataType=GL_UNSIGNED_BYTE, .mComponentCount=4}
            },
            true
        }
    );
    declareRenderTarget("tonemappedScene", 0);
    attachMesh("screenMesh", generateRectangleMesh());
    attachMaterial("screenMaterial",
        MaterialManager::getInstance().registerResource("", {})
    );

    Material::RegisterFloatProperty("exposure", 1.f);
    Material::RegisterFloatProperty("gamma", 2.2f);
    Material::RegisterIntProperty("combine", true);
}

void TonemappingRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("litScene") != mTextureAttachments.end());
    assert(mTextureAttachments.find("bloomEffect") != mTextureAttachments.end());
    assert(mFramebufferHandle.getResource().getColorBufferHandles().size() >= 1);

}

void TonemappingRenderStage::execute() {
    mShaderHandle.getResource().use();
    MaterialHandle screenMaterial {getMaterial("screenMaterial")};
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT);
    mFramebufferHandle.getResource().bind();
        mTextureAttachments.at("litScene").getResource().bind(0);
        mTextureAttachments.at("bloomEffect").getResource().bind(1);
        mShaderHandle.getResource().setUInt("uGenericTexture", 0);
        mShaderHandle.getResource().setUInt("uGenericTexture1", 1);
        mShaderHandle.getResource().setUFloat("uExposure", 
            screenMaterial.getResource().getFloatProperty("exposure")
        );
        mShaderHandle.getResource().setUFloat("uGamma", 
            screenMaterial.getResource().getFloatProperty("gamma")
        );
        mShaderHandle.getResource().setUInt("uCombine", 
            screenMaterial.getResource().getIntProperty("combine")
        );
        glBindVertexArray(mVertexArrayObject);
            mMeshAttachments.at("screenMesh").getResource().bind({{
                {"position", LOCATION_POSITION, 4, GL_FLOAT},
                {"UV1", LOCATION_UV1, 2, GL_FLOAT}
            }});
            glDrawElementsInstanced(
                GL_TRIANGLES,
                mMeshAttachments.at("screenMesh").getResource().getElementCount(),
                GL_UNSIGNED_INT,
                nullptr,
                1
            );
        glBindVertexArray(0);
    mFramebufferHandle.getResource().unbind();
}

void ScreenRenderStage::setup() {
    attachMesh("screenMesh", generateRectangleMesh());
}
void ScreenRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("renderSource") != mTextureAttachments.end());
}

void ScreenRenderStage::execute() {
    mShaderHandle.getResource().use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    // glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVertexArrayObject);
        mTextureAttachments.at("renderSource").getResource().bind(0);
        mShaderHandle.getResource().setUInt("uGenericTexture", 0);
        mMeshAttachments.at("screenMesh").getResource().bind({{
            {"position", LOCATION_POSITION, 4, GL_FLOAT},
            {"color", LOCATION_COLOR, 4, GL_FLOAT},
            {"UV1", LOCATION_UV1, 2, GL_FLOAT}
        }});
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mMeshAttachments.at("screenMesh").getResource().getElementCount(),
            GL_UNSIGNED_INT,
            nullptr,
            1
        );
    glBindVertexArray(0);
}
