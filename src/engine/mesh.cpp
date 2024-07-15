#include <vector>
#include <map>
#include <iostream>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "texture_manager.hpp"
#include "vertex.hpp"

#include "mesh.hpp"

BaseMesh::BaseMesh(
    const VertexLayout& vertexLayout
) :
    mVertexLayout{ vertexLayout }
{}

BaseMesh::~BaseMesh() {
    destroyResource();
}

BaseMesh::BaseMesh(BaseMesh&& other):
    mVertexBufferIndex { other.mVertexBufferIndex },
    mElementBufferIndex { other.mElementBufferIndex },
    mUploaded { other.mUploaded },
    mVertexLayout { other.mVertexLayout }
{
    // Prevent other from removing our resources when its destructor is called
    other.releaseResource();
}

BaseMesh::BaseMesh(const BaseMesh& other):
    mVertexLayout { other.mVertexLayout }
{
    mVertexLayout = other.mVertexLayout;
}

BaseMesh& BaseMesh::operator=(BaseMesh&& other) {
    if(&other == this) 
        return *this;

    destroyResource();

    mVertexBufferIndex = other.mVertexBufferIndex;
    mElementBufferIndex = other.mElementBufferIndex;
    mUploaded = other.mUploaded;

    other.releaseResource();

    return *this;
}

BaseMesh& BaseMesh::operator=(const BaseMesh& other) {
    if(&other == this) 
        return *this;
    
    destroyResource();
    mVertexLayout = other.mVertexLayout;

    return *this;
}

void BaseMesh::bind(const VertexLayout& shaderVertexLayout) {
    if(!mUploaded) {
        _upload();
    }
    assert(mUploaded);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferIndex);
        setAttributePointers(shaderVertexLayout);
        GLenum error = glGetError();
        if(error!= GL_FALSE) {
            glewGetErrorString(error);
            std::cout << "Error occurred during mesh attribute setting: "  << error
                << ":" << glewGetErrorString(error) << std::endl;
            throw error;
        }
}

void BaseMesh::setAttributePointers(const VertexLayout& shaderVertexLayout, std::size_t startingOffset) {
    assert(mUploaded);
    assert(shaderVertexLayout.isSubsetOf(mVertexLayout));

    const std::size_t stride { mVertexLayout.computeStride() };
    const std::vector<VertexAttributeDescriptor>& attributeDescList { mVertexLayout.getAttributeList() };
    const std::vector<VertexAttributeDescriptor>& shaderAttributeDescList { shaderVertexLayout.getAttributeList() };

    std::size_t currentOffset {0};
    std::size_t shaderVertexAttributeIndex {0};
    for(std::size_t i {0}; i < attributeDescList.size() && shaderVertexAttributeIndex < shaderAttributeDescList.size(); ++i) {
        const VertexAttributeDescriptor& attributeDesc = attributeDescList[i];
        const VertexAttributeDescriptor& shaderAttributeDesc = shaderAttributeDescList[shaderVertexAttributeIndex];

        if(attributeDesc == shaderAttributeDesc){
            switch(attributeDesc.mType){
                case GL_INT:
                case GL_UNSIGNED_INT:
                    glVertexAttribIPointer(
                        attributeDesc.mLayoutLocation,
                        attributeDesc.mNComponents,
                        attributeDesc.mType,
                        stride,
                        reinterpret_cast<void*>(startingOffset + currentOffset)
                    );
                break;
                case GL_FLOAT:
                default:
                    glVertexAttribPointer(
                        attributeDesc.mLayoutLocation,
                        attributeDesc.mNComponents,
                        attributeDesc.mType,
                        GL_FALSE,
                        stride,
                        reinterpret_cast<void*>(startingOffset + currentOffset)
                    );
                break;
            }
            glEnableVertexAttribArray(attributeDesc.mLayoutLocation);
            GLenum error { glGetError() };
            if(error!= GL_FALSE) {
                std::cout << "Error occurred during mesh attribute enabling: "  << error
                    << ":" << glewGetErrorString(error) << std::endl;
                throw error;
            }
            ++shaderVertexAttributeIndex;
        }

        currentOffset += attributeDesc.mSize;
    }

    assert(shaderVertexAttributeIndex == shaderAttributeDescList.size());
}

void BaseMesh::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BaseMesh::_upload() {
    if(mUploaded) return;

    upload();
    assert(mVertexBufferIndex);
    assert(mElementBufferIndex);
    mUploaded = true;
}

void BaseMesh::unload() {
    if(!mUploaded) return;

    glDeleteBuffers(1, &mVertexBufferIndex);
    mVertexBufferIndex = 0;
    glDeleteBuffers(1, &mElementBufferIndex);
    mElementBufferIndex = 0;

    mUploaded = false;
}

void BaseMesh::destroyResource() {
    unload();
}

void BaseMesh::releaseResource() {
    if(!mUploaded) return;

    mVertexBufferIndex = 0;
    mElementBufferIndex = 0;

    mUploaded = false;
}

VertexLayout BaseMesh::getVertexLayout() const {
    return mVertexLayout;
}

BuiltinMesh::BuiltinMesh(aiMesh* pAiMesh) 
: BaseMesh{BuiltinVertexLayout} 
{
    for (std::size_t i{0}; i < pAiMesh->mNumVertices; ++i) {
        // TODO: make fetching vertex data more robust. There is no
        // guarantee that a mesh will contain vertex colours or
        // texture sampling coordinates
        mVertices.push_back({
            { // position
                pAiMesh->mVertices[i].x,
                pAiMesh->mVertices[i].y,
                pAiMesh->mVertices[i].z,
                1.f
            },
            { // normal
                pAiMesh->mNormals[i].x,
                pAiMesh->mNormals[i].y,
                pAiMesh->mNormals[i].z,
                0.f
            },
            { // tangent
                pAiMesh->mTangents[i].x,
                pAiMesh->mTangents[i].y,
                pAiMesh->mTangents[i].z,
                0.f
            },
            { 1.f, 1.f, 1.f, 1.f },
            {
                pAiMesh->mTextureCoords[0][i].x,
                pAiMesh->mTextureCoords[0][i].y
            }
        });
    }

    for(std::size_t i{0}; i < pAiMesh->mNumFaces; ++i) {
        aiFace face = pAiMesh->mFaces[i];
        for(std::size_t elementIndex{0}; elementIndex < face.mNumIndices; ++elementIndex) {
            mElements.push_back(face.mIndices[elementIndex]);
        }
    }
}

BuiltinMesh::BuiltinMesh(
    const std::vector<BuiltinVertexData>& vertices,
    const std::vector<GLuint>& elements
) : BaseMesh{BuiltinVertexLayout}, mVertices {vertices}, mElements {elements}
{}


void BuiltinMesh::upload() {
    // Nop if already uploaded
    if(isUploaded()) return;

    //   TODO: somehow have the base class take care of more of the memory 
    // management? Regardless of the type of vertex data, so long as we have
    // a pointer to the underlying array, there's no reason we shouldn't
    // be able to upload data in the base class rather than leaving it to
    // the subclass
    //   Not sure how we can achieve this.

    // Generate names for our vertex, element, and array buffers
    glGenBuffers(1, &mVertexBufferIndex);
    glGenBuffers(1, &mElementBufferIndex);

    // Set up the buffer objects for this mesh
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(BuiltinVertexData), mVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mElements.size() * sizeof(GLuint), mElements.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
