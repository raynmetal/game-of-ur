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

    /* move constructor */
    Mesh(Mesh&& other);
    /* copy constructor */
    Mesh(const Mesh& other);

    /* move assignment */
    Mesh& operator=(Mesh&& other);
    /* copy assignment */
    Mesh& operator=(const Mesh& other);

    /* Sets up a VAO for a given shader program, setting vertex pointers as necessary */
    void associateShaderProgram(const ShaderProgram& shaderProgram, GLuint matrixBuffer);
    void associateShaderProgram(GLuint programID, GLuint matrixBuffer);
    /* Removes VAO for a given shader program */
    void disassociateShaderProgram(const ShaderProgram& shaderProgram);


    /* 
    Uses the shader program to render this mesh, setting uniform attributes. Assumes 
    instance related attributes are already set, requiring only the number of them to render
    */
    void draw(ShaderProgram& shaderProgram, GLuint instanceCount);

private:
    /* 
    Destroys resources used by this object
    */
    void free();
    /*
    Removes references to allocated resources without destroying the
    resources themselves
    */
    void releaseResources();
    void allocateBuffers();

    std::vector<Vertex> mVertices;
    std::vector<GLuint> mElements;
    std::vector<Texture*> mpTextures;

    GLuint mVertexBuffer;
    GLuint mElementBuffer;

    std::map<GLuint, GLuint> mShaderVAOMap {};

    bool mDirty {true};
};

#endif