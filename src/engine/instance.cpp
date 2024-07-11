#include <iostream>
#include <GL/glew.h>
#include "instance.hpp"

BaseInstanceAllocator::~BaseInstanceAllocator() {
    if(!mUploaded) return;
    glDeleteBuffers(1, &mVertexBufferIndex);
};

InstanceLayout BaseInstanceAllocator::getInstanceLayout() const {
    return mInstanceLayout;
}

void BaseInstanceAllocator::_upload() {
    if(mUploaded) return;

    upload();
    assert(mVertexBufferIndex);
    mUploaded = true;
}

void BaseInstanceAllocator::unload() {
    if(!mUploaded) return;

    glDeleteBuffers(1, &mVertexBufferIndex);
    mVertexBufferIndex = 0;

    mUploaded = false;
}

void BaseInstanceAllocator::setAttributePointers(const InstanceLayout& shaderInstanceLayout, std::size_t startingOffset) {
    assert(mUploaded);
    assert(shaderInstanceLayout.isSubsetOf(mInstanceLayout));

    const std::size_t stride {mInstanceLayout.computeStride()};
    const std::vector<InstanceAttributeDescriptor>& attributeDescList { mInstanceLayout.getAttributeList() };
    const std::vector<InstanceAttributeDescriptor>& shaderAttributeDescList { shaderInstanceLayout.getAttributeList() };

    std::size_t currentOffset {0};
    std::size_t shaderInstanceAttributeIndex {0};
    for(std::size_t i{0}; i < attributeDescList.size(); ++i) {
        const InstanceAttributeDescriptor& attributeDesc = attributeDescList[i];
        const InstanceAttributeDescriptor& shaderAttributeDesc = shaderAttributeDescList[shaderInstanceAttributeIndex];

        if(attributeDesc == shaderAttributeDesc) {
            glVertexAttribPointer(
                attributeDesc.mLayoutLocation,
                attributeDesc.mNComponents,
                attributeDesc.mType,
                GL_FALSE,
                stride,
                reinterpret_cast<void*>(startingOffset + currentOffset)
            );
            glEnableVertexAttribArray(attributeDesc.mLayoutLocation);
            glVertexAttribDivisor(attributeDesc.mLayoutLocation, 1);
            ++shaderInstanceAttributeIndex;
        }

        currentOffset += attributeDesc.mSize;
    }

    assert(shaderInstanceAttributeIndex == shaderAttributeDescList.size());
}

void BaseInstanceAllocator::bind(const InstanceLayout& shaderInstanceLayout) {
    if(!mUploaded) {
        _upload();
    }
    assert(mUploaded);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferIndex);
        setAttributePointers(shaderInstanceLayout);
        GLenum error = glGetError();
        if(error!= GL_FALSE) {
            glewGetErrorString(error);
            std::cout << "Error occurred during mesh attribute setting: "  << error
                << ":" << glewGetErrorString(error) << std::endl;
            throw error;
        }
}

void BaseInstanceAllocator::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BuiltinModelMatrixAllocator::upload() {
    if(isUploaded()) return;

    glGenBuffers(1, &mVertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferIndex);
        glBufferData(GL_ARRAY_BUFFER, mModelMatrices.size() * sizeof(glm::mat4), mModelMatrices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
