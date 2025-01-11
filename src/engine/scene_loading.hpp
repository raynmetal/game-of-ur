#ifndef ZOSCENELOADING_H
#define ZOSCENELOADING_H

#include "resource_database.hpp"

#include "sim_system.hpp"
#include "scene_system.hpp"

class SceneFromDescription;
class SceneNodeFromDescription;

class SceneFromFile: public ResourceConstructor<SimObject, SceneFromFile> {
public:
    SceneFromFile():
    ResourceConstructor<SimObject, SceneFromFile> {0}
    {}

    static std::string getResourceConstructorName() { return "fromSceneFile"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParams);
};

class SceneFromDescription: public ResourceConstructor<SimObject, SceneFromDescription> {
public:
    SceneFromDescription(): 
    ResourceConstructor<SimObject, SceneFromDescription> {0}
    {}

    static std::string getResourceConstructorName() { return "fromSceneDescription"; }

private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParams) override;
};

class SceneNodeFromDescription: public ResourceConstructor<SceneNode, SceneNodeFromDescription> {
public:
    SceneNodeFromDescription(): 
    ResourceConstructor<SceneNode, SceneNodeFromDescription>{0}
    {}

    static std::string getResourceConstructorName() { return "fromNodeDescription"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParams) override;
};

class SimObjectFromDescription: public ResourceConstructor<SimObject, SimObjectFromDescription> {
public:
    SimObjectFromDescription():
    ResourceConstructor<SimObject, SimObjectFromDescription> {0}
    {}

    static std::string getResourceConstructorName() { return "fromDescription"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters);
};

#endif
