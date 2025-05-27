#ifndef FOOLSENGINE_SCENELOADING_H
#define FOOLSENGINE_SCENELOADING_H

#include "core/resource_database.hpp"

#include "sim_system.hpp"
#include "scene_system.hpp"

namespace ToyMakersEngine {
    class SceneFromDescription;
    class SceneNodeFromDescription;
    class SimObjectFromDescription;
    class ViewportNodeFromDescription;

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
        void loadResources(const nlohmann::json& resourceList);
        std::shared_ptr<SimObject> loadSceneNodes(const nlohmann::json& nodeList);
        void loadConnections(const nlohmann::json& connectionList, std::shared_ptr<SceneNodeCore> localRoot);

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

    class ViewportNodeFromDescription: public ResourceConstructor<ViewportNode, ViewportNodeFromDescription> {
    public:
        ViewportNodeFromDescription():
        ResourceConstructor<ViewportNode, ViewportNodeFromDescription> {0}
        {}

        static std::string getResourceConstructorName() { return "fromDescription"; }

    private:
        std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters);
    };

}

#endif
