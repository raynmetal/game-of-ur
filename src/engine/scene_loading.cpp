#include <fstream>
#include <filesystem>

#include "scene_loading.hpp"

std::shared_ptr<IResource> SceneFromFile::createResource(const nlohmann::json& sceneFileDescription) {
    std::string scenePath { sceneFileDescription.at("path").get<std::string>() };
    std::ifstream jsonFileStream;

    jsonFileStream.open(scenePath);
    nlohmann::json sceneDescription { nlohmann::json::parse(jsonFileStream)[0].get<nlohmann::json>() };
    jsonFileStream.close();

    return ResourceDatabase::constructAnonymousResource<SceneNode>({
        { "type", SceneNode::getResourceTypeName() },
        { "method", SceneNodeFromDescription::getResourceConstructorName() },
        { "parameters", sceneDescription },
    });
}

std::shared_ptr<IResource> SceneFromDescription::createResource(const nlohmann::json& sceneDescription) {
    // Load resources described by this scene
    for(const nlohmann::json& resourceDescription: sceneDescription.at("resources")) {
        assert(
            resourceDescription.at("type").get<std::string>() != SimObject::getResourceTypeName()
            && "Resource section cannot contain SimObject descriptions"
        );
        if(resourceDescription.at("type").get<std::string>() == SceneNode::getResourceTypeName()) {
            assert(
                resourceDescription.at("method") == SceneFromFile::getResourceConstructorName()
                && "Only scene nodes loaded from files may be defined in the resources section of a scene"
            );
        }
        ResourceDatabase::addResourceDescription(resourceDescription);
    }
    
    assert(sceneDescription.at("nodes").is_array() && "A scene's \"nodes\" property must be an array of node descriptions");

    auto nodeDescriptionIterator { sceneDescription.at("nodes").cbegin() };
    const nlohmann::json& localRootDescription { nodeDescriptionIterator.value() };
    std::shared_ptr<SceneNode> localRoot { nullptr };

    assert(localRootDescription.at("parent").get<std::string>() == "" 
        && "Root node must not have a parent"
    );
    if(localRootDescription.at("type") == SimObject::getResourceTypeName()) {
        localRoot = ResourceDatabase::constructAnonymousResource<SimObject>({
            {"type", SimObject::getResourceTypeName()},
            {"method", SimObjectFromDescription::getResourceConstructorName()},
            {"parameters", localRootDescription},
        });

    } else if(localRootDescription.at("type") == SceneNode::getResourceTypeName()) {
        localRoot = ResourceDatabase::constructAnonymousResource<SceneNode>({
            {"type", SceneNode::getResourceTypeName()},
            {"method", SceneNodeFromDescription::getResourceConstructorName()},
            {"parameters", localRootDescription},
        });

    } else {
        assert(false && "Scene's root node must either be a scene node or sim object");
    }

    for(++nodeDescriptionIterator; nodeDescriptionIterator != sceneDescription.at("nodes").cend(); ++nodeDescriptionIterator) {
        const nlohmann::json& nodeDescription { nodeDescriptionIterator.value() };
        std::shared_ptr<SceneNode> node { nullptr };

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
        // TODO: make it so that certain resource constructors can define resource type 
        // Aliases that refer to the same type (and therefor any tables associated with 
        // it)
        } else if(nodeDescription.at("type") == "Scene") {
            node = ResourceDatabase::getRegisteredResource<SceneNode>(
                nodeDescription.at("name").get<std::string>()
            );
            if(nodeDescription.at("copy").get<bool>()) {
                node = SceneNode::copy(node);
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
    // SceneNode shared pointer control block created through SimObject::create
    std::shared_ptr<SimObject> simObjectPtr { SimObject::create(methodParameters) };

    // Resource<SimObject> shared pointer shares reference count with SceneNode shared pointer
    // control block
    return std::static_pointer_cast<Resource<SimObject>, SimObject>(simObjectPtr);
}
