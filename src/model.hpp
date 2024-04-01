#ifndef ZOMODEL_H
#define ZOMODEL_H

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "texture.hpp"
#include "mesh.hpp"

struct ModelNode {
};

class Model {
public:
    // /* load a model from the specified file path */
    // Model(const std::string& filepath);
    /* create a mesh out of a list of textures and vertices, store them in this model */
    Model(const std::vector<Vertex>& vertices, const std::vector<GLuint>& elements, std::vector<Texture>&& textures);

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

    /* Replaces the transform associated with this instance with transform in input */
    void updateInstance(GLuint instanceID, glm::mat4 transform);
    /* Replaces the transform associated with this instance based on model matrix computed from input */
    void updateInstance(GLuint instanceID, glm::vec3 position, glm::quat orientation, glm::vec3 scale);
    /* Removes instance associated with instanceID (returned by the instance add function earlier)*/
    void removeInstance(GLuint instanceID);

    void updateMatrixBuffer();

    void draw(ShaderProgram& shaderProgram);

    std::vector<Texture> mTextures {};
private:
    Model();

    void free();
    void stealResources(Model& other);
    void copyResources(const Model& other);
    void allocateBuffers();

    std::vector<Mesh> mMeshes {};

    GLuint mMatrixBuffer;
    std::map<GLuint, glm::mat4> mInstanceModelMatrixMap {};
    std::queue<GLuint> mDeletedInstanceIDs {};
    GLuint mNextInstanceID {0};
    GLuint mInstanceCapacity {128};
    bool mDirty {true};
};

#endif
