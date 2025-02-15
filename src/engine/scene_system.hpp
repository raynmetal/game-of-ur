#ifndef ZOSCENESYSTEM_H
#define ZOSCENESYSTEM_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ecs_world.hpp"
#include "texture.hpp"
#include "resource_database.hpp"
#include "input_system/input_system.hpp"
#include "scene_components.hpp"

class SceneNodeCore;
class SceneNode;
class ViewportNode;
class SceneSystem;

enum class RelativeTo : uint8_t {
    PARENT=0,
    // WORLD=1,
    // CAMERA=2,
};

NLOHMANN_JSON_SERIALIZE_ENUM(RelativeTo, {
    {RelativeTo::PARENT, "parent"},
});

enum SpecialEntity: EntityID {
    ENTITY_ROOT = kMaxEntities,
};

extern const std::string kSceneRootName;

class SceneNodeCore: public std::enable_shared_from_this<SceneNodeCore> {
public:
    static void SceneNodeCore_del_(SceneNodeCore* sceneNode);

    virtual ~SceneNodeCore()=default;

    template <typename TComponent>
    void addComponent(const TComponent& component, const bool bypassSceneActivityCheck=false);

    void addComponent(const nlohmann::json& jsonComponent, const bool bypassSceneActivityCheck=false);

    template <typename TComponent>
    TComponent getComponent(const float simulationProgress=1.f) const;

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
    WorldID getWorldID() const;
    UniversalEntityID getUniversalEntityID() const;
    ECSWorld& getWorld() const;

    // TODO: How can we be certain that the value returned by this method here,
    // and the one in SceneSystem, are both in sync? Yet more redundancy
    // that might trip us up
    bool inScene() const;
    // TODO: How can we be certain that the value returned by this method here,
    // and the one in SceneSystem, are both in sync? Yet more redundancy
    // that might trip us up
    bool isActive() const;

    bool isAncestorOf(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    void addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where);
    std::vector<std::shared_ptr<SceneNodeCore>> getChildren();
    std::vector<std::shared_ptr<const SceneNodeCore>> getChildren() const;
    std::vector<std::shared_ptr<SceneNodeCore>> getDescendants();
    template <typename TObject=std::shared_ptr<SceneNode>>
    TObject getByPath(const std::string& where);
    virtual std::shared_ptr<ViewportNode> getLocalViewport();
    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> getParentNode();
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);
    std::vector<std::shared_ptr<SceneNodeCore>> removeChildren();
    const std::string getName() const;

protected:

    void joinWorld(ECSWorld& world);
    static std::shared_ptr<SceneNodeCore> copy(const std::shared_ptr<const SceneNodeCore> other);

    template<typename ...TComponents>
    SceneNodeCore(const Placement& placement, const std::string& name, TComponents...components);
    SceneNodeCore(const nlohmann::json& jsonSceneNode);
    SceneNodeCore(const SceneNodeCore& sceneObject);

    // // TODO: sit down and figure out whether this operator will ever actually
    // // be used
    // BaseSceneNode& operator=(const BaseSceneNode& sceneObject);

    // lifecycle event hooks
    virtual void onCreated();
    virtual void onActivated();
    virtual void onDeactivated();
    virtual void onDestroyed();

    static void validateName(const std::string& nodeName);
private:
    struct Key {};

    template<typename ...TComponents>
    SceneNodeCore(const Key&, const Placement& placement, const std::string& name, TComponents...components);

    enum StateFlags: uint8_t {
        ENABLED=0x1,
        ACTIVE=0x2,
    };

    template <typename TObject, typename Enable=void>
    struct getByPath_Helper {
        static TObject get(std::shared_ptr<SceneNodeCore> rootNode, const std::string& where);
        static constexpr bool s_valid { false };
    };

    virtual std::shared_ptr<SceneNodeCore> clone() const;
    void copyDescendants(const SceneNodeCore& other);

    static void setParentViewport(std::shared_ptr<SceneNodeCore> node, std::shared_ptr<ViewportNode> newViewport);
    static std::shared_ptr<SceneNodeCore> disconnectNode(std::shared_ptr<SceneNodeCore> node);
    static bool detectCycle(std::shared_ptr<SceneNodeCore> node);
    static std::tuple<std::string, std::string> nextInPath(const std::string& where);
    void copyAndReplaceAttributes(const SceneNodeCore& other);

    std::string mName {};
    uint8_t mStateFlags { 0x00 | StateFlags::ENABLED };
    RelativeTo mRelativeTo{ RelativeTo::PARENT };
    std::shared_ptr<Entity> mEntity { nullptr };
    std::weak_ptr<SceneNodeCore> mParent {};
    std::weak_ptr<ViewportNode> mParentViewport {};
    std::unordered_map<std::string, std::shared_ptr<SceneNodeCore>> mChildren {};

    Signature mSystemMask {Signature{}.set()};

friend class SceneSystem;
template<typename TSceneNode>
friend class BaseSceneNode;
friend class ViewportNode;
};


template <typename TSceneNode>
class BaseSceneNode: public SceneNodeCore {
public:

protected:
    template <typename ...TComponents>
    static std::shared_ptr<TSceneNode> create(const Key&, const Placement& placement, const std::string& name, TComponents...components);
    template <typename ...TComponents>
    static std::shared_ptr<TSceneNode> create(const Placement& placement, const std::string& name, TComponents...components);
    static std::shared_ptr<TSceneNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<TSceneNode> copy(const std::shared_ptr<const TSceneNode> sceneNode);
    template <typename...TComponents>

    BaseSceneNode(const Key& key, const Placement& placement, const std::string& name, TComponents...components):
    SceneNodeCore{ key, placement, name, components... }
    {}
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

class ViewportNode: public BaseSceneNode<ViewportNode>, public Resource<ViewportNode> {
public:
    enum class ResizeType: uint8_t {
        OFF=0,
        VIEWPORT_DIMENSIONS, // Viewport transform configured per stretch mode and requested dimensions
        TEXTURE_DIMENSIONS, // Texture result rendered in base dimensions, and then warped to fit request dimensions
    };

    /**
     * Determines which dimensions the end result of the viewport
     * is allowed to expand on. 
     */
    enum class ResizeMode: uint8_t {
        FIXED_ASPECT=0,
        EXPAND_VERTICALLY,
        EXPAND_HORIZONTALLY,
        EXPAND_FILL,
    };

    enum class UpdateMode: uint8_t {
        NEVER=0,
        ONCE,
        ON_FETCH,
        CAP_FPS,
    };

    static std::shared_ptr<ViewportNode> create(const std::string& name, bool inheritsWorld, const glm::u16vec2& baseDimensions);
    static std::shared_ptr<ViewportNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<ViewportNode> copy(const std::shared_ptr<const ViewportNode> other);
    static inline std::string getResourceTypeName() { return "ViewportNode"; }

    std::shared_ptr<ViewportNode> getLocalViewport() override;
    std::shared_ptr<Texture> fetchRenderResult(float simulationProgress, uint32_t variableStep);

    void requestDimensions(glm::u16vec2 requestedDimensions);
    void setResizeType(ResizeType type);
    void setResizeMode(ResizeMode mode);
    void setRenderScale(float renderScale);
    void setUpdateMode(UpdateMode updateMode);
    void setFPSCap(float fpsCap);

    ActionDispatch& getActionDispatch();
protected:

    ViewportNode(const Placement& placement, const std::string& name):
    BaseSceneNode<ViewportNode>{placement, name},
    Resource<ViewportNode>{0}
    {}
    ViewportNode(const nlohmann::json& jsonSceneNode):
    BaseSceneNode<ViewportNode>{jsonSceneNode},
    Resource<ViewportNode>{0}
    {}
    ViewportNode(const ViewportNode& sceneObject):
    BaseSceneNode<ViewportNode>{sceneObject},
    Resource<ViewportNode>{0}
    {}
    std::unique_ptr<ECSWorld> mOwnWorld { nullptr };
    void onActivated() override;
    void onDeactivated() override;

private:
    static std::shared_ptr<ViewportNode> create(const Key& key, const std::string& name, bool inheritsWorld, const glm::u16vec2& baseDimensions);
    ViewportNode(const Key& key, const Placement& placement, const std::string& name):
    BaseSceneNode<ViewportNode>{key, Placement{}, name},
    Resource<ViewportNode>{0}
    {}
    std::shared_ptr<SceneNodeCore> clone() const override;

    void createAndJoinWorld();

    ActionDispatch mActionDispatch {};
    std::set<std::shared_ptr<ViewportNode>, std::owner_less<std::shared_ptr<ViewportNode>>> mChildViewports {};

    std::shared_ptr<Texture> mTextureResult { nullptr };

    ResizeType mResizeType { ResizeType::TEXTURE_DIMENSIONS };
    ResizeMode mResizeMode { ResizeMode::EXPAND_VERTICALLY };
    glm::u16vec2 mBaseDimensions { 800, 600 };
    glm::u16vec2 mComputedDimensions { 800, 600 };
    glm::u16vec2 mRequestedDimensions { 800, 600 };

    UpdateMode mUpdateMode { UpdateMode::CAP_FPS };
    float mFPSCap { 60.f };
    float mRenderScale { .3f };
    uint32_t mTimeSinceLastRender { static_cast<uint32_t>(1000/mFPSCap) };

friend class BaseSceneNode<ViewportNode>;
friend class SceneNodeCore;
friend class SceneSystem;
};

NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::ResizeType, {
    {ViewportNode::ResizeType::OFF, "off"},
    {ViewportNode::ResizeType::VIEWPORT_DIMENSIONS, "per-subobject"},
    {ViewportNode::ResizeType::TEXTURE_DIMENSIONS, "post-viewport-render"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::ResizeMode, {
    {ViewportNode::ResizeMode::FIXED_ASPECT,"fixed-aspect"},
    {ViewportNode::ResizeMode::EXPAND_VERTICALLY, "expand-vertically"},
    {ViewportNode::ResizeMode::EXPAND_HORIZONTALLY, "expand-horizontally"},
    {ViewportNode::ResizeMode::EXPAND_FILL, "expand-fill"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::UpdateMode, {
    {ViewportNode::UpdateMode::NEVER, "never"},
    {ViewportNode::UpdateMode::ONCE, "once"},
    {ViewportNode::UpdateMode::ON_FETCH, "on-fetch"},
    {ViewportNode::UpdateMode::CAP_FPS, "cap-fps"},
});

class SceneSystem: public System<SceneSystem, Placement, Transform> {
public:
    SceneSystem(ECSWorld& world):
    System<SceneSystem, Placement, Transform> { world }
    { }

    static std::string getSystemTypeName() { return "SceneSystem"; }

    bool isSingleton() const override { return true; }

    template<typename TObject=std::shared_ptr<SceneNode>>
    TObject getByPath(const std::string& where);

    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);
    void addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where);


    ECSWorld& getRootWorld() const;
    ViewportNode& getRootViewport() const;

    void onApplicationInitialize();
    void onApplicationStart();

    void simulate(uint32_t simStepMillis, std::vector<std::pair<ActionDefinition, ActionData>> triggeredActions={});
    void variableStep(float simulationProgress, uint32_t variableStepMillis);
    void updateTransforms();
    void render(float simulationProgress, uint32_t variableStep);

    void onApplicationEnd();

private:
    class SceneSubworld: public System<SceneSubworld, Placement, Transform> {
    public:
        SceneSubworld(ECSWorld& world):
        System<SceneSubworld, Placement, Transform> { world }
        {}
        static std::string getSystemTypeName() { return "SceneSubworld"; }
    private:
        void onEntityUpdated(EntityID entityID) override;
    };
    bool isActive(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    bool isActive(UniversalEntityID UniversalEntityID) const;
    bool inScene(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    bool inScene(UniversalEntityID UniversalEntityID) const;
    void markDirty(UniversalEntityID UniversalEntityID);

    Transform getLocalTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    Transform getCachedWorldTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    void nodeAdded(std::shared_ptr<SceneNodeCore> sceneNode);
    void nodeRemoved(std::shared_ptr<SceneNodeCore> sceneNode);
    void nodeActivationChanged(std::shared_ptr<SceneNodeCore> sceneNode, bool state);
    void activateSubtree(std::shared_ptr<SceneNodeCore> sceneNode);
    void deactivateSubtree(std::shared_ptr<SceneNodeCore> sceneNode);

    void onWorldEntityUpdate(UniversalEntityID UniversalEntityID);


    std::shared_ptr<ViewportNode> mRootNode{ nullptr };

    std::map<UniversalEntityID, std::shared_ptr<SceneNodeCore>, std::less<UniversalEntityID>> mEntityToNode {};
    std::set<UniversalEntityID, std::less<UniversalEntityID>> mActiveEntities {};
    std::set<UniversalEntityID, std::less<UniversalEntityID>> mComputeTransformQueue {};

friend class SceneNodeCore;
};


template <typename TSceneNode>
template <typename ...TComponents>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::create(const Placement& placement, const std::string& name, TComponents...components) {
    std::shared_ptr<SceneNodeCore> newNode ( new TSceneNode(placement, name, components...), &SceneNodeCore_del_);
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}
template <typename TSceneNode>
template <typename ...TComponents>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::create(const Key& key, const Placement& placement, const std::string& name, TComponents...components) {
    std::shared_ptr<SceneNodeCore> newNode( new TSceneNode(key, placement, name, components...), &SceneNodeCore_del_);
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}

template <typename TSceneNode>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::create(const nlohmann::json& sceneNodeDescription) {
    std::shared_ptr<SceneNodeCore> newNode{ new TSceneNode{ sceneNodeDescription }, &SceneNodeCore_del_};
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}

template <typename TSceneNode>
std::shared_ptr<TSceneNode> BaseSceneNode<TSceneNode>::copy(const std::shared_ptr<const TSceneNode> sceneNode) {
    std::shared_ptr<SceneNodeCore> newNode { SceneNodeCore::copy(sceneNode) };
    newNode->onCreated();
    return std::static_pointer_cast<TSceneNode>(newNode);
}

template<typename ...TComponents>
std::shared_ptr<SceneNode> SceneNode::create(const Placement& placement, const std::string& name,  TComponents...components) {
    return BaseSceneNode<SceneNode>::create<TComponents...>(placement, name, components...);
}


template <typename TObject>
TObject SceneNodeCore::getByPath(const std::string& where) {
    return getByPath_Helper<TObject>::get(shared_from_this(), where);
}

template <typename TObject>
TObject SceneSystem::getByPath(const std::string& where) {
    return mRootNode->getByPath<TObject>(where);
}

// Fail retrieval in cases where no explicitly defined object by path
// method exists
template <typename TObject, typename Enable>
TObject SceneNodeCore::getByPath_Helper<TObject, Enable>::get(std::shared_ptr<SceneNodeCore> rootNode, const std::string& where) {
    static_assert(false && "No Object-by-Path method for this type exists");
    return TObject{}; // this is just to shut the compiler up about no returned value
}

template <typename TObject>
struct SceneNodeCore::getByPath_Helper<std::shared_ptr<TObject>, typename std::enable_if_t<std::is_base_of<SceneNodeCore, TObject>::value>> {
    static std::shared_ptr<TObject> get(std::shared_ptr<SceneNodeCore> rootNode, const std::string& where) {
        return std::static_pointer_cast<TObject>(rootNode->getNode(where));
    }
    static constexpr bool s_valid { true };
};

template <typename ...TComponents>
SceneNodeCore::SceneNodeCore(const Placement& placement, const std::string& name, TComponents...components) {
    validateName(name);
    mName = name;
    mEntity = std::make_shared<Entity>(
        ECSWorld::createEntityPrototype<Placement, Transform, TComponents...>(
            placement,
            Transform{glm::mat4{1.f}},
            components...
        )
    );
}
template <typename ...TComponents>
SceneNodeCore::SceneNodeCore(const Key&, const Placement& placement, const std::string& name, TComponents...components) {
    mName = name;
    mEntity = std::make_shared<Entity>(
        ECSWorld::createEntityPrototype<Placement, Transform, TComponents...>(
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
TComponent SceneNodeCore::getComponent(const float simulationProgress) const {
    return mEntity->getComponent<TComponent>(simulationProgress);
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
    const SystemType systemType { mEntity->getWorld().getSystemType<TSystem>() };
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
    const SystemType systemType { mEntity->getWorld().getSystemType<SceneSystem>() };
    // TODO: enabled entities are tracked in both SceneSystem's 
    // mActiveNodes and ECS getEnabledEntities, which is 
    // redundant and may eventually cause errors
    mSystemMask.set(systemType, state);
    //TODO: More redundancy. Why?
    mStateFlags = state? (mStateFlags | SceneNodeCore::StateFlags::ENABLED): (mStateFlags & ~SceneNodeCore::StateFlags::ENABLED);
    mEntity->getWorld().getSystem<SceneSystem>()->nodeActivationChanged(
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
