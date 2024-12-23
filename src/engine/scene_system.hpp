#ifndef ZOSCENESYSTEM_H
#define ZOSCENESYSTEM_H

#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "simple_ecs.hpp"
#include "resource_database.hpp"
#include "apploop_events.hpp"
#include "scene_components.hpp"

class SceneNode;
class SceneSystem;

enum class RelativeTo : uint8_t {
    PARENT=0,
    // WORLD=1,
    // CAMERA=2,
};

NLOHMANN_JSON_SERIALIZE_ENUM(RelativeTo, {
    {RelativeTo::PARENT, "parent"},
})

enum SpecialEntity: EntityID {
    ENTITY_ROOT = kMaxEntities,
};

class SceneNode: public std::enable_shared_from_this<SceneNode>, public Resource<SceneNode> {
public:
    template<typename ...TComponents>
    static std::shared_ptr<SceneNode> create(const Placement& placement, const std::string& name,  TComponents...components);
    static std::shared_ptr<SceneNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<SceneNode> copy(const std::shared_ptr<const SceneNode> sceneNode);


    virtual ~SceneNode()=default;

    template <typename TComponent>
    void addComponent(const TComponent& component, const bool bypassSceneActivityCheck=false);

    void addComponent(const nlohmann::json& jsonComponent, const bool bypassSceneActivityCheck=false);

    template <typename TComponent>
    TComponent getComponent() const;

    template <typename TComponent>
    bool hasComponent() const;

    template <typename TComponent>
    void updateComponent(const TComponent& component);

    template <typename TComponent>
    void removeComponent();

    template <typename TSystem>
    void setEnabled(bool state);
    template <typename TSystem>
    bool getEnabled() const;

    EntityID getEntityID() const;

    bool inScene() const;
    bool isActive() const;
    bool isAncestorOf(std::shared_ptr<const SceneNode> sceneNode) const;
    void addNode(std::shared_ptr<SceneNode> node, const std::string& where);
    std::vector<std::shared_ptr<SceneNode>> getChildren();
    std::vector<std::shared_ptr<const SceneNode>> getChildren() const;
    std::vector<std::shared_ptr<SceneNode>> getDescendants();
    std::shared_ptr<SceneNode> getNode(const std::string& where);
    std::shared_ptr<SceneNode> getParentNode();
    std::shared_ptr<SceneNode> removeNode(const std::string& where);

    const std::string getName() const;

    static inline std::string getResourceTypeName() { return "SceneNode"; }

protected:
    template<typename ...TComponents>
    SceneNode(const Placement& placement, const std::string& name, TComponents...components);
    SceneNode(const nlohmann::json& jsonSceneNode);

    SceneNode(const SceneNode& sceneObject);

    // // TODO: sit down and figure out whether this operator will ever actually
    // // be used
    // SceneNode& operator=(const SceneNode& sceneObject);

    static void validateName(const std::string& nodeName);

private:
    SceneNode():
    Resource<SceneNode>{0}
    {} // special constructor used to create the root node in the scene system

    virtual std::shared_ptr<SceneNode> clone() const;
    void copyDescendants(const SceneNode& other);

    static bool detectCycle(std::shared_ptr<SceneNode> node);
    static std::tuple<std::string, std::string> nextInPath(const std::string& where);

    void copyAndReplaceAttributes(const SceneNode& other);

    std::string mName {};
    bool mEnabled { true };
    RelativeTo mRelativeTo{ RelativeTo::PARENT };
    std::shared_ptr<Entity> mEntity { nullptr };
    std::shared_ptr<SceneNode> mParent { nullptr };
    std::unordered_map<std::string, std::shared_ptr<SceneNode>> mChildren {};

    Signature mSystemMask {Signature{}.set()};
friend class SceneSystem;
};

class SceneSystem : public System<SceneSystem, Placement, Transform> {
public:
    SceneSystem():
    System<SceneSystem, Placement, Transform>{0}
    {}
    std::shared_ptr<SceneNode> getNode(const std::string& where);
    std::shared_ptr<SceneNode> removeNode(const std::string& where);
    void addNode(std::shared_ptr<SceneNode> node, const std::string& where);

private:
    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        ApploopEventHandler(){}
        inline void initializeEventHandler(SceneSystem* pSystem){ mSystem = pSystem; }
    private:
        void onPreRenderStep(float simulationProgress) override;
        void onApplicationStart() override;
        SceneSystem* mSystem;
    };
    bool isActive(std::shared_ptr<const SceneNode> sceneNode) const;
    bool isActive(EntityID entityID) const;
    bool inScene(std::shared_ptr<const SceneNode> sceneNode) const;
    void markDirty(EntityID entity);
    void updateTransforms();

    Transform getLocalTransform(std::shared_ptr<const SceneNode> sceneNode) const;
    Transform getCachedWorldTransform(std::shared_ptr<const SceneNode> sceneNode) const;
    void nodeAdded(std::shared_ptr<SceneNode> sceneNode);
    void nodeRemoved(std::shared_ptr<SceneNode> sceneNode);
    void nodeActivationChanged(std::shared_ptr<SceneNode> sceneNode, bool state);

    void onEntityUpdated(EntityID entityID) override;

    std::shared_ptr<SceneNode> mRootNode{ new SceneNode {} };

    std::shared_ptr<ApploopEventHandler> mApploopEventHandler { ApploopEventHandler::registerHandler(this) };
    std::unordered_map<EntityID, std::shared_ptr<SceneNode>> mEntityToNode {
        {SpecialEntity::ENTITY_ROOT, mRootNode}
    };
    std::set<EntityID> mActiveEntities { {SpecialEntity::ENTITY_ROOT} };
    std::set<EntityID> mComputeTransformQueue {};

friend class SceneSystem::ApploopEventHandler;
friend class SceneNode;
};

template<typename ...TComponents>
std::shared_ptr<SceneNode> SceneNode::create(const Placement& placement, const std::string& name,  TComponents...components) {
    return std::shared_ptr<SceneNode>( new SceneNode(placement, name, components...));
}

template <typename ...TComponents>
SceneNode::SceneNode(const Placement& placement, const std::string& name, TComponents...components):
Resource<SceneNode>{0}
{
    validateName(name);

    mName = name;
    mEntity = std::make_shared<Entity>(
        SimpleECS::createEntity<Placement, Transform, TComponents...>(
            placement,
            Transform{glm::mat4{1.f}},
            components...
        )
    );
}
template <typename TComponent>
void SceneNode::addComponent(const TComponent& component, bool bypassSceneActivityCheck) {
    mEntity->addComponent<TComponent>(component);

    // NOTE: required because even though this node's entity's signature changes, it
    // is disabled by default on any systems it is eligible for. We need to activate
    // the node according to its system mask
    if(!bypassSceneActivityCheck && isActive()) {
        // TODO: we shouldn't need to visit every node on the tree just because this change
        // has occurred; just the node to which this component was added should be 
        // sufficient
        SimpleECS::getSystem<SceneSystem>()->nodeActivationChanged(
            shared_from_this(), true
        );
    }
}

template <typename TComponent>
TComponent SceneNode::getComponent() const {
    return mEntity->getComponent<TComponent>();
}

template <typename TComponent>
bool SceneNode::hasComponent() const {
    return mEntity->hasComponent<TComponent>();
}

template <typename TComponent>
void SceneNode::updateComponent(const TComponent& component) {
    mEntity->updateComponent<TComponent>(component);
}

template <typename TComponent>
void SceneNode::removeComponent() {
    mEntity->removeComponent<TComponent>();
}

template <typename TSystem>
bool SceneNode::getEnabled() const {
    return mEntity->isEnabled<TSystem>();
}

template <typename TSystem>
void SceneNode::setEnabled(bool state) {
    const SystemType systemType { SimpleECS::getSystemType<TSystem>() };
    mSystemMask.set(systemType, state);

    // since the system mask has been changed, we'll want the scene
    // to talk to ECS and make this node visible to the system that
    // was enabled
    if(inScene() && mEnabled){
        SimpleECS::getSystem<SceneSystem>()->nodeActivationChanged(
            shared_from_this(),
            true
        );
    }
}

// Specialization for when the scene system itself is marked
// enabled or disabled
template <>
inline void SceneNode::setEnabled<SceneSystem>(bool state) {
    const SystemType systemType { SimpleECS::getSystemType<SceneSystem>() };
    // TODO: enabled entities are tracked in both SceneSystem's 
    // mActiveNodes and ECS getEnabledEntities, which is 
    // redundant and may eventually cause errors
    mSystemMask.set(systemType, state);
    mEnabled = state;
    SimpleECS::getSystem<SceneSystem>()->nodeActivationChanged(
        shared_from_this(),
        state
    );
}


// Prevent removal of components essential to a scene node
template <>
inline void SceneNode::removeComponent<Placement>() {
    assert(false && "Cannot remove a scene node's placement component");
}
template <>
inline void SceneNode::removeComponent<Transform>() {
    assert(false && "Cannot remove a scene node's transform component");
}

#endif
