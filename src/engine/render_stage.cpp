#include <iostream>

#include "resource_database.hpp"
#include "texture.hpp"
#include "shader_program.hpp"
#include "framebuffer.hpp"
#include "light.hpp"

#include "render_stage.hpp"

BaseRenderStage::BaseRenderStage(
    const std::string& shaderFilepath
) : mShaderHandle {nullptr}
{
    if(!ResourceDatabase::hasResourceDescription(shaderFilepath)){
        nlohmann::json shaderDescription {
            {"name", shaderFilepath},
            {"type", ShaderProgram::getResourceTypeName()},
            {"method", ShaderProgramFromFile::getResourceConstructorName()},
            {"parameters", {
                {"path", shaderFilepath}
            }}
        };
        ResourceDatabase::addResourceDescription(shaderDescription);
    }
    mShaderHandle = ResourceDatabase::getResource<ShaderProgram>(shaderFilepath);
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
    std::shared_ptr<StaticMesh> meshHandle
) {
    mMeshAttachments.insert_or_assign(name, meshHandle);
}

std::shared_ptr<StaticMesh> BaseRenderStage::getMesh(const std::string& name) {
    return mMeshAttachments.at(name);
}

void BaseRenderStage::attachTexture(
    const std::string& name,
    std::shared_ptr<Texture> textureHandle
) {
    mTextureAttachments.insert_or_assign(name, textureHandle);
}

std::shared_ptr<Texture> BaseRenderStage::getTexture(const std::string& name) {
    return mTextureAttachments.at(name);
}

void BaseRenderStage::attachMaterial(
    const std::string& name,
    std::shared_ptr<Material> materialHandle
) {
    mMaterialAttachments.insert_or_assign(name, materialHandle);
}

std::shared_ptr<Material> BaseRenderStage::getMaterial(const std::string& name) {
    return mMaterialAttachments.at(name);
}

void BaseRenderStage::submitToRenderQueue(OpaqueRenderUnit renderUnit) {
    mOpaqueMeshQueue.push(renderUnit);
}

void BaseRenderStage::submitToRenderQueue(LightRenderUnit renderLightUnit) {
    mLightQueue.push(renderLightUnit);
}

void BaseOffscreenRenderStage::declareRenderTarget(const std::string& name, unsigned int index) {
    assert(index < mFramebufferHandle->getColorBufferHandles().size());
    mRenderTargets.insert_or_assign(name, index);
}

std::shared_ptr<Texture> BaseOffscreenRenderStage::getRenderTarget(const std::string& name) {
    return mFramebufferHandle->getColorBufferHandles()[mRenderTargets.at(name)];
}

void GeometryRenderStage::setup() {
    if(!ResourceDatabase::hasResourceDescription("geometryFramebuffer")) {
        nlohmann::json framebufferDescription {
            {"name", "geometryFramebuffer"},
            {"type", Framebuffer::getResourceTypeName()},
            {"method", FramebufferFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"nColorAttachments", 3},
                {"dimensions", {
                    800, 600
                }},
                {"useRBO", true },
                {"colorBufferDefinitions",{
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_UNSIGNED_BYTE,
                        .mComponentCount=4
                    })
                }},
            }}
        };
        ResourceDatabase::addResourceDescription(framebufferDescription);
    }
    mFramebufferHandle = ResourceDatabase::getResource<Framebuffer>("geometryFramebuffer");

    declareRenderTarget("geometryPosition", 0);
    declareRenderTarget("geometryNormal", 1);
    declareRenderTarget("geometryAlbedoSpecular", 2);

    mShaderHandle->use();
    mShaderHandle->setUniformBlock("Matrices", 0);
    if(!ResourceDatabase::hasResourceDescription("PlainWhite")) {
        ResourceDatabase::addResourceDescription({
            {"name", "PlainWhite"},
            {"type", Texture::getResourceTypeName()},
            {"method", TextureFromColorBufferDefinition::getResourceConstructorName()},
            {"parameters", 
                colorBufferDefinitionToJSON(ColorBufferDefinition{})
            }
        });
    }
    std::shared_ptr<Texture> plainWhite = ResourceDatabase::getResource<Texture>("PlainWhite");
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
    assert(3 == mFramebufferHandle->getColorBufferHandles().size());
    assert(mFramebufferHandle->hasRBO());
    /*
     * TODO: more geometry pass related assertions
     */
    return;
}

void GeometryRenderStage::execute() {
    mShaderHandle->use();
    mFramebufferHandle->bind();
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

            OpaqueRenderUnit first { mOpaqueMeshQueue.top() };
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
            mShaderHandle->use();
            glBindVertexArray(mVertexArrayObject);
                first.mMeshHandle->bind(vertexLayout);
                BuiltinModelMatrixAllocator modelMatrixAllocator{instanceData};
                modelMatrixAllocator.bind(BuiltinModelMatrixLayout);


                mShaderHandle->setUFloat(
                    "uMaterial.mSpecularExponent",
                    first.mMaterialHandle->getFloatProperty("specularExponent")
                );

                first.mMaterialHandle->getTextureProperty("textureAlbedo")->bind(0);
                mShaderHandle->setUInt("uMaterial.mTextureAlbedo", 0);
                mShaderHandle->setUBool(
                    "uMaterial.mUsingAlbedoMap",
                    first.mMaterialHandle->getIntProperty("usesTextureAlbedo")
                );

                first.mMaterialHandle->getTextureProperty("textureSpecular")->bind(1);
                mShaderHandle->setUInt("uMaterial.mTextureSpecular", 1);
                mShaderHandle->setUBool(
                    "uMaterial.mUsingSpecularMap",
                    first.mMaterialHandle->getIntProperty("usesTextureSpecular")
                );

                first.mMaterialHandle->getTextureProperty("textureNormal")->bind(2);
                mShaderHandle->setUInt("uMaterial.mTextureNormal", 2);
                mShaderHandle->setUBool(
                    "uMaterial.mUsingNormalMap", 
                    first.mMaterialHandle->getIntProperty("usesTextureNormal")
                );

                glDrawElementsInstanced(
                    GL_TRIANGLES, first.mMeshHandle->getElementCount(),
                    GL_UNSIGNED_INT, nullptr, instanceData.size()
                );
            glBindVertexArray(0);
        }

    mFramebufferHandle->unbind();
}

void LightingRenderStage::setup() {
    if(!ResourceDatabase::hasResourceDescription("lightingFramebuffer")) {
        nlohmann::json framebufferDescription {
            {"name", "lightingFramebuffer"},
            {"type", Framebuffer::getResourceTypeName()},
            {"method", FramebufferFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"nColorAttachments", 2},
                {"dimensions", {
                    800, 600
                }},
                {"useRBO", true },
                {"colorBufferDefinitions",{
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                }},
            }}
        };
        ResourceDatabase::addResourceDescription(framebufferDescription);
    }
    mFramebufferHandle = ResourceDatabase::getResource<Framebuffer>("lightingFramebuffer");
    declareRenderTarget("litScene", 0);
    declareRenderTarget("brightCutoff", 1);
    mShaderHandle->use();
    mShaderHandle->setUniformBlock("Matrices", 0);

    Material::RegisterFloatProperty("brightCutoff", 1.f);
    Material::RegisterIntProperty("screenWidth", 800);
    Material::RegisterIntProperty("screenHeight", 600);
}

void LightingRenderStage::validate() {
    assert(mTextureAttachments.find("positionMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("normalMap") != mTextureAttachments.end());
    assert(mTextureAttachments.find("albedoSpecularMap") != mTextureAttachments.end());
    assert(mFramebufferHandle->hasRBO());
    assert(mFramebufferHandle->getColorBufferHandles().size() >= 1);
    /*
     * TODO: more assertions related to the lighting stage
     */
}

void LightingRenderStage::execute() {
    mShaderHandle->use();
    mFramebufferHandle->bind();
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glClear(GL_COLOR_BUFFER_BIT);

        while(!mLightQueue.empty()) {
            std::vector<glm::mat4> modelMatrices {};
            std::vector<LightEmissionData> lightEmissionList {};

            LightRenderUnit first {mLightQueue.top()};
            mLightQueue.pop();

            modelMatrices.push_back(first.mModelMatrix);
            lightEmissionList.push_back(first.mLightAttributes);

            while(
                !mLightQueue.empty() 
                && mLightQueue.top().mMeshHandle == first.mMeshHandle
                && mLightQueue.top().mMaterialHandle == first.mMaterialHandle
            ) {
                const LightRenderUnit renderLightUnit {mLightQueue.top()};
                mLightQueue.pop();

                modelMatrices.push_back(renderLightUnit.mModelMatrix);
                lightEmissionList.push_back(renderLightUnit.mLightAttributes);
            }


            mShaderHandle->use();
            std::vector<std::string> gBufferAliases {
                {"positionMap"}, {"normalMap"}, {"albedoSpecularMap"}
            };
            for(int i{0}; i < 3; ++i) {
                mTextureAttachments.at(gBufferAliases[i])->bind(i);
            }
            mShaderHandle->setUInt("uGeometryPositionMap", 0);
            mShaderHandle->setUInt("uGeometryNormalMap", 1);
            mShaderHandle->setUInt("uGeometryAlbedoSpecMap", 2);
            mShaderHandle->setUInt(
                "uScreenWidth",
                first.mMaterialHandle->getIntProperty("screenWidth")
            );
            mShaderHandle->setUInt(
                "uScreenHeight",
                first.mMaterialHandle->getIntProperty("screenHeight")
            );

            glBindVertexArray(mVertexArrayObject);
                first.mMeshHandle->bind(VertexLayout{{
                    {"position", LOCATION_POSITION, 4, GL_FLOAT}
                }});
                BuiltinModelMatrixAllocator modelMatrixAllocator{ modelMatrices };
                LightInstanceAllocator lightInstanceAllocator{ lightEmissionList, modelMatrices };
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
                    GL_TRIANGLES, first.mMeshHandle->getElementCount(), 
                    GL_UNSIGNED_INT, nullptr, lightEmissionList.size()
                );
            glBindVertexArray(0);
        }
    mFramebufferHandle->unbind();
}

void BlurRenderStage::setup() {
    if(!ResourceDatabase::hasResourceDescription("bloomFramebuffer")) {
        nlohmann::json framebufferDescription {
            {"name", "bloomFramebuffer"},
            {"type", Framebuffer::getResourceTypeName()},
            {"method", FramebufferFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"nColorAttachments", 2},
                {"dimensions", {
                    800, 600
                }},
                {"useRBO", false},
                {"colorBufferDefinitions",{
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_FLOAT,
                        .mComponentCount=4
                    }),
                }},
            }}
        };
        ResourceDatabase::addResourceDescription(framebufferDescription);
    }
    if(!ResourceDatabase::hasResourceDescription("screenRectangleMesh")) {
        nlohmann::json rectangleMeshDefinition {
            {"name", "screenRectangleMesh"},
            {"type", StaticMesh::getResourceTypeName()},
            {"method", StaticMeshRectangleDimensions::getResourceConstructorName()},
            {"parameters", {
                {"width", 2.f},
                {"height", 2.f},
            }}
        };
        ResourceDatabase::addResourceDescription(rectangleMeshDefinition);
    }
    mFramebufferHandle = ResourceDatabase::getResource<Framebuffer>("bloomFramebuffer");

    declareRenderTarget("pingBuffer", 0);
    declareRenderTarget("pongBuffer", 1);
    attachMesh("screenMesh", ResourceDatabase::getResource<StaticMesh>("screenRectangleMesh"));
    attachMaterial("screenMaterial",
        std::make_shared<Material>()
    );
    Material::RegisterIntProperty("nBlurPasses", 12);
}

void BlurRenderStage::validate() {
    assert(2 == mFramebufferHandle->getColorBufferHandles().size());
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("unblurredImage") != mTextureAttachments.end());
    /*
     * TODO: assert various other things related to rendering a blur
     */
}

void BlurRenderStage::execute() {
    mShaderHandle->use();
    mFramebufferHandle->bind();
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
            .at("screenMaterial")->getIntProperty("nBlurPasses")
        };
        mTextureAttachments.at("unblurredImage")->bind(0);
        mShaderHandle->setUInt("uGenericTexture", 0);
        mShaderHandle->setUBool("uHorizontal", false);
        std::shared_ptr<StaticMesh> screenMeshHandle { mMeshAttachments.at("screenMesh") };

        // In the first pass, draw to the pong buffer, then 
        // alternate between ping and pong
        std::vector<std::shared_ptr<Texture>> pingpongBuffers {
            mFramebufferHandle->getColorBufferHandles()
        };

        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        // Blur our image along one axis
        glBindVertexArray(mVertexArrayObject);
            screenMeshHandle->bind({{
                {"position", LOCATION_POSITION, 4, GL_FLOAT},
                {"UV1", LOCATION_UV1, 2, GL_FLOAT}
            }});
            for(int i{0}; i < nPasses; ++i) {
                glDrawElements(
                    GL_TRIANGLES, screenMeshHandle->getElementCount(), 
                    GL_UNSIGNED_INT, nullptr
                );

                // Prepare for the next pass, flipping color buffers
                // and axis
                pingpongBuffers[(1+i)%2]->bind(0);
                mShaderHandle->setUBool("uHorizontal", (1+i)%2);
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + i%2);
            }
        glBindVertexArray(0);
    mFramebufferHandle->unbind();
}

void TonemappingRenderStage::setup() {
    if(!ResourceDatabase::hasResourceDescription("tonemappingFramebuffer")) {
        nlohmann::json framebufferDescription {
            {"name", "tonemappingFramebuffer"},
            {"type", Framebuffer::getResourceTypeName()},
            {"method", FramebufferFromDescription::getResourceConstructorName()},
            {"parameters", {
                {"nColorAttachments", 1},
                {"dimensions", {
                    800, 600
                }},
                {"useRBO", false},
                {"colorBufferDefinitions",{
                    colorBufferDefinitionToJSON({
                        .mDataType=GL_UNSIGNED_BYTE,
                        .mComponentCount=4
                    }),
                }},
            }}
        };
        ResourceDatabase::addResourceDescription(framebufferDescription);
    }
    if(!ResourceDatabase::hasResourceDescription("screenRectangleMesh")) {
        nlohmann::json rectangleMeshDefinition {
            {"name", "screenRectangleMesh"},
            {"type", StaticMesh::getResourceTypeName()},
            {"method", StaticMeshRectangleDimensions::getResourceConstructorName()},
            {"parameters", {
                {"width", 2.f},
                {"height", 2.f},
            }}
        };
        ResourceDatabase::addResourceDescription(rectangleMeshDefinition);
    }
    mFramebufferHandle = ResourceDatabase::getResource<Framebuffer>("tonemappingFramebuffer");
    declareRenderTarget("tonemappedScene", 0);
    attachMesh("screenMesh", ResourceDatabase::getResource<StaticMesh>("screenRectangleMesh"));
    attachMaterial("screenMaterial",
        std::make_shared<Material>()
    );

    Material::RegisterFloatProperty("exposure", 1.f);
    Material::RegisterFloatProperty("gamma", 2.2f);
    Material::RegisterIntProperty("combine", true);
}

void TonemappingRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("litScene") != mTextureAttachments.end());
    assert(mTextureAttachments.find("bloomEffect") != mTextureAttachments.end());
    assert(mFramebufferHandle->getColorBufferHandles().size() >= 1);
}

void TonemappingRenderStage::execute() {
    mShaderHandle->use();
    std::shared_ptr<Material> screenMaterial {getMaterial("screenMaterial")};
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT);
    mFramebufferHandle->bind();
        mTextureAttachments.at("litScene")->bind(0);
        mTextureAttachments.at("bloomEffect")->bind(1);
        mShaderHandle->setUInt("uGenericTexture", 0);
        mShaderHandle->setUInt("uGenericTexture1", 1);
        mShaderHandle->setUFloat("uExposure", 
            screenMaterial->getFloatProperty("exposure")
        );
        mShaderHandle->setUFloat("uGamma", 
            screenMaterial->getFloatProperty("gamma")
        );
        mShaderHandle->setUInt("uCombine", 
            screenMaterial->getIntProperty("combine")
        );
        glBindVertexArray(mVertexArrayObject);
            mMeshAttachments.at("screenMesh")->bind({{
                {"position", LOCATION_POSITION, 4, GL_FLOAT},
                {"UV1", LOCATION_UV1, 2, GL_FLOAT}
            }});
            glDrawElementsInstanced(
                GL_TRIANGLES,
                mMeshAttachments.at("screenMesh")->getElementCount(),
                GL_UNSIGNED_INT,
                nullptr,
                1
            );
        glBindVertexArray(0);
    mFramebufferHandle->unbind();
}

void ScreenRenderStage::setup() {
    if(!ResourceDatabase::hasResourceDescription("screenRectangleMesh")) {
        nlohmann::json rectangleMeshDefinition {
            {"name", "screenRectangleMesh"},
            {"type", StaticMesh::getResourceTypeName()},
            {"method", StaticMeshRectangleDimensions::getResourceConstructorName()},
            {"parameters", {
                {"width", 2.f},
                {"height", 2.f},
            }}
        };
        ResourceDatabase::addResourceDescription(rectangleMeshDefinition);
    }
    attachMesh("screenMesh", ResourceDatabase::getResource<StaticMesh>("screenRectangleMesh"));
}

void ScreenRenderStage::validate() {
    assert(mMeshAttachments.find("screenMesh") != mMeshAttachments.end());
    assert(mTextureAttachments.find("renderSource") != mTextureAttachments.end());
}

void ScreenRenderStage::execute() {
    mShaderHandle->use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
    // glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(mVertexArrayObject);
        mTextureAttachments.at("renderSource")->bind(0);
        mShaderHandle->setUInt("uGenericTexture", 0);
        mMeshAttachments.at("screenMesh")->bind({{
            {"position", LOCATION_POSITION, 4, GL_FLOAT},
            {"color", LOCATION_COLOR, 4, GL_FLOAT},
            {"UV1", LOCATION_UV1, 2, GL_FLOAT}
        }});
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mMeshAttachments.at("screenMesh")->getElementCount(),
            GL_UNSIGNED_INT,
            nullptr,
            1
        );
    glBindVertexArray(0);
}
