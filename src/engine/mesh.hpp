#ifndef ZOMESH_H
#define ZOMESH_H

#include <vector>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "vertex.hpp"
#include "texture_manager.hpp"
#include "shader_program_manager.hpp"

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
        const std::vector<TextureHandle>& textureHandles
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
    void associateShaderProgram(ShaderProgramHandle shaderProgramHandle);
    /* Removes VAO for a given shader program */
    void disassociateShaderProgram(ShaderProgramHandle shaderProgramHandle);
    /* Returns the ID of the VAO associated with this shader program */
    GLuint getShaderVAO(const ShaderProgramHandle& shaderProgramHandle) const;

    /* 
    Uses the shader program to render this mesh, setting uniform attributes. Assumes 
    instance related attributes are already set, requiring only the number of them to render
    */
    void draw(ShaderProgramHandle shaderProgramHandle, GLuint instanceCount);

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
    /*
    Sends vertex and element data to the GPU
    */
    void allocateBuffers();

    void bindMaterial(ShaderProgramHandle shaderProgramHandle);

    std::vector<Vertex> mVertices {};
    std::vector<GLuint> mElements {};
    std::vector<TextureHandle> mTextureHandles {};
    std::map<ShaderProgramHandle, GLuint> mShaderVAOMap {};

    GLuint mVertexBuffer;
    GLuint mElementBuffer;

    GLfloat mSpecularExponent {64.f};

    bool mDirty {true};
};

#endif
