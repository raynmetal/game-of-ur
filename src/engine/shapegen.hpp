#ifndef ZOSHAPEGEN_H
#define ZOSHAPEGEN_H

#include <memory>
#include <nlohmann/json.hpp>

#include "resource_database.hpp"
#include "mesh.hpp"

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

#endif
