#include <iostream>
#include <vector>
#include <utility>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "window_context_manager.hpp"
#include "material_manager.hpp"
#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "mesh_manager.hpp"
#include "vertex.hpp"
#include "model.hpp"

glm::mat4 buildModelMatrix(glm::vec3 position, glm::quat orientation, glm::vec3 scale);

void Model::draw(ShaderProgramHandle shaderProgramHandle) {
    updateBuffers();
    shaderProgramHandle.getResource().use();
    for(std::size_t i {0}; i < mMeshHandles.size(); ++i) {
        mMaterialHandles[i].getResource().bind(shaderProgramHandle);
        mMeshHandles[i].getResource().draw(shaderProgramHandle, mInstanceModelMatrixMap.size());
    }
}

Model::Model() {
    allocateBuffers();
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<GLuint>& elements, const std::vector<TextureHandle>& textureHandles) :
    Model()
{
    mpHierarchyRoot = new Model::TreeNode {};
    mpHierarchyRoot->mMeshIndices.push_back(0);

    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource(
            "",
            {textureHandles, 18.f}
        )
    );

    mMeshHandles.push_back(
        MeshManager::getInstance().registerResource(
            "",
            { vertices, elements }
        )
    );
}

Model::Model(const MeshHandle& meshHandle) : Model() {
    mpHierarchyRoot = new Model::TreeNode {};
    mpHierarchyRoot->mMeshIndices.push_back(0);
    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource(
            "", {{}, 18.f}
        )
    );
    mMeshHandles.push_back(
        MeshManager::getInstance().registerResource(
            "",
            Mesh{ meshHandle.getResource() }
        )
    );
}

glm::mat4 Model::getInstance(GLuint instanceID) {
    if(mInstanceModelMatrixMap.find(instanceID) != mInstanceModelMatrixMap.end())
        return mInstanceModelMatrixMap[instanceID];
    return glm::mat4{0.f};
}

Model::Model(const std::string& filepath): Model() {
    Assimp::Importer* pImporter { WindowContextManager::getInstance().getAssetImporter() };
    const aiScene* pAiScene {
        pImporter->ReadFile(
            filepath,
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
        )
    };

    if(
        !pAiScene 
        || (pAiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        || !(pAiScene->mRootNode)
    ) {
        std::cout << "ERROR::ASSIMP:: " << pImporter->GetErrorString() << std::endl;
        return;
    }

    mpHierarchyRoot = processAssimpNode(nullptr, pAiScene->mRootNode, pAiScene);
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

void Model::updateBuffers() {
    if(!mDirty) return;

    // Double the matrix buffer capacity if we already have more instances than space
    if(mInstanceModelMatrixMap.size() > mInstanceCapacity){
        mInstanceCapacity *= 2;
        allocateBuffers();
    }

    // Build a list out of the currently instanced model matrices
    std::vector<glm::mat4> modelMatrices { mInstanceModelMatrixMap.size() };
    int currIndex {0};
    for(const std::pair<const GLuint, glm::mat4>& matrix : mInstanceModelMatrixMap) {
        modelMatrices[currIndex ++] = matrix.second;
    }

    // Move everything in the list to our matrix buffer
    glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * mInstanceModelMatrixMap.size(), modelMatrices.data());
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
    mMeshHandles.clear();
    mMaterialHandles.clear();
    deleteTree(mpHierarchyRoot);
    mpHierarchyRoot = nullptr;
}

void Model::stealResources(Model& other) {
    std::swap(mMeshHandles, other.mMeshHandles);

    mMatrixBuffer = other.mMatrixBuffer;
    mMaterialHandles = other.mMaterialHandles;
    mInstanceModelMatrixMap = other.mInstanceModelMatrixMap;
    mDeletedInstanceIDs = other.mDeletedInstanceIDs;
    mNextInstanceID = other.mNextInstanceID;
    mInstanceCapacity = other.mInstanceCapacity;
    mpHierarchyRoot = other.mpHierarchyRoot;
    mDirty = other.mDirty;

    // Prevent other from destroying our resources when its
    // destructor is called
    other.releaseResource();
}

void Model::copyResources(const Model& other) {
    mMeshHandles = other.mMeshHandles;

    mInstanceModelMatrixMap = other.mInstanceModelMatrixMap;
    mMaterialHandles = other.mMaterialHandles;
    mDeletedInstanceIDs = other.mDeletedInstanceIDs;
    mNextInstanceID = other.mNextInstanceID;
    mInstanceCapacity = other.mInstanceCapacity;
    mDirty = true;

    mpHierarchyRoot = copyTree(other.mpHierarchyRoot);
    allocateBuffers();
}


void Model::allocateBuffers() {
    if(!mMatrixBuffer) {
        glGenBuffers(1, &mMatrixBuffer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(glm::mat4) * mInstanceCapacity,
            NULL, GL_DYNAMIC_DRAW
        );
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::deleteTree(Model::TreeNode* pRootNode) {
    if(!pRootNode) return;

    for(Model::TreeNode* pChild : pRootNode->mpChildren) {
        deleteTree(pChild);
    }

    delete pRootNode;
}

Model::TreeNode* Model::copyTree(const Model::TreeNode* pRootNode, Model::TreeNode* pParentNode) {
    if(!pRootNode) return nullptr;
    Model::TreeNode* pNewRootNode {
        new Model::TreeNode {
            pRootNode->mMeshIndices,
            pParentNode
        }
    };
    for(Model::TreeNode* pChild : pRootNode->mpChildren) {
        pNewRootNode->mpChildren.push_back(
            copyTree(pChild, pNewRootNode)
        );
    }
    return pNewRootNode;
}

Model::TreeNode* Model::processAssimpNode(Model::TreeNode* pParentNode, aiNode* pAiNode, const aiScene* pAiScene) {
    Model::TreeNode* pNewNode {
        new Model::TreeNode {
            {},
            pParentNode
        }
    };
    for(std::size_t  i{0}; i < pAiNode->mNumMeshes; ++i) {
        aiMesh* pAiMesh {
            pAiScene->mMeshes[pAiNode->mMeshes[i]]
        };
        int meshIndex { static_cast<int>(mMeshHandles.size()) };
        processAssimpMesh(pAiMesh, pAiScene);
        pNewNode->mMeshIndices.push_back(meshIndex);
    }

    for(std::size_t i{0}; i < pAiNode->mNumChildren; ++i) {
        pNewNode->mpChildren.push_back(
            processAssimpNode(pNewNode, pAiNode->mChildren[i], pAiScene)
        );
    }
    return pNewNode;
}

void Model::processAssimpMesh(aiMesh* pAiMesh, const aiScene* pAiScene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> elements;
    std::vector<TextureHandle> textureHandles;

    // Load vertex data
    for(std::size_t i{0}; i < pAiMesh->mNumVertices; ++i) {
        vertices.push_back({
            {
                pAiMesh->mVertices[i].x,
                pAiMesh->mVertices[i].y,
                pAiMesh->mVertices[i].z,
                1.f
            },
            {
                pAiMesh->mNormals[i].x,
                pAiMesh->mNormals[i].y,
                pAiMesh->mNormals[i].z,
                0.f
            },
            {
                pAiMesh->mTangents[i].x,
                pAiMesh->mTangents[i].y,
                pAiMesh->mTangents[i].z,
                0.f
            },
            { 1.f, 1.f, 1.f, 1.f },
            {
                pAiMesh->mTextureCoords[0][i].x,
                pAiMesh->mTextureCoords[0][i].y
            }
        });
    }

    // Load element array
    for(std::size_t i{0}; i < pAiMesh->mNumFaces; ++i) {
        aiFace face = pAiMesh->mFaces[i];
        for(std::size_t elementIndex{0}; elementIndex < face.mNumIndices; ++elementIndex) {
            elements.push_back(face.mIndices[elementIndex]);
        }
    }

    mMeshHandles.push_back(
        MeshManager::getInstance().registerResource(
            "",
            {vertices, elements}
        )
    );

    // Load textures
    if(pAiMesh->mMaterialIndex >= 0) {
        aiMaterial* pAiMaterial {
            pAiScene->mMaterials[pAiMesh->mMaterialIndex]
        };

        std::vector<TextureHandle> textureHandlesAlbedo{
            loadAssimpTextures(pAiMaterial, Texture::Albedo)
        };
        std::vector<TextureHandle> textureHandlesNormal {
            loadAssimpTextures(pAiMaterial, Texture::Normal)
        };
        std::vector<TextureHandle> textureHandlesSpecular {
            loadAssimpTextures(pAiMaterial, Texture::Specular)
        };

        textureHandles.insert(textureHandles.end(), textureHandlesAlbedo.begin(), textureHandlesAlbedo.end());
        textureHandles.insert(textureHandles.end(), textureHandlesNormal.begin(), textureHandlesNormal.end());
        textureHandles.insert(textureHandles.end(), textureHandlesSpecular.begin(), textureHandlesSpecular.end());
    }

    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource(
            "", {textureHandles, 18.f}
        )
    );
}

std::vector<TextureHandle> Model::loadAssimpTextures(aiMaterial* pAiMaterial, Texture::Usage usage) {
    // Determine assimp's representation of type of textures being loaded
    std::map<Texture::Usage, aiTextureType> kUsageTypeMap {
        { Texture::Albedo, aiTextureType_DIFFUSE },
        { Texture::Normal, aiTextureType_NORMALS },
        { Texture::Specular, aiTextureType_SPECULAR }
    };
    aiTextureType type { kUsageTypeMap[usage] };

    // build up a list of texture pointers, adding textures
    // to this model if it isn't already present
    std::vector<TextureHandle> textureHandles {};
    std::size_t textureCount {
        pAiMaterial->GetTextureCount(
            kUsageTypeMap[usage]
        )
    };
    for(std::size_t i{0}; i < textureCount; ++i) {
        aiString aiTextureName;
        pAiMaterial->GetTexture(type, i, &aiTextureName);
        std::string textureName { aiTextureName.C_Str() };
        textureHandles.push_back(
            TextureManager::getInstance().registerResource(
                textureName,
                Texture(textureName, usage)
            )
        );
    }

    return textureHandles;
}

void Model::associateShaderProgram(ShaderProgramHandle shaderProgramHandle) {
    for(const MeshHandle& meshHandle: mMeshHandles) {
        // TODO: This needs to be more robust somehow
        // Its okay to assume for now that since this model owns these meshes, our meshes won't
        // be associated with a shader until this model associates them
        GLuint shaderVAO = meshHandle.getResource().getShaderVAO(shaderProgramHandle);
        if(shaderVAO) continue;

        meshHandle.getResource().associateShaderProgram(shaderProgramHandle);
        shaderVAO = meshHandle.getResource().getShaderVAO(shaderProgramHandle);
        glBindVertexArray(shaderVAO);
            // Enable and set matrix pointers
            glBindBuffer(GL_ARRAY_BUFFER, mMatrixBuffer);
            GLint attrModelMatrix { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrModelMatrix") };
            GLint attrNormalMatrix { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrNormalMatrix") };
            if(attrModelMatrix >= 0) {
                for(int i {0}; i < 4; ++i) {
                    glEnableVertexAttribArray(attrModelMatrix+i);
                    glVertexAttribPointer(attrModelMatrix+i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(i*sizeof(glm::vec4)));
                    glVertexAttribDivisor(attrModelMatrix+i, 1);
                }
            }
            if(attrNormalMatrix >= 0) {
                // TODO: both model and normal matrices are presently the same. Add proper normal matrix allocation
                // an option in this class, in case accuracy is desired later
                for(int i{0}; i < 4; ++i) {
                    glEnableVertexAttribArray(attrNormalMatrix+i);
                    glVertexAttribPointer(attrNormalMatrix+i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(i*sizeof(glm::vec4)));
                    glVertexAttribDivisor(attrNormalMatrix+i, 1);
                }
            }
        glBindVertexArray(0);
    }
}

void Model::disassociateShaderProgram(ShaderProgramHandle shaderProgramHandle) {
    for(const MeshHandle& meshHandle: mMeshHandles) {
        meshHandle.getResource().disassociateShaderProgram(shaderProgramHandle);
    }
}

void Model::destroyResource() {
    free();
}

void Model::releaseResource() {
    // Prevent this from destroying its resources when its
    // destructor is called. 
    mMatrixBuffer = 0;
    mpHierarchyRoot = nullptr;
}
