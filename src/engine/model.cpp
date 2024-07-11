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

StaticModel::StaticModel() {
}

StaticModel::StaticModel(const std::vector<BuiltinVertexData>& vertices, const std::vector<GLuint>& elements, const std::vector<TextureHandle>& textureHandles) :
    StaticModel()
{
    mpHierarchyRoot = new StaticModel::TreeNode {};
    mpHierarchyRoot->mMeshIndices.push_back(0);

    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource(
            "",
            {}
        )
    );

    mMeshHandles.push_back(
        MeshManager::getInstance().registerResource(
            "",
            BuiltinMesh{ vertices, elements }
        )
    );
}

StaticModel::StaticModel(const MeshHandle& meshHandle) : StaticModel() {
    mpHierarchyRoot = new StaticModel::TreeNode {};
    mpHierarchyRoot->mMeshIndices.push_back(0);
    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource("", {})
    );
    mMeshHandles.push_back(meshHandle);
}

StaticModel::StaticModel(const std::string& filepath): StaticModel() {
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

StaticModel::~StaticModel() {
    free();
}

StaticModel::StaticModel(StaticModel&& other) {
    stealResources(other);
}
StaticModel& StaticModel::operator=(StaticModel&& other) {
    if(this == &other) 
        return *this;

    free();
    stealResources(other);

    return *this;
}

StaticModel::StaticModel(const StaticModel& other) {
    copyResources(other);
}
StaticModel& StaticModel::operator=(const StaticModel& other) {
    if(this == &other)
        return *this;

    free();
    copyResources(other);

    return *this;
}


std::vector<MeshHandle> StaticModel::getMeshHandles() const { return mMeshHandles; }
std::vector<MaterialHandle> StaticModel::getMaterialHandles() const { return mMaterialHandles; }

void StaticModel::free() {
    mMeshHandles.clear();
    mMaterialHandles.clear();
    deleteTree(mpHierarchyRoot);
    mpHierarchyRoot = nullptr;
}

void StaticModel::stealResources(StaticModel& other) {
    std::swap(mMeshHandles, other.mMeshHandles);

    mMaterialHandles = other.mMaterialHandles;
    mpHierarchyRoot = other.mpHierarchyRoot;

    // Prevent other from destroying our resources when its
    // destructor is called
    other.releaseResource();
}

void StaticModel::copyResources(const StaticModel& other) {
    mMeshHandles = other.mMeshHandles;
    mMaterialHandles = other.mMaterialHandles;
    mpHierarchyRoot = copyTree(other.mpHierarchyRoot);
}

void StaticModel::deleteTree(StaticModel::TreeNode* pRootNode) {
    if(!pRootNode) return;

    for(StaticModel::TreeNode* pChild : pRootNode->mpChildren) {
        deleteTree(pChild);
    }

    delete pRootNode;
}

StaticModel::TreeNode* StaticModel::copyTree(const StaticModel::TreeNode* pRootNode, StaticModel::TreeNode* pParentNode) {
    if(!pRootNode) return nullptr;
    StaticModel::TreeNode* pNewRootNode {
        new StaticModel::TreeNode {
            pRootNode->mMeshIndices,
            pParentNode
        }
    };
    for(StaticModel::TreeNode* pChild : pRootNode->mpChildren) {
        pNewRootNode->mpChildren.push_back(
            copyTree(pChild, pNewRootNode)
        );
    }
    return pNewRootNode;
}

StaticModel::TreeNode* StaticModel::processAssimpNode(StaticModel::TreeNode* pParentNode, aiNode* pAiNode, const aiScene* pAiScene) {
    StaticModel::TreeNode* pNewNode {
        new StaticModel::TreeNode {
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

void StaticModel::processAssimpMesh(aiMesh* pAiMesh, const aiScene* pAiScene) {

    std::vector<TextureHandle> textureHandles;

    mMeshHandles.push_back(
        MeshManager::getInstance().registerResource(
            "",
            BuiltinMesh{ pAiMesh }
        )
    );
    mMaterialHandles.push_back(
        MaterialManager::getInstance().registerResource(
            "", {}
        )
    );

    // Load textures
    if(pAiMesh->mMaterialIndex >= 0) {
        aiMaterial* pAiMaterial {
            pAiScene->mMaterials[pAiMesh->mMaterialIndex]
        };

        // TODO: Make associating textures with their respective material
        // props more elegant somehow?
        std::vector<TextureHandle> textureHandlesAlbedo {
            loadAssimpTextures(pAiMaterial, aiTextureType_DIFFUSE)
        };
        std::vector<TextureHandle> textureHandlesNormal {
            loadAssimpTextures(pAiMaterial, aiTextureType_NORMALS)
        };
        std::vector<TextureHandle> textureHandlesSpecular {
            loadAssimpTextures(pAiMaterial, aiTextureType_SPECULAR)
        };

        if(!textureHandlesAlbedo.empty()) {
            mMaterialHandles.back().getResource().updateTextureProperty(
                "textureAlbedo", textureHandlesAlbedo.back()
            );
            mMaterialHandles.back().getResource().updateIntProperty(
                "usesTextureAlbedo", true
            );
        }
        if(!textureHandlesNormal.empty()) {
            mMaterialHandles.back().getResource().updateTextureProperty(
                "textureNormal", textureHandlesNormal.back()
            );
            mMaterialHandles.back().getResource().updateIntProperty(
                "usesTextureNormal", true
            );
        }
        if(!textureHandlesSpecular.empty()) {
            mMaterialHandles.back().getResource().updateTextureProperty(
                "textureSpecular", textureHandlesSpecular.back()
            );
            mMaterialHandles.back().getResource().updateIntProperty(
                "usesTextureSpecular", true
            );
        }
    }
}

std::vector<TextureHandle> StaticModel::loadAssimpTextures(aiMaterial* pAiMaterial, aiTextureType textureType) {
    // build up a list of texture pointers, adding textures
    // to this model if it isn't already present
    std::vector<TextureHandle> textureHandles {};
    std::size_t textureCount {
        pAiMaterial->GetTextureCount(
            textureType
        )
    };

    for(std::size_t i{0}; i < textureCount; ++i) {
        aiString aiTextureName;
        pAiMaterial->GetTexture(textureType, i, &aiTextureName);
        std::string textureName { aiTextureName.C_Str() };
        textureHandles.push_back(
            TextureManager::getInstance().registerResource(
                textureName, Texture(textureName)
            )
        );
    }

    return textureHandles;
}

void StaticModel::destroyResource() {
    free();
}

void StaticModel::releaseResource() {
    // Prevent this from destroying its resources when its
    // destructor is called. 
    mpHierarchyRoot = nullptr;
}
