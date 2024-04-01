#include <vector>
#include <utility>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "vertex.hpp"
#include "mesh.hpp"
#include "model.hpp"

glm::mat4 buildModelMatrix(glm::vec3 position, glm::quat orientation, glm::vec3 scale);

void Model::draw(ShaderProgram& shaderProgram) {
    updateMatrixBuffer();

    for(Mesh& mesh : mMeshes) {
        mesh.associateShaderProgram(shaderProgram, mMatrixBuffer);
        mesh.draw(shaderProgram, mInstanceModelMatrixMap.size());
    }
}

Model::Model(): mMatrixBuffer{0} {
    allocateBuffers();
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<GLuint>& elements, std::vector<Texture>&& textures) :
    Model()
{
    std::swap(mTextures, textures);
    std::vector<Texture*> pTextures(mTextures.size());
    for(auto i{0}; i < mTextures.size(); ++i) {
        pTextures[i] = &mTextures[i];
    }
    mMeshes.push_back(
        Mesh{vertices, elements, pTextures}
    );
}

Model::~Model() {
    free();
}

Model::Model(Model&& other) {
    stealResources(other);
}
Model& Model::operator=(Model&& other) {
    if(this == &other) 
        return *this;

    free();
    stealResources(other);

    return *this;
}

Model::Model(const Model& other) {
    copyResources(other);
}
Model& Model::operator=(const Model& other) {
    if(this == &other)
        return *this;

    free();
    copyResources(other);

    return *this;
}

GLuint Model::addInstance(glm::mat4 modelMatrix) {
    // Determine the name of the newly added instance
    GLuint newInstanceID;
    if(!mDeletedInstanceIDs.empty()){
        newInstanceID = mDeletedInstanceIDs.front();
        mDeletedInstanceIDs.pop();
    } else newInstanceID = mNextInstanceID++;

    mInstanceModelMatrixMap[newInstanceID] = modelMatrix;
    mDirty = true;

    return newInstanceID;
}

GLuint Model::addInstance(glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    return addInstance(buildModelMatrix(position, orientation, scale));
}

void Model::updateInstance(GLuint instanceID, glm::mat4 modelMatrix) {
    if(mInstanceModelMatrixMap.find(instanceID) == mInstanceModelMatrixMap.end()) return;

    mInstanceModelMatrixMap[instanceID] = modelMatrix;
    mDirty = true;
}

void Model::updateInstance(GLuint instanceID, glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    updateInstance(instanceID, buildModelMatrix(position, orientation, scale));
}

void Model::removeInstance(GLuint instanceID) {
    if(mInstanceModelMatrixMap.find(instanceID) == mInstanceModelMatrixMap.end()) return;

    mDeletedInstanceIDs.push(instanceID);
    mInstanceModelMatrixMap.erase(instanceID);
    mDirty = true;
}

void Model::updateMatrixBuffer() {
    if(!mDirty) return;

    // Double the matrix buffer capacity if we already have more instances than space
    if(mInstanceModelMatrixMap.size() > mInstanceCapacity) mInstanceCapacity *= 2;

    // Build a list out of the currently instanced model matrices
    std::vector<glm::mat4> modelMatrices { mInstanceModelMatrixMap.size() };
    int currIndex {0};
    for(std::pair<GLuint, glm::mat4> matrix : mInstanceModelMatrixMap) {
        modelMatrices[currIndex ++] = matrix.second;
    }

    // Move everything in the list to our matrix buffer
    glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * mInstanceCapacity, modelMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Reset the dirty bool
    mDirty = false;
}

glm::mat4 buildModelMatrix(glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    glm::mat4 rotateMatrix { glm::normalize(orientation) };
    glm::mat4 translateMatrix { glm::translate(glm::mat4(1.f), position) };
    glm::mat4 scaleMatrix { glm::scale(glm::mat4(1.f), scale) };

    return translateMatrix * rotateMatrix * scaleMatrix;
}

void Model::free() {
    glDeleteBuffers(1, &mMatrixBuffer);
    mMatrixBuffer = 0;
    mTextures.clear();
    mMeshes.clear();
}

void Model::stealResources(Model& other) {
    std::swap(mMeshes, other.mMeshes);
    std::swap(mTextures, other.mTextures);

    mMatrixBuffer = other.mMatrixBuffer;
    mInstanceModelMatrixMap = other.mInstanceModelMatrixMap;
    mDeletedInstanceIDs = other.mDeletedInstanceIDs;
    mNextInstanceID = other.mNextInstanceID;
    mInstanceCapacity = other.mInstanceCapacity;
    mDirty = other.mDirty;

    // Prevent other from destroying our resources when its
    // destructor is called
    other.mMatrixBuffer = 0;
}
void Model::copyResources(const Model& other) {
    mMeshes = other.mMeshes;
    mTextures = other.mTextures;

    mInstanceModelMatrixMap = other.mInstanceModelMatrixMap;
    mDeletedInstanceIDs = other.mDeletedInstanceIDs;
    mNextInstanceID = other.mNextInstanceID;
    mInstanceCapacity = other.mInstanceCapacity;
    mDirty = true;

    allocateBuffers();
}


void Model::allocateBuffers() {
    if(!mMatrixBuffer) {
        glGenBuffers(1, &mMatrixBuffer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(glm::mat4) * mInstanceCapacity,
            NULL, GL_STATIC_DRAW
        );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
