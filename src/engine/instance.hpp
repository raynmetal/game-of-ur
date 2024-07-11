#ifndef ZOINSTANCE
#define ZOINSTANCE

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <GL/glew.h>

enum DefaultInstanceAttributeLocations {
    LOCATION_MATRIXMODEL=7,
    LOCATION_LIGHTPOSITION=11,
    LOCATION_LIGHTDIRECTION=12,
    LOCATION_DIFFUSECOLOR=13,
    LOCATION_SPECULARCOLOR=14,
    LOCATION_AMBIENTCOLOR=15,
    LOCATION_LIGHTTYPE=16,
    LOCATION_LIGHTDECAYLINEAR=17,
    LOCATION_LIGHTDECAYQUADRATIC=18,
    LOCATION_LIGHTCOSCUTOFFINNER=19,
    LOCATION_LIGHTCOSCUTOFFOUTER=20
};

struct InstanceAttributeDescriptor {
    InstanceAttributeDescriptor(const std::string& name, const GLint layoutLocation, GLuint nComponents, GLenum type=GL_FLOAT) 
    : mName {name}, mLayoutLocation { layoutLocation }, 
        mNComponents {nComponents}, mType {type}, 
        mSize{GetGLTypeSize(type) * nComponents}
    {}

    const std::string mName;
    const GLint mLayoutLocation;
    const GLuint mNComponents;
    const GLenum mType;
    const std::size_t mSize;

    // TODO: code duplicated from VertexAttributeDescriptor. Refactor needed
    bool operator==(const InstanceAttributeDescriptor& other) const {
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

struct InstanceLayout {
public:
    InstanceLayout(const std::vector<InstanceAttributeDescriptor>& attributeList) : mAttributeList{attributeList} {}

    std::vector<InstanceAttributeDescriptor> getAttributeList() const {
        return mAttributeList;
    }

    std::size_t computeStride() const {
        std::size_t stride {0};
        for(const auto& attribute: mAttributeList) {
            stride += attribute.mSize;
        }
        return stride;
    }

    std::size_t computeRelativeOffset(std::size_t attributeIndex) const {
        assert(attributeIndex < mAttributeList.size());
        std::size_t baseOffset {0};
        for(std::size_t i{0} ; i < attributeIndex; ++i) {
            baseOffset += mAttributeList[i].mSize;
        }
        return baseOffset;
    }

    bool isSubsetOf(const InstanceLayout& other) const {
        if(mAttributeList.size() > other.mAttributeList.size()) return false;

        std::size_t myAttributeIndex {0};
        for(std::size_t i{0}; i < other.mAttributeList.size() && myAttributeIndex < mAttributeList.size(); ++i) {
            if(other.mAttributeList[i] == mAttributeList[myAttributeIndex]) {
                ++ myAttributeIndex;
                if(myAttributeIndex == mAttributeList.size()) return true;
            }
        }

        return false;
    }

private:
    std::vector<InstanceAttributeDescriptor> mAttributeList;
};

class BaseInstanceAllocator {
public:
    BaseInstanceAllocator(const InstanceLayout& instanceLayout)
    : mInstanceLayout(instanceLayout) 
    {}

    BaseInstanceAllocator(BaseInstanceAllocator&& other) = delete;
    BaseInstanceAllocator(const BaseInstanceAllocator& other) = delete;

    BaseInstanceAllocator& operator=(BaseInstanceAllocator&& other) = delete;
    BaseInstanceAllocator& operator=(const BaseInstanceAllocator& other) = delete;

    virtual ~BaseInstanceAllocator();

    InstanceLayout getInstanceLayout() const;

    void bind(const InstanceLayout& shaderInstanceLayout);

    void unbind();

    bool isUploaded() { return mUploaded; }

protected:
    virtual void upload() = 0;

    GLuint mVertexBufferIndex {0};

private:

    void unload();
    void _upload();

    // Sets attribute pointers per the data contained in 
    // the Instance layout.
    void setAttributePointers(const InstanceLayout& shaderInstanceLayout, std::size_t startingOffset = 0);

    InstanceLayout mInstanceLayout;
    bool mUploaded {false};
};

static InstanceLayout BuiltinModelMatrixLayout {
    {
        {"modelMatrixCol0", LOCATION_MATRIXMODEL, 4, GL_FLOAT},
        {"modelMatrixCol1", LOCATION_MATRIXMODEL+1, 4, GL_FLOAT},
        {"modelMatrixCol2", LOCATION_MATRIXMODEL+2, 4, GL_FLOAT},
        {"modelMatrixCol3", LOCATION_MATRIXMODEL+3, 4, GL_FLOAT},
    }
};

class BuiltinModelMatrixAllocator : public BaseInstanceAllocator {
public:
    BuiltinModelMatrixAllocator(std::vector<glm::mat4> modelMatrices)
    : 
        BaseInstanceAllocator{ BuiltinModelMatrixLayout },
        mModelMatrices{modelMatrices}
    {}

protected:
    virtual void upload() override;

private:
    std::vector<glm::mat4> mModelMatrices;
};

#endif
