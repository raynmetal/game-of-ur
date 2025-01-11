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

class SceneNodeCore;
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

class SceneNodeCore: public std::enable_shared_from_this<SceneNodeCore> {
public:
    virtual ~SceneNodeCore();

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
    bool isAncestorOf(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    void addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where);
    std::vector<std::shared_ptr<SceneNodeCore>> getChildren();
    std::vector<std::shared_ptr<const SceneNodeCore>> getChildren() const;
    std::vector<std::shared_ptr<SceneNodeCore>> getDescendants();
    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> getParentNode();
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);

    const std::string getName() const;

protected:
    static std::shared_ptr<SceneNodeCore> copy(const std::shared_ptr<const SceneNodeCore> other);

    template<typename ...TComponents>
    SceneNodeCore(const Placement& placement, const std::string& name, TComponents...components);
    SceneNodeCore(const nlohmann::json& jsonSceneNode);
    SceneNodeCore(const SceneNodeCore& sceneObject);

    // // TODO: sit down and figure out whether this operator will ever actually
    // // be used
    // BaseSceneNode& operator=(const BaseSceneNode& sceneObject);

    // lifecycle event hooks
    virtual void onCreated(){}
    virtual void onActivated(){}
    virtual void onDeactivated(){}
    virtual void onDestroyed(){}

    static void validateName(const std::string& nodeName);

private:
    SceneNodeCore(){} // special constructor used to create the root node in the scene system

    virtual std::shared_ptr<SceneNodeCore> clone() const;
    void copyDescendants(const SceneNodeCore& other);

    static bool detectCycle(std::shared_ptr<SceneNodeCore> node);
    static std::tuple<std::string, std::string> nextInPath(const std::string& where);

    void copyAndReplaceAttributes(const SceneNodeCore& other);

    std::string mName {};
    bool mEnabled { true };
    RelativeTo mRelativeTo{ RelativeTo::PARENT };
    std::shared_ptr<Entity> mEntity { nullptr };
    std::shared_ptr<SceneNodeCore> mParent { nullptr };
    std::unordered_map<std::string, std::shared_ptr<SceneNodeCore>> mChildren {};

    Signature mSystemMask {Signature{}.set()};
friend class SceneSystem;

template<typename TSceneNode>
friend class BaseSceneNode;
};

template <typename TSceneNode>
class BaseSceneNode: public SceneNodeCore {
public:

protected:
    template <typename ...TComponents>
    static std::shared_ptr<TSceneNode> create(const Placement& placement, const std::string& name, TComponents...components);
    static std::shared_ptr<TSceneNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<TSceneNode> copy(const std::shared_ptr<const TSceneNode> sceneNode);

    template<typename ...TComponents>
    BaseSceneNode(const Placement& placement, const std::string& name, TComponents...components):
    SceneNodeCore{ placement, name, components... }
    {}
    BaseSceneNode(const nlohmann::json& nodeDescription) : SceneNodeCore { nodeDescription } {}
    BaseSceneNode(const SceneNodeCore& other): SceneNodeCore{ other } {}

friend class SceneNodeCore;
};

class SceneNode: public BaseSceneNode<SceneNode>, public Resource<SceneNode> { 
public:
    template <typename ...TComponents>
    static std::shared_ptr<SceneNode> create(const Placement& placement, const std::string& name, TComponents...components);
    static std::shared_ptr<SceneNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<SceneNode> copy(const std::shared_ptr<const SceneNode> other);

    static inline std::string getResourceTypeName() { return "SceneNode"; }

protected:
    template<typename ...TComponents>
    SceneNode(const Placement& placement, const std::string& name, TComponents...components):
    BaseSceneNode<SceneNode>{placement, name, components...},
    Resource<SceneNode>{0}
    {}

    SceneNode(const nlohmann::json& jsonSceneNode):
    BaseSceneNode<SceneNode>{jsonSceneNode},
    Resource<SceneNode>{0}
    {}

    SceneNode(const SceneNode& sceneObject):
    BaseSceneNode<SceneNode>{sceneObject},
    Resource<SceneNode>{0}
    {}
friend class BaseSceneNode<SceneNode>;
};

class SceneSystem : public System<SceneSystem, Placement, Transform> {
public:
    SceneSystem():
    System<SceneSystem, Placement, Transform>{0}
    {}
    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);
    void addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where);

private:
    class ApploopEventHandler : public IApploopEventHandler<ApploopEventHandler> {
    public:
        ApploopEventHandler(){}
        inline void initializeEventHandler(SceneSystem* pSystem){ mSystem = pSystem; }
    private:
        void onPreRenderStep(float simulationProgress) override;
        void onApplicationStart() override;
        void onApplicationEnd() override;
        SceneSystem* mSystem;
    };
    bool isActive(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    bool isActive(EntityID entityID) const;
    bool inScene(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    void markDirty(EntityID entity);
    void updateTransforms();

    Transform getLocalTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    Transform getCachedWorldTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    void nodeAdded(std::shared_ptr<SceneNodeCore> sceneNode);
    void nodeRemoved(std::shared_ptr<SceneNodeCore> sceneNode);
    void nodeActivationChanged(std::shared_ptr<SceneNodeCore> sceneNode, bool state);
    void activateSubtree(std::shared_ptr<SceneNodeCore> sceneNode);
    void deactivateSubtree(std::shared_ptr<SceneNodeCore> sceneNode);

    void onEntityUpdated(EntityID entityID) override;

    std::shared_ptr<SceneNodeCore> mRootNode{ new SceneNodeCore {} };

    std::shared_ptr<ApploopEventHandler> mApploopEventHandler { ApploopEventHandler::registerHandler(this) };
    std::unordered_map<EntityID, std::shared_ptr<SceneNodeCore>> mEntityToNode {
        {SpecialEntity::ENTITY_ROOT, mRootNode}
    };
    std::set<EntityID> mActiveEntities { {SpecialEntity::ENTITY_ROOT} };
    std::set<EntityID> mComputeTransformQueue {};

friend class SceneSystem::ApploopEventHandler;
friend class SceneNodeCore;
};


template <typename TSceneNode>
template <typename ...TComponents>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::create(const Placement& placement, const std::string& name, TComponents...components) {
    std::shared_ptr<SceneNodeCore> newNode ( new TSceneNode(placement, name, components...));
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}

template <typename TSceneNode>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::create(const nlohmann::json& sceneNodeDescription) {
    std::shared_ptr<SceneNodeCore> newNode{ new TSceneNode{ sceneNodeDescription } };
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}

template <typename TSceneNode>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::copy(const std::shared_ptr<const TSceneNode> sceneNode) {
    return std::static_pointer_cast<TSceneNode>(SceneNodeCore::copy(sceneNode));
}

template<typename ...TComponents>
std::shared_ptr<SceneNode> SceneNode::create(const Placement& placement, const std::string& name,  TComponents...components) {
    return BaseSceneNode<SceneNode>::create<TComponents...>(placement, name, components...);
}


template <typename ...TComponents>
SceneNodeCore::SceneNodeCore(const Placement& placement, const std::string& name, TComponents...components):
Resource<SceneNodeCore>{0}
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
void SceneNodeCore::addComponent(const TComponent& component, bool bypassSceneActivityCheck) {
    mEntity->addComponent<TComponent>(component);

    // NOTE: required because even though this node's entity's signature changes, it
    // is disabled by default on any systems it is eligible for. We need to activate
    // the node according to its system mask
    if(!bypassSceneActivityCheck && isActive()) {
        mEntity->enableSystems(mSystemMask);
    }
    // NOTE: no removeComponent() equivalent required, as systems that depend on the removed
    // component will automatically have this entity removed from their list, and hence
    // be disabled
}

template <typename TComponent>
TComponent SceneNodeCore::getComponent() const {
    return mEntity->getComponent<TComponent>();
}

template <typename TComponent>
bool SceneNodeCore::hasComponent() const {
    return mEntity->hasComponent<TComponent>();
}

template <typename TComponent>
void SceneNodeCore::updateComponent(const TComponent& component) {
    mEntity->updateComponent<TComponent>(component);
}

template <typename TComponent>
void SceneNodeCore::removeComponent() {
    mEntity->removeComponent<TComponent>();
}

template <typename TSystem>
bool SceneNodeCore::getEnabled() const {
    return mEntity->isEnabled<TSystem>();
}

template <typename TSystem>
void SceneNodeCore::setEnabled(bool state) {
    const SystemType systemType { SimpleECS::getSystemType<TSystem>() };
    mSystemMask.set(systemType, state);

    // since the system mask has been changed, we'll want the scene
    // to talk to ECS and make this node visible to the system that
    // was enabled, if eligible
    if(state == true && isActive()){
        mEntity->enableSystems(mSystemMask);
    }
}

// Specialization for when the scene system itself is marked
// enabled or disabled
template <>
inline void SceneNodeCore::setEnabled<SceneSystem>(bool state) {
    const SystemType systemType { SimpleECS::getSystemType<SceneSystem>() };
    // TODO: enabled entities are tracked in both SceneSystem's 
    // mActiveNodes and ECS getEnabledEntities, which is 
    // redundant and may eventually cause errors
    mSystemMask.set(systemType, state);
    mEnabled = state; // TODO: more redundancy; why?
    SimpleECS::getSystem<SceneSystem>()->nodeActivationChanged(
        shared_from_this(),
        state
    );
}


// Prevent removal of components essential to a scene node
template <>
inline void SceneNodeCore::removeComponent<Placement>() {
    assert(false && "Cannot remove a scene node's placement component");
}
template <>
inline void SceneNodeCore::removeComponent<Transform>() {
    assert(false && "Cannot remove a scene node's transform component");
}

#endif
