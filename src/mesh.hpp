#ifndef ZOMESH_H
#define ZOMESH_H

#include <vector>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "texture.hpp"
#include "vertex.hpp"
#include "shader_program.hpp"

constexpr GLuint kInitialInstanceCapacity {128};

/* 
A class whose main purpose is to store geometry-related information. At present, it also holds data
representing its material (primarily textures)
*/
class Mesh {
public:
    /*
    Initializer for the mesh class
    */
    Mesh(
        const std::vector<Vertex>& vertices, 
        const std::vector<GLuint>& elements,
        const std::vector<Texture*>& textures
    );

    /*
    Mesh class destructor
    */
    ~Mesh();

    /* Sets up a VAO for a given shader program, setting vertex pointers as necessary */
    void associateShaderProgram(const ShaderProgram& shaderProgram);
    /* Removes VAO for a given shader program */
    void disassociateShaderProgram(const ShaderProgram& shaderProgram);

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

    /* Uses the shader program to render this mesh, setting uniform attributes
    as required */
    void Draw(ShaderProgram& shaderProgram);

private:
    void updateMatrixBuffer();

    std::vector<Vertex> mVertices;
    std::vector<GLuint> mElements;
    std::vector<Texture*> mpTextures;

    GLuint mVertexBuffer;
    GLuint mMatrixBuffer;
    GLuint mElementBuffer;
    GLuint mNextInstanceID {0};
    GLuint mInstanceCapacity { kInitialInstanceCapacity };

    std::map<GLuint, GLuint> mShaderVAOMap {};
    std::map<GLuint, glm::mat4> mInstanceModelMatrixMap{};
    std::queue<GLuint> mDeletedInstanceIDs {};

    bool mDirty {true};
};

#endif
