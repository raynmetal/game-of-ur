#ifndef FOOLSENGINE_MESH_H
#define FOOLSENGINE_MESH_H

#include <vector>
#include <map>
#include <queue>
#include <memory>

#include <GL/glew.h>

#include <assimp/scene.h>

#include "core/resource_database.hpp"
#include "vertex.hpp"

namespace ToyMakersEngine {
    /**
     *  A class whose current main purpose is to store geometry related info, and to
     * upload it to GPU memory when requested
     */
    class StaticMesh: public Resource<StaticMesh> {
    public:
        StaticMesh(const std::vector<BuiltinVertexData>& mVertices, const std::vector<GLuint>& mElements, GLuint vertexBuffer=0, GLuint elementBuffer=0, bool isUploaded=false);

        StaticMesh(const StaticMesh& other);
        StaticMesh& operator=(const StaticMesh& other);

        StaticMesh(StaticMesh&& other);
        StaticMesh& operator=(StaticMesh&& other);

        ~StaticMesh();

        GLuint getElementCount() {
            return mElements.size();
        }

        void bind(const VertexLayout& shaderVertexLayout);
        void unbind();

        std::vector<BuiltinVertexData>::iterator getVertexListBegin();
        std::vector<BuiltinVertexData>::iterator getVertexListEnd();
        std::vector<BuiltinVertexData>::const_iterator getVertexListBegin() const;
        std::vector<BuiltinVertexData>::const_iterator getVertexListEnd() const;

        VertexLayout getVertexLayout() const;
        inline static std::string getResourceTypeName() { return "StaticMesh"; }


    private:

        void setAttributePointers(const VertexLayout& shaderVertexLayout, std::size_t startingOffset=0);

        std::vector<BuiltinVertexData> mVertices {};
        std::vector<GLuint> mElements {};

        VertexLayout mVertexLayout;
        bool mIsUploaded { false };

        GLuint mVertexBuffer { 0 };
        GLuint mElementBuffer { 0 };

        void upload();
        void unload();
        void destroyResource();
        void releaseResource();
    };

    class StaticMeshFromDescription: public ResourceConstructor<StaticMesh, StaticMeshFromDescription> {
    public:
        StaticMeshFromDescription():
        ResourceConstructor<StaticMesh, StaticMeshFromDescription>{0}
        {}

        inline static std::string getResourceConstructorName(){ return "fromDescription"; }

    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

}

#endif
