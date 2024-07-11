#ifndef ZOVERTEX_H
#define ZOVERTEX_H

#include <string>
#include <glm/glm.hpp>

enum DefaultAttributeLocation {
    LOCATION_POSITION=0,
    LOCATION_NORMAL=1,
    LOCATION_TANGENT=2,
    LOCATION_COLOR=3,
    LOCATION_UV1=4,
    LOCATION_UV2=5,
    LOCATION_UV3=6
};

struct VertexAttributeDescriptor {
    VertexAttributeDescriptor(const std::string& name, GLint layoutLocation, GLuint nComponents=1, GLenum type=GL_FLOAT)
    : 
        mName{name}, mLayoutLocation{layoutLocation}, mNComponents{nComponents}, 
        mType{type}, mSize { GetGLTypeSize(type) * nComponents}
    {
        assert(mNComponents >= 1 && mNComponents <= 4);
        assert(mType == GL_FLOAT || mType == GL_UNSIGNED_INT || mType == GL_INT);
    }

    std::string mName;
    GLint mLayoutLocation;
    GLuint mNComponents;
    GLenum mType;
    std::size_t mSize;

    bool operator==(const VertexAttributeDescriptor& other) const {
        if(
            other.mName == mName 
            && other.mLayoutLocation == mLayoutLocation 
            && other.mNComponents == mNComponents
            && other.mType == mType
            && other.mSize == mSize
        ) return true;
        return false;
    }

private:
    static std::size_t GetGLTypeSize(GLenum type){
        std::size_t componentSize;
        switch(type) {
            case GL_FLOAT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            default:
                componentSize = sizeof(GLfloat);
        }
        return componentSize;
    }
};

struct VertexLayout {
    VertexLayout(std::vector<VertexAttributeDescriptor> attributeList) : mAttributeList{attributeList} {}

    std::vector<VertexAttributeDescriptor> getAttributeList() const {
        return mAttributeList;
    }
    std::size_t computeStride() const {
        std::size_t stride {0};
        for(const auto& attribute : mAttributeList) {
            stride += attribute.mSize;
        }
        return stride;
    }

    std::size_t computeRelativeOffset(std::size_t attributeIndex) const {
        assert(attributeIndex < mAttributeList.size() && attributeIndex >= 0);
        std::size_t baseOffset {0};
        for(std::size_t i {0}; i < attributeIndex; ++i) {
            baseOffset += mAttributeList[i].mSize;
        }
        return baseOffset;
    }

    bool isSubsetOf(const VertexLayout& other) const {
        if(mAttributeList.size() > other.mAttributeList.size()) return false;

        std::size_t myAttributeIndex {0};
        for(std::size_t i{0}; i < other.mAttributeList.size() && myAttributeIndex < mAttributeList.size(); ++i) {
            if(other.mAttributeList[i] == mAttributeList[myAttributeIndex]){
                ++myAttributeIndex;
                if(myAttributeIndex == mAttributeList.size()) return true;
            }
        }

        return false;
    }

private:
    std::vector<VertexAttributeDescriptor> mAttributeList {};
};

struct BuiltinVertexData {
    glm::vec4 mPosition;
    glm::vec4 mNormal;
    glm::vec4 mTangent;
    glm::vec4 mColor {1.f}; // by default, white
    glm::vec2 mUV1;
    glm::vec2 mUV2;
    glm::vec2 mUV3;
};


static VertexLayout BuiltinVertexLayout {
    {
        {"position", LOCATION_POSITION, 4, GL_FLOAT},
        {"normal", LOCATION_NORMAL, 4, GL_FLOAT},
        {"tangent", LOCATION_TANGENT, 4, GL_FLOAT},
        {"color", LOCATION_COLOR, 4, GL_FLOAT},
        {"UV1", LOCATION_UV1, 2, GL_FLOAT},
        {"UV2", LOCATION_UV2, 2, GL_FLOAT},
        {"UV3", LOCATION_UV3, 2, GL_FLOAT}
    }
};

#endif
