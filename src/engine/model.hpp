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

class Model : IResource {
public:
    struct TreeNode {
        std::vector<GLint> mMeshIndices {};

        TreeNode* mpParent {nullptr};
        std::vector<TreeNode*> mpChildren {};
    };

    Model();

    /* Load a model from the specified file path */
    Model(const std::string& filepath);
    /* Create a mesh out of a list of textures and vertices, store them in this model */
    Model(const std::vector<Vertex>& vertices, const std::vector<GLuint>& elements, const std::vector<TextureHandle>& textureHandles);
    /* Create a model with a single mesh */
    Model(const MeshHandle& meshHandle);

    /* Model destructor */
    ~Model();

    /* Move constructor */
    Model(Model&& other);
    /* Copy constructor */
    Model(const Model& other);

    /* Move assignment */
    Model& operator=(Model&& other);
    /* Copy assignment */
    Model& operator=(const Model& other);

    /* Adds an instance based on model matrix provided as input */
    GLuint addInstance(glm::mat4 modelMatrix);
    /* Adds an instance based on position and scale vectors, and orientation quaternion */
    GLuint addInstance(glm::vec3 position, glm::quat orientation, glm::vec3 scale);
    /* Gets matrix transform for this instance*/
    glm::mat4 getInstance(GLuint instanceID);
    /* Replaces the transform associated with this instance with transform in input */
    void updateInstance(GLuint instanceID, glm::mat4 transform);
    /* Replaces the transform associated with this instance based on model matrix computed from input */
    void updateInstance(GLuint instanceID, glm::vec3 position, glm::quat orientation, glm::vec3 scale);
    /* Removes instance associated with instanceID (returned by the instance add function earlier)*/
    void removeInstance(GLuint instanceID);

    /* Sets up a VAO for a given shader program, setting pointers as necessary */
    void associateShaderProgram(ShaderProgramHandle shaderProgramHandle);
    /* Removes VAO for a given shader program */
    void disassociateShaderProgram(ShaderProgramHandle shaderProgramHandle);


    void draw(ShaderProgramHandle shaderProgramhandle);

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
     * ID of the matrix buffer managed by this object
     */
    GLuint mMatrixBuffer {0};
    /*
     * The transform to the current world position for a given instance, with
     * the instance ID as key
     */
    std::map<GLuint, glm::mat4> mInstanceModelMatrixMap {};
    /*
     * The IDs of deleted instances, to be reused by any new instances
     * of this model that may be instantiated later
     */
    std::queue<GLuint> mDeletedInstanceIDs {};
    GLuint mNextInstanceID {0};
    /*
     * The number of instances the memory on GPU presently allocated can store
     * for this model
     */
    GLuint mInstanceCapacity {128};
    /*
     * Whether there has been a change in position or number of instance(s) of this
     * model within this cycle, requiring fresh data to be sent to the GPU
     */
    bool mDirty {true};

    /* Updates buffers stored in GPU */
    void updateBuffers();

    /* 
     * Utility method for destroying resources associated with
     * this model
     */
    void free();
    /*
     * Utility method for taking resources from another instance of
     * this class 
     */
    void stealResources(Model& other);
    /*
     * Utility method for deeply replicating resources from another 
     * instance of this class
     */
    void copyResources(const Model& other);
    /*
     * Utility method for allocating buffers
     */
    void allocateBuffers();

    void deleteTree(Model::TreeNode* pRootNode);
    TreeNode* copyTree(const TreeNode* pRootNode, Model::TreeNode* pParentNode=nullptr);

    TreeNode* processAssimpNode(TreeNode* pParentNode, aiNode* pAiNode, const aiScene* pAiScene);
    void processAssimpMesh(aiMesh* pAiMesh, const aiScene* pAiScene);

    std::vector<TextureHandle> loadAssimpTextures(aiMaterial* pAiMaterial, Texture::Usage usage);

    void destroyResource() override;
    void releaseResource() override;

friend class ResourceManager<Model>;
};

#endif
