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
#include "texture.hpp"
#include "mesh.hpp"

class Model {
public:
    struct TreeNode {
        std::vector<GLint> mMeshIndices {};

        TreeNode* mpParent {nullptr};
        std::vector<TreeNode*> mpChildren {};
    };

    /* load a model from the specified file path */
    Model(const std::string& filepath);
    /* create a mesh out of a list of textures and vertices, store them in this model */
    Model(const std::vector<Vertex>& vertices, const std::vector<GLuint>& elements, std::vector<Texture>&& textures);
    /* create a model with a single mesh */
    Model(const Mesh& mesh);

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
    void associateShaderProgram(GLuint programID);
    /* Removes VAO for a given shader program */
    void disassociateShaderProgram(GLuint programID);


    void draw(ShaderProgram& shaderProgram);

private:
    Model();

    /* updates buffers stored in GPU */
    void updateBuffers();

    /* 
    utility method for destroying resources associated with
    this model
    */
    void free();
    /*
    utility method for taking resources from another instance of
    this class 
    */
    void stealResources(Model& other);
    /*
    utility method for deeply replicating resources from another 
    instance of this class
    */
    void copyResources(const Model& other);
    /*
    utility method for allocating buffers
    */
    void allocateBuffers();

    void deleteTree(Model::TreeNode* pRootNode);
    TreeNode* copyTree(const TreeNode* pRootNode, Model::TreeNode* pParentNode=nullptr);

    TreeNode* processAssimpNode(TreeNode* pParentNode, aiNode* pAiNode, const aiScene* pAiScene);
    Mesh processAssimpMesh(aiMesh* pAiMesh, const aiScene* pAiScene);

    std::vector<Texture*> loadAssimpTextures(aiMaterial* pAiMaterial, Texture::Usage usage);

    /*
    Textures used by meshes belonging to this model
    */
    std::vector<Texture> mTextures {};
    /*
    pointers to this model's textures, indexed by their names
    */
    std::map<const std::string, Texture*> mLoadedTextures {};
    /*
    Meshes that make up this model
    */
    std::vector<Mesh> mMeshes {};
    /*
    root node leading to full hierarchy (of meshes)
    */
    TreeNode* mpHierarchyRoot {nullptr};
    /*
    string containing path to the model file
    */
    std::string mModelpath {""};

    /*
    ID of the matrix buffer managed by this object
    */
    GLuint mMatrixBuffer {0};
    std::map<GLuint, glm::mat4> mInstanceModelMatrixMap {};
    std::queue<GLuint> mDeletedInstanceIDs {};
    GLuint mNextInstanceID {0};
    GLuint mInstanceCapacity {128};
    bool mDirty {true};

};

#endif
