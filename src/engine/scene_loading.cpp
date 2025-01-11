#include <fstream>
#include <filesystem>

#include "scene_loading.hpp"

std::shared_ptr<IResource> SceneFromFile::createResource(const nlohmann::json& sceneFileDescription) {
    std::string scenePath { sceneFileDescription.at("path").get<std::string>() };
    std::ifstream jsonFileStream;

    jsonFileStream.open(scenePath);
    nlohmann::json sceneDescription { nlohmann::json::parse(jsonFileStream) };
    jsonFileStream.close();

    return ResourceDatabase::constructAnonymousResource<SceneNode>({
        { "type", SimObject::getResourceTypeName() },
        { "method", SceneFromDescription::getResourceConstructorName() },
        { "parameters", sceneDescription[0].get<nlohmann::json>() },
    });
}

std::shared_ptr<IResource> SceneFromDescription::createResource(const nlohmann::json& sceneDescription) {
    // Load resources described by this scene
    for(const nlohmann::json& resourceDescription: sceneDescription.at("resources")) {
        assert(
            resourceDescription.at("type").get<std::string>() != SceneNode::getResourceTypeName()
            && "Resource section cannot contain descriptions of scene nodes, only SimObjects loaded via files"
        );
        if(resourceDescription.at("type") == SimObject::getResourceTypeName()) {
            assert(
                resourceDescription.at("method") == SceneFromFile::getResourceConstructorName()
                && "Only scenes loaded from files may be defined in the resources section of a scene"
            );
        }
        ResourceDatabase::addResourceDescription(resourceDescription);
    }
    
    assert(sceneDescription.at("nodes").is_array() && "A scene's \"nodes\" property must be an array of node descriptions");

    auto nodeDescriptionIterator { sceneDescription.at("nodes").cbegin() };
    const nlohmann::json& localRootDescription { nodeDescriptionIterator.value() };
    std::shared_ptr<SimObject> localRoot { nullptr };

    assert(localRootDescription.at("parent").get<std::string>() == "" 
        && "Root node must not have a parent"
    );
    assert(localRootDescription.at("type") == SimObject::getResourceTypeName() && "Scene's root must be a sim object");
    localRoot = ResourceDatabase::constructAnonymousResource<SimObject>({
        {"type", SimObject::getResourceTypeName()},
        {"method", SimObjectFromDescription::getResourceConstructorName()},
        {"parameters", localRootDescription},
    });

    for(++nodeDescriptionIterator; nodeDescriptionIterator != sceneDescription.at("nodes").cend(); ++nodeDescriptionIterator) {
        const nlohmann::json& nodeDescription { nodeDescriptionIterator.value() };
        std::shared_ptr<SceneNodeCore> node { nullptr };

        assert(
            nodeDescription.at("parent").get<std::string>() != "" 
            && "All scene nodes besides the root must specify a parent node present in the same scene file"
        );

        // construct scene node resource according to its type
        if(nodeDescription.at("type") == SimObject::getResourceTypeName()) {
            node = ResourceDatabase::constructAnonymousResource<SimObject>({
                {"type", SimObject::getResourceTypeName()},
                {"method", SimObjectFromDescription::getResourceConstructorName()},
                {"parameters", nodeDescription},
            });
        } else if(nodeDescription.at("type") == SceneNode::getResourceTypeName()) {
            node = ResourceDatabase::constructAnonymousResource<SceneNode>({
                {"type", SceneNode::getResourceTypeName()},
                {"method", SceneNodeFromDescription::getResourceConstructorName()},
                {"parameters", nodeDescription},
            });
        // TODO: make it so that certain resource constructors can define resource aliases
        // that refer to the same type (and by extension any tables associated with it)
        } else if(nodeDescription.at("type") == "Scene") {
            node = ResourceDatabase::getRegisteredResource<SimObject>(
                nodeDescription.at("name").get<std::string>()
            );
            if(nodeDescription.at("copy").get<bool>()) {
                node = SimObject::copy(std::static_pointer_cast<SimObject>(node));
            }
        } else {
            assert(false && "Scene nodes in file must be scene nodes, sim objects, or reference to a scene node file resource");
        }
        localRoot->addNode(node, nodeDescription.at("parent").get<std::string>());
    }

    return localRoot;
}

std::shared_ptr<IResource> SceneNodeFromDescription::createResource(const nlohmann::json& methodParams) {
    return SceneNode::create(methodParams);
}

std::shared_ptr<IResource> SimObjectFromDescription::createResource(const nlohmann::json& methodParameters) {
    return SimObject::create(methodParameters);
}
