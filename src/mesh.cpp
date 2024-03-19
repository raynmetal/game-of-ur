#include <vector>
#include <map>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "texture.hpp"
#include "vertex.hpp"

#include "mesh.hpp"

glm::mat4 buildModelMatrix(glm::vec3 position, glm::quat orientation, glm::vec3 scale);

void Mesh::Draw(ShaderProgram& shaderProgram) {
    updateMatrixBuffer();
    associateShaderProgram(shaderProgram);
    shaderProgram.use();

    // Set uniforms
    if(mTextureAlbedo) {
        glActiveTexture(GL_TEXTURE0);
            mTextureAlbedo->bind();
        glActiveTexture(GL_TEXTURE0);
        shaderProgram.setUBool("uUsingAlbedoMap", true);
        shaderProgram.setUInt("uMaterial.textureAlbedo", 0);
    } else shaderProgram.setUBool("uUsingAlbedoMap", false);
    if(mTextureNormal){
        glActiveTexture(GL_TEXTURE1);
            mTextureNormal->bind();
        glActiveTexture(GL_TEXTURE0);
        shaderProgram.setUBool("uUsingNormalMap", true);
        shaderProgram.setUInt("uMaterial.textureNormal", 1);
    } else shaderProgram.setUBool("uUsingNormalMap", false);;
    if(mTextureSpecular) {
        glActiveTexture(GL_TEXTURE2);
            mTextureSpecular->bind();
        glActiveTexture(GL_TEXTURE0);
        shaderProgram.setUBool("uUsingSpecularMap", true);
        shaderProgram.setUInt("uMaterial.textureSpecular", 2);
    } else shaderProgram.setUBool("uUsingSpecularMap", false);

    glBindVertexArray(mShaderVAOMap[shaderProgram.getProgramID()]);
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mElements.size(),
            GL_UNSIGNED_INT,
            0,
            mInstanceModelMatrixMap.size()
        );
    glBindVertexArray(0);
}

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<GLuint>& elements,
    Texture* textureAlbedo,
    Texture* textureNormal,
    Texture* textureSpecular
) : 
    mVertices{vertices},
    mElements{elements},
    mTextureAlbedo{textureAlbedo},
    mTextureNormal{textureNormal},
    mTextureSpecular{textureSpecular}
{
    glGenBuffers(1, &mVertexBuffer);
    glGenBuffers(1, &mMatrixBuffer);
    glGenBuffers(1, &mElementBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), mVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        glBufferData(
            GL_ARRAY_BUFFER, mInstanceCapacity * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW
        );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mElements.size() * sizeof(GLuint),
            mElements.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mElementBuffer);
    glDeleteBuffers(1, &mMatrixBuffer);
    for(std::pair shaderVAO : mShaderVAOMap) {
        glDeleteBuffers(1, &shaderVAO.second);
    }
}

void Mesh::associateShaderProgram(const ShaderProgram& shaderProgram) {
    GLuint programID { shaderProgram.getProgramID() };

    //Ensure duplicate calls to this function don't create duplicate VAOs
    if(mShaderVAOMap.find(programID) != mShaderVAOMap.end()) return;
    GLuint shaderVAO;
    glGenVertexArrays(1, &shaderVAO);
    shaderProgram.use();
    glBindVertexArray(shaderVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);

        //Enable and set vertex pointers
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        GLint attrPosition { shaderProgram.getLocationAttribArray("attrPosition") };
        GLint attrNormal { shaderProgram.getLocationAttribArray("attrNormal") };
        GLint attrTangent { shaderProgram.getLocationAttribArray("attrTangent") };
        GLint attrColor { shaderProgram.getLocationAttribArray("attrColor") };
        GLint attrTextureCoordinates { shaderProgram.getLocationAttribArray("attrTextureCoordinates") };
        if(attrPosition >= 0) {
            shaderProgram.enableAttribArray(attrPosition);
            glVertexAttribPointer(attrPosition, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mPosition)));
        }
        if(attrNormal >= 0) {
            shaderProgram.enableAttribArray(attrNormal);
            glVertexAttribPointer(attrNormal, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mNormal)));
        }
        if(attrTangent >= 0) {
            shaderProgram.enableAttribArray(attrTangent);
            glVertexAttribPointer(attrTangent, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mTangent)));
        }
        if(attrColor >= 0) {
            shaderProgram.enableAttribArray(attrColor);
            glVertexAttribPointer(attrColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mColor)));
        }
        if(attrTextureCoordinates >= 0) {
            shaderProgram.enableAttribArray(attrTextureCoordinates);
            glVertexAttribPointer(attrTextureCoordinates, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mTextureCoordinates)));
        }


        // Enable and set matrix pointers
        glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        GLint attrModelMatrix { shaderProgram.getLocationAttribArray("attrModelMatrix") };
        GLint attrNormalMatrix { shaderProgram.getLocationAttribArray("attrNormalMatrix") };
        if(attrModelMatrix >= 0) {
            for(int i {0}; i < 4; ++i) {
                shaderProgram.enableAttribArray(attrModelMatrix+i);
                glVertexAttribPointer(attrModelMatrix+i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(i*sizeof(glm::vec4)));
                glVertexAttribDivisor(attrModelMatrix+i, 1);
            }
        }
        if(attrNormalMatrix >= 0) {
            // TODO: both model and normal matrices are presently the same. Add proper normal matrix allocation
            // an option in this class, in case accuracy is desired later
            for(int i{0}; i < 4; ++i) {
                shaderProgram.enableAttribArray(attrNormalMatrix+i);
                glVertexAttribPointer(attrNormalMatrix+i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(i*sizeof(glm::vec4)));
                glVertexAttribDivisor(attrNormalMatrix+i, 1);
            }
        }
    glBindVertexArray(0);

    mShaderVAOMap[programID] = shaderVAO;
}

void Mesh::disassociateShaderProgram(const ShaderProgram& shaderProgram) {
    GLuint programID {shaderProgram.getProgramID()};
    if(mShaderVAOMap.find(programID) == mShaderVAOMap.end()) return;

    glDeleteVertexArrays(1, &mShaderVAOMap[programID]);
    mShaderVAOMap.erase(programID);
}

GLuint Mesh::addInstance(glm::mat4 modelMatrix) {
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

GLuint Mesh::addInstance(glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    return addInstance(buildModelMatrix(position, orientation, scale));
}

void Mesh::updateInstance(GLuint instanceID, glm::mat4 modelMatrix) {
    if(mInstanceModelMatrixMap.find(instanceID) == mInstanceModelMatrixMap.end()) return;

    mInstanceModelMatrixMap[instanceID] = modelMatrix;
    mDirty = true;
}

void Mesh::updateInstance(GLuint instanceID, glm::vec3 position, glm::quat orientation, glm::vec3 scale) {
    updateInstance(instanceID, buildModelMatrix(position, orientation, scale));
}

void Mesh::removeInstance(GLuint instanceID) {
    if(mInstanceModelMatrixMap.find(instanceID) == mInstanceModelMatrixMap.end()) return;

    mDeletedInstanceIDs.push(instanceID);
    mInstanceModelMatrixMap.erase(instanceID);
    mDirty = true;
}

void Mesh::updateMatrixBuffer() {
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
