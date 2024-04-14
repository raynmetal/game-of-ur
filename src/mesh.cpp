#include <vector>
#include <map>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "texture.hpp"
#include "vertex.hpp"

#include "mesh.hpp"

void Mesh::draw(ShaderProgram& shaderProgram, GLuint instanceCount) {
    shaderProgram.use();
    bindMaterial(shaderProgram);
    glBindVertexArray(mShaderVAOMap[shaderProgram.getProgramID()]);
        glDrawElementsInstanced(
            GL_TRIANGLES,
            mElements.size(),
            GL_UNSIGNED_INT,
            0,
            instanceCount
        );
    glBindVertexArray(0);
}

void Mesh::bindMaterial(ShaderProgram& shaderProgram) {
    //Set texture-related uniforms
    bool usingAlbedoMap {false};
    bool usingNormalMap {false};
    bool usingSpecularMap {false};

    GLuint textureUnit {0};
    for(Texture* pTexture : mpTextures) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, pTexture->getTextureID());
        //TODO: allow multiple materials to make up a single mesh
        switch(pTexture->getUsage()) {
            case Texture::Albedo:
                usingAlbedoMap = true;
                shaderProgram.setUInt("uMaterial.mTextureAlbedo", textureUnit);
            break;
            case Texture::Normal:
                usingNormalMap = true;
                shaderProgram.setUInt("uMaterial.mTextureNormal", textureUnit);
            break;
            case Texture::Specular:
                usingSpecularMap = true;
                shaderProgram.setUInt("uMaterial.mTextureSpecular", textureUnit);
            break;
            default: break;
        }
        ++textureUnit;
    }
    glActiveTexture(GL_TEXTURE0);
    shaderProgram.setUBool("uMaterial.mUsingAlbedoMap", usingAlbedoMap);
    shaderProgram.setUBool("uMaterial.mUsingNormalMap", usingNormalMap);
    shaderProgram.setUBool("uMaterial.mUsingSpecularMap", usingSpecularMap);

    shaderProgram.setUFloat("uMaterial.mSpecularExponent", mSpecularExponent);
}

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<GLuint>& elements,
    const std::vector<Texture*>& pTextures
) : 
    mVertices{vertices},
    mElements{elements},
    mpTextures{pTextures},
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
    mpTextures {other.mpTextures},
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
    mpTextures{other.mpTextures},
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
    mpTextures = other.mpTextures;
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
    mpTextures = other.mpTextures;
    mVertexBuffer = 0;
    mElementBuffer = 0;
    allocateBuffers();

    return *this;
}

void Mesh::associateShaderProgram(GLuint programID){
    //Ensure duplicate calls to this function don't create duplicate VAOs
    if(getShaderVAO(programID)) return;
    GLuint shaderVAO;
    glGenVertexArrays(1, &shaderVAO);
    glUseProgram(programID);
    glBindVertexArray(shaderVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);

        //Enable and set vertex pointers
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        GLint attrPosition { glGetAttribLocation(programID, "attrPosition") };
        GLint attrNormal { glGetAttribLocation(programID, "attrNormal") };
        GLint attrTangent { glGetAttribLocation(programID, "attrTangent") };
        GLint attrColor { glGetAttribLocation(programID, "attrColor") };
        GLint attrTextureCoordinates { glGetAttribLocation(programID, "attrTextureCoordinates") };
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

    GLuint error { glGetError() };
    std::cout << "Error in mesh association: " << glewGetErrorString(error) << " (error #" << error << ")\n";

    mShaderVAOMap[programID] = shaderVAO;
}

void Mesh::disassociateShaderProgram(GLuint programID) {
    if(!getShaderVAO(programID)) return;
    glDeleteVertexArrays(1, &mShaderVAOMap[programID]);
    mShaderVAOMap.erase(programID);
}

GLuint Mesh::getShaderVAO(const GLuint programID) const {
    auto resultIterator { mShaderVAOMap.find(programID) };
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
