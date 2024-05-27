#include <vector>
#include <map>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "texture_manager.hpp"
#include "vertex.hpp"

#include "mesh.hpp"

void Mesh::draw(ShaderProgramHandle shaderProgramHandle, GLuint instanceCount) {
    shaderProgramHandle.getResource().use();
    mMaterial.bind(shaderProgramHandle);
    glBindVertexArray(mShaderVAOMap[shaderProgramHandle]);
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mElements.size(),
            GL_UNSIGNED_INT,
            NULL,
            instanceCount
        );
    glBindVertexArray(0);
}

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<GLuint>& elements,
    const std::vector<TextureHandle>& textureHandles
) :
    mVertices{vertices},
    mElements{elements},
    mMaterial{textureHandles, 18.f},
    mVertexBuffer{0},
    mElementBuffer{0}
{
    allocateBuffers();
}

Mesh::~Mesh() {
    free();
}

Mesh::Mesh(Mesh&& other):
    mVertices {other.mVertices},
    mElements {other.mElements},
    mMaterial {other.mMaterial},
    mShaderVAOMap {other.mShaderVAOMap},
    mVertexBuffer {other.mVertexBuffer},
    mElementBuffer {other.mElementBuffer},
    mDirty {other.mDirty}
{
    // prevent other from removing our resources when its deconstructor is called
    other.releaseResources();
}

Mesh::Mesh(const Mesh& other):
    mVertices{other.mVertices},
    mElements{other.mElements},
    mMaterial {other.mMaterial},
    mVertexBuffer{0},
    mElementBuffer{0}
{
    allocateBuffers();
}

Mesh& Mesh::operator=(Mesh&& other) {
    if(&other == this) 
        return *this;

    free();

    mVertices = other.mVertices;
    mElements = other.mElements;
    mMaterial = other.mMaterial;
    mShaderVAOMap = other.mShaderVAOMap;
    mDirty = other.mDirty;
    mVertexBuffer = other.mVertexBuffer;
    mElementBuffer = other.mElementBuffer;

    other.releaseResources();

    return *this;
}

Mesh& Mesh::operator=(const Mesh& other) {
    if(&other == this) 
        return *this;
    
    free();

    mVertices = other.mVertices;
    mElements = other.mElements;
    mMaterial = other.mMaterial;
    mVertexBuffer = 0;
    mElementBuffer = 0;
    allocateBuffers();

    return *this;
}

void Mesh::associateShaderProgram(ShaderProgramHandle shaderProgramHandle){
    //Ensure duplicate calls to this function don't create duplicate VAOs
    if(getShaderVAO(shaderProgramHandle)) return;
    GLuint shaderVAO;
    glGenVertexArrays(1, &shaderVAO);
    glUseProgram(shaderProgramHandle.getResource().getProgramID());
    glBindVertexArray(shaderVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);

        //Enable and set vertex pointers
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        GLint attrPosition { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrPosition") };
        GLint attrNormal { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrNormal") };
        GLint attrTangent { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrTangent") };
        GLint attrColor { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrColor") };
        GLint attrTextureCoordinates { glGetAttribLocation(shaderProgramHandle.getResource().getProgramID(), "attrTextureCoordinates") };
        if(attrPosition >= 0) {
            glEnableVertexAttribArray(attrPosition);
            glVertexAttribPointer(attrPosition, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mPosition)));
        }
        if(attrNormal >= 0) {
            glEnableVertexAttribArray(attrNormal);
            glVertexAttribPointer(attrNormal, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mNormal)));
        }
        if(attrTangent >= 0) {
            glEnableVertexAttribArray(attrTangent);
            glVertexAttribPointer(attrTangent, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mTangent)));
        }
        if(attrColor >= 0) {
            glEnableVertexAttribArray(attrColor);
            glVertexAttribPointer(attrColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mColor)));
        }
        if(attrTextureCoordinates >= 0) {
            glEnableVertexAttribArray(attrTextureCoordinates);
            glVertexAttribPointer(attrTextureCoordinates, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, mTextureCoordinates)));
        }
    glBindVertexArray(0);

    mShaderVAOMap[shaderProgramHandle] = shaderVAO;
}

void Mesh::disassociateShaderProgram(ShaderProgramHandle shaderProgramHandle) {
    if(!getShaderVAO(shaderProgramHandle)) return;
    glDeleteVertexArrays(1, &mShaderVAOMap[shaderProgramHandle]);
    mShaderVAOMap.erase(shaderProgramHandle);
}

GLuint Mesh::getShaderVAO(const ShaderProgramHandle& shaderProgramHandle) const {
    auto resultIterator { mShaderVAOMap.find(shaderProgramHandle) };
    if(resultIterator == mShaderVAOMap.end()) return 0;
    return resultIterator->second;
}

void Mesh::free() {
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mElementBuffer);

    std::vector<GLuint> shaderVAOs(mShaderVAOMap.size());
    std::size_t count{0};
    for(std::pair shaderVAO : mShaderVAOMap) {
        shaderVAOs[count ++] = shaderVAO.second;
    }
    glDeleteVertexArrays(count, shaderVAOs.data());

    mShaderVAOMap.clear();
}

void Mesh::releaseResources() {
    mVertexBuffer = 0;
    mElementBuffer = 0;
    mShaderVAOMap.clear();
}

void Mesh::allocateBuffers() {
    if(!mElementBuffer) {
        glGenBuffers(1, &mElementBuffer);
    }
    if(!mVertexBuffer) {
        glGenBuffers(1, &mVertexBuffer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(Vertex) * mVertices.size(),
            mVertices.data(), GL_STATIC_DRAW
        );
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mElements.size(),
            mElements.data(), GL_STATIC_DRAW
        );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
