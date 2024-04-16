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

    free();
    stealResources(other);

    return *this;
}
LightCollection& LightCollection::operator=(const LightCollection& other) {
    if(&other == this) return *this;

    free();
    copyResources(other);

    return *this;
}

void LightCollection::associateShader(GLuint programID) {
    //TODO: set light and matrix instance pointers for this shader+mesh's
    //  VAO
    mLightVolume.associateShaderProgram(programID);
}
void LightCollection::disassociateShader(GLuint programID) {
    mLightVolume.disassociateShaderProgram(programID);
}

GLuint LightCollection::addLight(const Light& light) {
    GLuint newInstanceID;
    if(!mDeletedInstanceIDs.empty()) {
        newInstanceID = mDeletedInstanceIDs.front();
        mDeletedInstanceIDs.pop();
    } else newInstanceID = mNextInstanceID++;

    glm::mat4 modelMatrix { 
        glm::scale(
            glm::translate(glm::mat4(1.f), light.mPosition),
            glm::vec3(light.calculateRadius(0.05f))
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

    glm::mat4 modelMatrix { 
        glm::scale(
            glm::translate(glm::mat4(1.f), light.mPosition),
            glm::vec3(light.calculateRadius(0.05f))
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

void LightCollection::free() {
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
    other.mModelMatrixBuffer = 0;
    other.mLightBuffer = 0;
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
    float intensityMax = mDiffuse.r;
    if(mDiffuse.g > intensityMax) intensityMax = mDiffuse.g;
    if(mDiffuse.b > intensityMax) intensityMax = mDiffuse.b;
    float radiusCutoff {
        (
            -mLinear
            + std::sqrt(
                mLinear*mLinear - 4.f * mQuadratic * (mConstant - intensityMax / intensityCutoff)
            )
        )
        / (2.f * mQuadratic)
    };
    return radiusCutoff;
}

Light Light::MakeDirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient){
    return Light{
        .mType { Light::directional },
        .mDirection { direction },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {0.f},
        .mQuadratic {0.f},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light Light::MakePointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst){
    return Light{
        .mType { Light::point },
        .mPosition { position },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {linearConst},
        .mQuadratic {quadraticConst},
        .mCosCutoffInner {0.f},
        .mCosCutoffOuter {0.f}
    };
}

Light Light::MakeSpotLight(const glm::vec3& position, const glm::vec3& direction, float innerAngle, float outerAngle, const glm::vec3& diffuse, const glm::vec3& specular, const glm::vec3& ambient, float linearConst, float quadraticConst) {
    return Light{
        .mType { Light::spot },
        .mPosition { position },
        .mDirection { direction },
        .mDiffuse { diffuse },
        .mSpecular { specular },
        .mAmbient{ ambient },
        .mConstant {1.f},
        .mLinear {linearConst},
        .mQuadratic {quadraticConst},
        .mCosCutoffInner {glm::cos(glm::radians(innerAngle))},
        .mCosCutoffOuter {glm::cos(glm::radians(outerAngle))}
    };
}
