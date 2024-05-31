#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

#include "light.hpp"

LightCollection::LightCollection() {
    allocateBuffers();
}
LightCollection::LightCollection(LightCollection&& other) {
    stealResources(other);
}
LightCollection::LightCollection(const LightCollection& other) {
    copyResources(other);
}
LightCollection& LightCollection::operator=(LightCollection&& other) {
    if(&other == this) return *this;

    destroyResource();
    stealResources(other);

    return *this;
}
LightCollection& LightCollection::operator=(const LightCollection& other) {
    if(&other == this) return *this;

    destroyResource();
    copyResources(other);

    return *this;
}

void LightCollection::draw(ShaderProgramHandle shaderProgramHandle) {
    updateBuffers();

    shaderProgramHandle.getResource().use();
    mLightVolume.getResource().draw(shaderProgramHandle, mInstanceLightMap.size());
}

void LightCollection::associateShaderProgram(ShaderProgramHandle shaderProgramHandle) {
    glUseProgram(shaderProgramHandle.getResource().getProgramID());
    //TODO: set light and matrix instance pointers for this shader+mesh's
    mLightVolume.getResource().associateShaderProgram(shaderProgramHandle);

    GLuint shaderVAO { mLightVolume.getResource().getShaderVAO(shaderProgramHandle) };
    glBindVertexArray(shaderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mModelMatrixBuffer);
            GLint attrModelMatrix { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrModelMatrix") };
            for(int i{0}; i < 4; ++i) {
                glEnableVertexAttribArray(attrModelMatrix+i);
                glVertexAttribPointer(
                    attrModelMatrix+i,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(glm::mat4),
                    reinterpret_cast<void*>(i * sizeof(glm::vec4))
                );
                glVertexAttribDivisor(attrModelMatrix+i, 1);
            }
        glBindBuffer(GL_ARRAY_BUFFER, mLightBuffer);
            GLint attrLightPlacementPosition { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightPlacement.mPosition") };
            GLint attrLightPlacementDirection { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightPlacement.mDirection") };
            GLint attrLightEmissionDiffuseColor { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mDiffuseColor") };
            GLint attrLightEmissionSpecularColor { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mSpecularColor") };
            GLint attrLightEmissionAmbientColor { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mAmbientColor") };
            GLint attrLightEmissionType { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mType") };
            GLint attrLightEmissionDecayLinear { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mDecayLinear") };
            GLint attrLightEmissionDecayQuadratic { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mDecayQuadratic") };
            GLint attrLightEmissionCosCutoffInner { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mCosCutoffInner") };
            GLint attrLightEmissionCosCutoffOuter { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrLightEmission.mCosCutoffOuter") };

            if(attrLightPlacementPosition >= 0) {
                glEnableVertexAttribArray(attrLightPlacementPosition);
                glVertexAttribPointer(attrLightPlacementPosition, 4, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mPosition)));
                glVertexAttribDivisor(attrLightPlacementPosition, 1);
            }
            if(attrLightPlacementDirection >= 0) {
                glEnableVertexAttribArray(attrLightPlacementDirection);
                glVertexAttribPointer(attrLightPlacementDirection, 4, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mDirection)));
                glVertexAttribDivisor(attrLightPlacementDirection, 1);
            }
            if(attrLightEmissionDiffuseColor >= 0) {
                glEnableVertexAttribArray(attrLightEmissionDiffuseColor);
                glVertexAttribPointer(attrLightEmissionDiffuseColor, 4, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mDiffuseColor)));
                glVertexAttribDivisor(attrLightEmissionDiffuseColor, 1);
            }
            if(attrLightEmissionSpecularColor >= 0) {
                glEnableVertexAttribArray(attrLightEmissionSpecularColor);
                glVertexAttribPointer(attrLightEmissionSpecularColor, 4, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mSpecularColor)));
                glVertexAttribDivisor(attrLightEmissionSpecularColor, 1);
            }
            if(attrLightEmissionAmbientColor >= 0) {
                glEnableVertexAttribArray(attrLightEmissionAmbientColor);
                glVertexAttribPointer(attrLightEmissionAmbientColor, 4, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mAmbientColor)));
                glVertexAttribDivisor(attrLightEmissionAmbientColor, 1);
            }
            if(attrLightEmissionType >= 0) {
                glEnableVertexAttribArray(attrLightEmissionType);
                glVertexAttribIPointer(attrLightEmissionType, 1, GL_INT, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mType)));
                glVertexAttribDivisor(attrLightEmissionType, 1);
            }
            if(attrLightEmissionDecayLinear >= 0) {
                glEnableVertexAttribArray(attrLightEmissionDecayLinear);
                glVertexAttribPointer(attrLightEmissionDecayLinear, 1, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mDecayLinear)));
                glVertexAttribDivisor(attrLightEmissionDecayLinear, 1);
            }
            if(attrLightEmissionDecayQuadratic >= 0) {
                glEnableVertexAttribArray(attrLightEmissionDecayQuadratic);
                glVertexAttribPointer(attrLightEmissionDecayQuadratic, 1, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mDecayQuadratic)));
                glVertexAttribDivisor(attrLightEmissionDecayQuadratic, 1);
            }
            if(attrLightEmissionCosCutoffInner >= 0) {
                glEnableVertexAttribArray(attrLightEmissionCosCutoffInner);
                glVertexAttribPointer(attrLightEmissionCosCutoffInner, 1, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mCosCutoffInner)));
                glVertexAttribDivisor(attrLightEmissionCosCutoffInner, 1);
            }
            if(attrLightEmissionCosCutoffOuter >= 0) {
                glEnableVertexAttribArray(attrLightEmissionCosCutoffOuter);
                glVertexAttribPointer(attrLightEmissionCosCutoffOuter, 1, GL_FLOAT, GL_FALSE, sizeof(Light), reinterpret_cast<void*>(offsetof(Light, mCosCutoffOuter)));
                glVertexAttribDivisor(attrLightEmissionCosCutoffOuter, 1);
            }
    glBindVertexArray(0);
}
void LightCollection::disassociateShaderProgram(ShaderProgramHandle shaderProgramHandle) {
    mLightVolume.getResource().disassociateShaderProgram(shaderProgramHandle);
}

GLuint LightCollection::addLight(const Light& light) {
    GLuint newInstanceID;
    if(!mDeletedInstanceIDs.empty()) {
        newInstanceID = mDeletedInstanceIDs.front();
        mDeletedInstanceIDs.pop();
    } else newInstanceID = mNextInstanceID++;

    float sqrt2 {glm::sqrt(2.f)};

    glm::mat4 modelMatrix { 
        glm::scale(
            glm::translate(glm::mat4(1.f), glm::vec3(light.mPosition)),
            light.mType != Light::LightType::directional? 
                glm::vec3(light.calculateRadius(0.05f)):
                // scale the sphere so that it encompasses the entire screen for directional
                //lights
                glm::vec3(sqrt2, sqrt2, 1.f)
        )
    };

    mInstanceModelMatrixMap[newInstanceID] = modelMatrix;
    mInstanceLightMap[newInstanceID] = light;
    mDirty = true;

    return newInstanceID;
}
Light LightCollection::getLight(GLuint instanceID) const {
    const auto& retPair {mInstanceLightMap.find(instanceID)};
    if(retPair == mInstanceLightMap.end()) return Light{};
    return retPair->second;
}
void LightCollection::updateLight(GLuint instanceID, const Light& light) {
    if(mInstanceLightMap.find(instanceID) == mInstanceLightMap.end()) return;
    float sqrt2 {glm::sqrt(2.f)};

    glm::mat4 modelMatrix { 
        glm::scale(
            glm::translate(glm::mat4(1.f), glm::vec3(light.mPosition)),
            light.mType != Light::LightType::directional?
                glm::vec3(light.calculateRadius(0.05f)):
                // scale the sphere so it encompasses the entire screen, for directional lights
                glm::vec3(sqrt2, sqrt2, 1.f)
        )
    };

    mInstanceLightMap[instanceID] = light;
    mInstanceModelMatrixMap[instanceID] = modelMatrix;
    mDirty = true;

    return;
}
void LightCollection::removeLight(GLuint instanceID) {
    if(mInstanceLightMap.find(instanceID) == mInstanceLightMap.end()) return;

    mInstanceLightMap.erase(instanceID);
    mInstanceModelMatrixMap.erase(instanceID);
    mDeletedInstanceIDs.push(instanceID);

    mDirty = true;
}

void LightCollection::destroyResource() {
    glDeleteBuffers(1, &mModelMatrixBuffer);
    glDeleteBuffers(1, &mLightBuffer);
    mModelMatrixBuffer = 0;
    mLightBuffer = 0;
    mNextInstanceID = 0;
    mDeletedInstanceIDs = {};
    mInstanceModelMatrixMap = {};
    mInstanceLightMap = {};
    mDirty = true;
}

void LightCollection::updateBuffers() {
    if(!mDirty) return;

    // Resize buffers if required
    if(mInstanceLightMap.size() > mInstanceCapacity){ 
        mInstanceCapacity *= 2;
        allocateBuffers();
    }

    // Build a list of lights, model matrices
    std::vector<Light> lights(mInstanceLightMap.size());
    std::vector<glm::mat4> modelMatrices(mInstanceLightMap.size());
    std::size_t currIndex {0};
    for(const std::pair<const GLuint, Light>& instanceLightPair: mInstanceLightMap) {
        GLuint instanceID { instanceLightPair.first };
        lights[currIndex] =   instanceLightPair.second;
        modelMatrices[currIndex] = mInstanceModelMatrixMap[instanceID];
        ++currIndex;
    }

    // Send new light data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, mModelMatrixBuffer);
        glBufferSubData(
            GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4),
            modelMatrices.data()
        );
    glBindBuffer(GL_ARRAY_BUFFER, mLightBuffer);
        glBufferSubData(
            GL_ARRAY_BUFFER, 0, lights.size() * sizeof(Light), 
            lights.data()
        );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mDirty = false;
}

void LightCollection::allocateBuffers() {
    if(!mModelMatrixBuffer) {
        glGenBuffers(1, &mModelMatrixBuffer);
    }
    if(!mLightBuffer) {
        glGenBuffers(1, &mLightBuffer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mModelMatrixBuffer);
        glBufferData(GL_ARRAY_BUFFER, mInstanceCapacity * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, mLightBuffer);
        glBufferData(GL_ARRAY_BUFFER, mInstanceCapacity * sizeof(Light), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void LightCollection::releaseResource() {
    // Prevent ourself from destroying our release resourced when its destructor
    // is called
    mModelMatrixBuffer = 0;
    mLightBuffer = 0;
}

void LightCollection::stealResources(LightCollection& other) {
    mModelMatrixBuffer = other.mModelMatrixBuffer;
    mLightBuffer = other.mLightBuffer;
    std::swap(mLightVolume, other.mLightVolume);
    mNextInstanceID = other.mNextInstanceID;
    std::swap(mDeletedInstanceIDs, other.mDeletedInstanceIDs);
    std::swap(mInstanceModelMatrixMap, other.mInstanceModelMatrixMap);
    std::swap(mInstanceLightMap, other.mInstanceLightMap);
    mInstanceCapacity = other.mInstanceCapacity;
    mDirty = other.mDirty;

    // Prevent other from destroying our resources when its destructor
    // is called
    other.releaseResource();
}

void LightCollection::copyResources(const LightCollection& other) {
    mLightVolume = other.mLightVolume;
    mNextInstanceID = other.mNextInstanceID;
    mDeletedInstanceIDs = other.mDeletedInstanceIDs;
    mInstanceModelMatrixMap = other.mInstanceModelMatrixMap;
    mInstanceLightMap = other.mInstanceLightMap;
    mInstanceCapacity = other.mInstanceCapacity;
    mDirty = true;

    allocateBuffers();
}

float Light::calculateRadius(float intensityCutoff) const {
    float intensityMax = mDiffuseColor.r;
    if(mDiffuseColor.g > intensityMax) intensityMax = mDiffuseColor.g;
    if(mDiffuseColor.b > intensityMax) intensityMax = mDiffuseColor.b;
    float radiusCutoff {
        (
            -mDecayLinear
            + std::sqrt(
                mDecayLinear*mDecayLinear - 4.f * mDecayQuadratic * (1.f - intensityMax / intensityCutoff)
            )
        )
        / (2.f * mDecayQuadratic)
    };
    return radiusCutoff;
}

Light Light::MakeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient){
    return Light{
        .mType { Light::directional },
        .mDirection { direction, 0.f },
        .mDiffuseColor { diffuse, 1.f },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {0.f},
        .mDecayQuadratic {0.f},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light Light::MakePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst){
    return Light{
        .mType { Light::point },
        .mPosition { position, 1.f },
        .mDiffuseColor { diffuse, 1.f, },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {linearConst},
        .mDecayQuadratic {quadraticConst},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light Light::MakeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst) {
    return Light{
        .mType { Light::spot },
        .mPosition { position, 1.f },
        .mDirection { direction, 0.f },
        .mDiffuseColor { diffuse, 1.f },
        .mSpecularColor { specular, 1.f },
        .mAmbientColor{ ambient, 1.f },
        .mDecayLinear {linearConst},
        .mDecayQuadratic {quadraticConst},
        .mCosCutoffInner {glm::cos(glm::radians(innerAngle))},
        .mCosCutoffOuter {glm::cos(glm::radians(outerAngle))}
    };
}
