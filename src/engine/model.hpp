#ifndef ZOMODEL_H
#define ZOMODEL_H

#include <vector>
#include <string>
#include <map>
#include <unordered_set>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>

#include "vertex.hpp"
#include "mesh_manager.hpp"
#include "texture_manager.hpp"
#include "shader_program_manager.hpp"
#include "material_manager.hpp"
#include "resource_manager.hpp"

/*
 * A class that
 *  a) Loads models from their respective 3D model files
 *  b) Stores references to all the meshes used by the model
 *  c) Stores the hierarchical relationship between the meshes
 *  d) Stores material properties used by shaders for each mesh
 */
class StaticModel : IResource {
public:
    struct TreeNode {
        std::vector<GLint> mMeshIndices {};

        TreeNode* mpParent {nullptr};
        std::vector<TreeNode*> mpChildren {};
    };

    StaticModel();

    /* Load a model from the specified file path */
    StaticModel(const std::string& filepath);
    /* Create a mesh out of a list of textures and vertices, store them in this model */
    StaticModel(const std::vector<BuiltinVertexData>& vertices, const std::vector<GLuint>& elements, const std::vector<TextureHandle>& textureHandles);
    /* Create a model with a single mesh */
    StaticModel(const MeshHandle& meshHandle);

    /* Model destructor */
    ~StaticModel();

    /* Move constructor */
    StaticModel(StaticModel&& other);
    /* Copy constructor */
    StaticModel(const StaticModel& other);

    /* Move assignment */
    StaticModel& operator=(StaticModel&& other);
    /* Copy assignment */
    StaticModel& operator=(const StaticModel& other);

    std::vector<MeshHandle> getMeshHandles() const;
    std::vector<MaterialHandle> getMaterialHandles() const;

private:
    /*
     * Meshes that make up this model
     */
    std::vector<MeshHandle> mMeshHandles {};
    /*
     * The materials that correspond to each mesh on this model
     */
    std::vector<MaterialHandle> mMaterialHandles {};
    /*
     * Root node leading to full hierarchy (of meshes)
     */
    TreeNode* mpHierarchyRoot {nullptr};

    /* 
     * Utility method for destroying resources associated with
     * this model
     */
    void free();

    /*
     * Utility method for taking resources from another instance of
     * this class 
     */
    void stealResources(StaticModel& other);

    /*
     * Utility method for deeply replicating resources from another 
     * instance of this class
     */
    void copyResources(const StaticModel& other);

    void deleteTree(StaticModel::TreeNode* pRootNode);
    TreeNode* copyTree(const TreeNode* pRootNode, StaticModel::TreeNode* pParentNode=nullptr);

    TreeNode* processAssimpNode(TreeNode* pParentNode, aiNode* pAiNode, const aiScene* pAiScene);
    void processAssimpMesh(aiMesh* pAiMesh, const aiScene* pAiScene);

    std::vector<TextureHandle> loadAssimpTextures(aiMaterial* pAiMaterial, aiTextureType textureType);

    void destroyResource() override;
    void releaseResource() override;

friend class ResourceManager<StaticModel>;
};

#endif
