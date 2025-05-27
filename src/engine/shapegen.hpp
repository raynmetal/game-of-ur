#ifndef FOOLSENGINE_SHAPEGEN_H
#define FOOLSENGINE_SHAPEGEN_H

#include <memory>
#include <nlohmann/json.hpp>

#include "core/resource_database.hpp"
#include "mesh.hpp"
#include "model.hpp"

namespace ToyMakersEngine {

    class StaticMeshSphereLatLong: public ResourceConstructor<StaticMesh, StaticMeshSphereLatLong> {
    public:
        StaticMeshSphereLatLong():
        ResourceConstructor<StaticMesh, StaticMeshSphereLatLong>{0}
        {}
        inline static std::string getResourceConstructorName() { return "sphereLatLong"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

    class StaticMeshRectangleDimensions: public ResourceConstructor<StaticMesh, StaticMeshRectangleDimensions> {
    public:
        StaticMeshRectangleDimensions():
        ResourceConstructor<StaticMesh, StaticMeshRectangleDimensions>{0}
        {}
        inline static std::string getResourceConstructorName() { return "rectangleDimensions"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

    class StaticMeshCuboidDimensions: public ResourceConstructor<StaticMesh, StaticMeshCuboidDimensions> {
    public:
        StaticMeshCuboidDimensions():
        ResourceConstructor<StaticMesh, StaticMeshCuboidDimensions>{0}
        {}
        inline static std::string getResourceConstructorName() { return "cuboidDimensions"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

    class StaticModelSphereLatLong: public ResourceConstructor<StaticModel, StaticModelSphereLatLong> {
    public:
        StaticModelSphereLatLong():
        ResourceConstructor<StaticModel, StaticModelSphereLatLong>{0}
        {}
        inline static std::string getResourceConstructorName() { return "sphereLatLong"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

    class StaticModelRectangleDimensions: public ResourceConstructor<StaticModel, StaticModelRectangleDimensions> {
    public:
        StaticModelRectangleDimensions():
        ResourceConstructor<StaticModel, StaticModelRectangleDimensions>{0}
        {}
        inline static std::string getResourceConstructorName() { return "rectangleDimensions"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

    class StaticModelCuboidDimensions: public ResourceConstructor<StaticModel, StaticModelCuboidDimensions> {
    public:
        StaticModelCuboidDimensions():
        ResourceConstructor<StaticModel, StaticModelCuboidDimensions>{0}
        {}
        inline static std::string getResourceConstructorName() { return "cuboidDimensions"; }
    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
    };

}
#endif
