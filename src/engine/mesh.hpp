#ifndef ZOMESH_H
#define ZOMESH_H

#include <vector>
#include <map>
#include <queue>

#include <GL/glew.h>

#include "resource_manager.hpp"
#include <assimp/scene.h>

#include "simple_ecs.hpp"
#include "vertex.hpp"

/* 
A class whose main purpose is to store geometry-related information.
*/
class BaseMesh: IResource {
public:
    /*
    Initializer for the mesh class
    */
    BaseMesh(const VertexLayout& vertexLayout);

    /*
    Mesh class destructor
    */
    virtual ~BaseMesh();

    /* move constructor */
    BaseMesh(BaseMesh&& other);
    /* copy constructor */
    BaseMesh(const BaseMesh& other);

    /* move assignment */
    BaseMesh& operator=(BaseMesh&& other);
    /* copy assignment */
    BaseMesh& operator=(const BaseMesh& other);

    VertexLayout getVertexLayout() const;

    virtual GLuint getElementCount() = 0;

    // Bind the VAO associated with this mesh. If it hasn't been 
    // created yet, calls upload, then binds the VAO
    void bind(const VertexLayout& shaderVertexLayout);

    // Unbind the VAO associated with this mesh. as simple as binding
    // vertex array 0
    void unbind();

    bool isUploaded() { return mUploaded; }

protected:
    // uploads vertex data to the GPU, and sets up a corresponding VBO,
    // VAO, and EBO
    virtual void upload() = 0;

    GLuint mVertexBufferIndex { 0 };
    GLuint mElementBufferIndex { 0 };

protected:

    bool mUploaded { false };

    // deletes vertex data corresponding to this mesh from the GPU. Should unset
    // mUploaded once done
    void unload();

    // Sets attribute pointers per the data contained in 
    // the vertex layout.
    void setAttributePointers(const VertexLayout& shaderVertexLayout, std::size_t startingOffset = 0);


    void _upload();

    VertexLayout mVertexLayout;

    /* 
    Destroys the GPU resources used by this object
    */
    virtual void destroyResource() override;

    /*
    Removes references to GPU allocated resources without destroying the
    resources themselves
    */
    virtual void releaseResource() override;
};

class BuiltinMesh : public BaseMesh {
public:
    BuiltinMesh(aiMesh* const pAiMesh);
    BuiltinMesh(
        const std::vector<BuiltinVertexData>& vertices={}, 
        const std::vector<GLuint>& elements={}
    );

    virtual GLuint getElementCount() {
        return mElements.size();
    }

protected:
    virtual void upload() override;

private:
    std::vector<BuiltinVertexData> mVertices {};
    std::vector<GLuint> mElements {};

    virtual void destroyResource() override {BaseMesh::destroyResource();}
    virtual void releaseResource() override {BaseMesh::releaseResource();}

friend class ResourceManager<BuiltinMesh>;
};

template<>
inline ResourceHandle<BuiltinMesh> Interpolator<ResourceHandle<BuiltinMesh>>::operator() (
    const ResourceHandle<BuiltinMesh>& previousState,
    const ResourceHandle<BuiltinMesh>& nextState,
    float simulationProgress
) const {
    return nextState;
}

#endif
