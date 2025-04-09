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

#include "scene_components.hpp"
#include "spatial_query_math.hpp"
#include "render_system.hpp"
#include "ecs_world.hpp"
#include "texture.hpp"
#include "resource_database.hpp"
#include "input_system/input_system.hpp"

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
    ENTITY_NULL = kMaxEntities,
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
    std::weak_ptr<ECSWorld> getWorld() const;

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
    std::string getPathFromAncestor(std::shared_ptr<const SceneNodeCore> ancestor);
    virtual std::shared_ptr<ViewportNode> getLocalViewport();
    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> getParentNode();
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);
    std::vector<std::shared_ptr<SceneNodeCore>> removeChildren();
    const std::string getName() const;

protected:

    virtual void joinWorld(ECSWorld& world);
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
    struct RenderConfiguration {
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
            FIXED_ASPECT=0, // both, while retaining aspect ratio
            EXPAND_VERTICALLY,
            EXPAND_HORIZONTALLY, 
            EXPAND_FILL, // no constraint in either dimension
        };

        enum class UpdateMode: uint8_t {
            NEVER=0,
            ONCE, // update on next render frame, then set to never
            ON_FETCH, // update whenever a request for the texture is made
            ON_FETCH_CAP_FPS, // update on request, but ignore requests exceeding FPS cap
            ON_RENDER, // update every render call
            ON_RENDER_CAP_FPS, // update on render call when fps cap isn't exceeded
        };

        ResizeType mResizeType { ResizeType::VIEWPORT_DIMENSIONS };
        ResizeMode mResizeMode { ResizeMode::EXPAND_HORIZONTALLY };
        glm::u16vec2 mBaseDimensions { 800, 600 };
        glm::u16vec2 mComputedDimensions { 800, 600 };
        glm::u16vec2 mRequestedDimensions { 800, 600 };
        float mRenderScale { 1.f };

        UpdateMode mUpdateMode { UpdateMode::ON_RENDER_CAP_FPS };
        float mFPSCap { 60.f };
    };

    static std::shared_ptr<ViewportNode> create(const std::string& name, bool inheritsWorld, const RenderConfiguration& renderConfiguration);
    static std::shared_ptr<ViewportNode> create(const nlohmann::json& sceneNodeDescription);
    static std::shared_ptr<ViewportNode> copy(const std::shared_ptr<const ViewportNode> other);
    static inline std::string getResourceTypeName() { return "ViewportNode"; }

    std::shared_ptr<ViewportNode> getLocalViewport() override;
    std::shared_ptr<Texture> fetchRenderResult(float simulationProgress);
    void setActiveCamera(const std::string& cameraPath);
    void setActiveCamera(std::shared_ptr<SceneNodeCore> cameraNode);

    RenderConfiguration getRenderConfiguration() const;
    void setRenderConfiguration(const RenderConfiguration& renderConfiguration);
    void setResizeType(RenderConfiguration::ResizeType type);
    void setResizeMode(RenderConfiguration::ResizeMode mode);
    void setRenderScale(float renderScale);
    void setUpdateMode(RenderConfiguration::UpdateMode updateMode);
    void setFPSCap(float fpsCap);

    void requestDimensions(glm::u16vec2 requestedDimensions);

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
    std::shared_ptr<ECSWorld> mOwnWorld { nullptr };
    void onActivated() override;
    void onDeactivated() override;
    void onDestroyed() override;

    void joinWorld(ECSWorld& world) override;

private:
    static std::shared_ptr<ViewportNode> create(const Key& key, const std::string& name, bool inheritsWorld, const RenderConfiguration& renderConfiguration);
    ViewportNode(const Key& key, const Placement& placement, const std::string& name):
    BaseSceneNode<ViewportNode>{key, Placement{}, name},
    Resource<ViewportNode>{0}
    {}

    std::shared_ptr<SceneNodeCore> clone() const override;

    void createAndJoinWorld();
    void registerDomainCamera(std::shared_ptr<SceneNodeCore> cameraNode);
    void unregisterDomainCamera(std::shared_ptr<SceneNodeCore> cameraNode);
    std::shared_ptr<SceneNodeCore> findFallbackCamera();

    std::vector<std::shared_ptr<ViewportNode>> getActiveDescendantViewports();
    std::vector<std::weak_ptr<ECSWorld>> getActiveDescendantWorlds();

    std::shared_ptr<Texture> render(float simulationProgress, uint32_t variableStep);

    ActionDispatch mActionDispatch {};
    std::set<std::shared_ptr<ViewportNode>, std::owner_less<std::shared_ptr<ViewportNode>>> mChildViewports {};
    std::shared_ptr<SceneNodeCore> mActiveCamera { nullptr };
    std::set<std::shared_ptr<SceneNodeCore>, std::owner_less<std::shared_ptr<SceneNodeCore>>> mDomainCameras {};
    RenderSetID mRenderSet;

    std::shared_ptr<Texture> mTextureResult { nullptr };
    RenderConfiguration mRenderConfiguration {};

    uint32_t mTimeSinceLastRender { static_cast<uint32_t>(1000/mRenderConfiguration.mFPSCap) };

friend class BaseSceneNode<ViewportNode>;
friend class SceneNodeCore;
friend class SceneSystem;
};

class SceneSystem: public System<SceneSystem, std::tuple<>, std::tuple<Placement, SceneHierarchyData, Transform>> {
public:
    SceneSystem(std::weak_ptr<ECSWorld> world):
    System<SceneSystem, std::tuple<>, std::tuple<Placement, SceneHierarchyData, Transform>> { world }
    {}

    static std::string getSystemTypeName() { return "SceneSystem"; }

    bool isSingleton() const override { return true; }

    template<typename TObject=std::shared_ptr<SceneNode>>
    TObject getByPath(const std::string& where);

    std::shared_ptr<SceneNodeCore> getNode(const std::string& where);
    std::shared_ptr<SceneNodeCore> removeNode(const std::string& where);
    void addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where);


    std::weak_ptr<ECSWorld> getRootWorld() const;
    ViewportNode& getRootViewport() const;

    void onApplicationInitialize(const ViewportNode::RenderConfiguration& rootViewportRenderConfiguration);
    void onApplicationStart();
    void onApplicationEnd();

    void simulationStep(uint32_t simStepMillis, std::vector<std::pair<ActionDefinition, ActionData>> triggeredActions={});
    void variableStep(float simulationProgress, uint32_t simulationLagMillis, uint32_t variableStepMillis, std::vector<std::pair<ActionDefinition, ActionData>> triggeredActions={});
    void updateTransforms();
    void render(float simulationProgress, uint32_t variableStep);

private:
    class SceneSubworld: public System<SceneSubworld, std::tuple<Placement>, std::tuple<Transform, SceneHierarchyData>> {
    public:
        SceneSubworld(std::weak_ptr<ECSWorld> world):
        System<SceneSubworld, std::tuple<Placement>, std::tuple<Transform, SceneHierarchyData>> { world }
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

    std::vector<std::shared_ptr<ViewportNode>> getActiveViewports();
    std::vector<std::weak_ptr<ECSWorld>> getActiveWorlds();

    Transform getLocalTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;
    Transform getCachedWorldTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const;

    void updateHierarchyDataInsertion(std::shared_ptr<SceneNodeCore> insertedNode);
    void updateHierarchyDataRemoval(std::shared_ptr<SceneNodeCore> removedNode);

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
        ECSWorld::createEntityPrototype<Placement, SceneHierarchyData, Transform, ObjectBounds, AxisAlignedBounds, TComponents...>(
            placement,
            SceneHierarchyData{},
            Transform{glm::mat4{1.f}},
            ObjectBounds {},
            AxisAlignedBounds {},
            components...
        )
    );
}
template <typename ...TComponents>
SceneNodeCore::SceneNodeCore(const Key&, const Placement& placement, const std::string& name, TComponents...components) {
    mName = name;
    mEntity = std::make_shared<Entity>(
        ECSWorld::createEntityPrototype<Placement, SceneHierarchyData, Transform, ObjectBounds, AxisAlignedBounds, TComponents...>(
            placement,
            SceneHierarchyData{},
            Transform{glm::mat4{1.f}},
            ObjectBounds {},
            AxisAlignedBounds{},
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
    const SystemType systemType { mEntity->getWorld().lock()->getSystemType<TSystem>() };
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
    const SystemType systemType { mEntity->getWorld().lock()->getSystemType<SceneSystem>() };
    // TODO: enabled entities are tracked in both SceneSystem's 
    // mActiveNodes and ECS getEnabledEntities, which is 
    // redundant and may eventually cause errors
    mSystemMask.set(systemType, state);
    //TODO: More redundancy. Why?
    mStateFlags = state? (mStateFlags | SceneNodeCore::StateFlags::ENABLED): (mStateFlags & ~SceneNodeCore::StateFlags::ENABLED);
    mEntity->getWorld().lock()->getSystem<SceneSystem>()->nodeActivationChanged(
        shared_from_this(),
        state
    );
}

// Prevent removal of components essential to a scene node
template <>
inline void SceneNodeCore::removeComponent<Placement>() {
    assert(false && "Cannot remove a scene node's Placement component");
}
template <>
inline void SceneNodeCore::removeComponent<Transform>() {
    assert(false && "Cannot remove a scene node's Transform component");
}


NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::RenderConfiguration::ResizeType, {
    {ViewportNode::RenderConfiguration::ResizeType::OFF, "off"},
    {ViewportNode::RenderConfiguration::ResizeType::VIEWPORT_DIMENSIONS, "viewport-dimensions"},
    {ViewportNode::RenderConfiguration::ResizeType::TEXTURE_DIMENSIONS, "texture-dimensions"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::RenderConfiguration::ResizeMode, {
    {ViewportNode::RenderConfiguration::ResizeMode::FIXED_ASPECT,"fixed-aspect"},
    {ViewportNode::RenderConfiguration::ResizeMode::EXPAND_VERTICALLY, "expand-vertically"},
    {ViewportNode::RenderConfiguration::ResizeMode::EXPAND_HORIZONTALLY, "expand-horizontally"},
    {ViewportNode::RenderConfiguration::ResizeMode::EXPAND_FILL, "expand-fill"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(ViewportNode::RenderConfiguration::UpdateMode, {
    {ViewportNode::RenderConfiguration::UpdateMode::NEVER, "never"},
    {ViewportNode::RenderConfiguration::UpdateMode::ONCE, "once"},
    {ViewportNode::RenderConfiguration::UpdateMode::ON_FETCH, "on-fetch"},
    {ViewportNode::RenderConfiguration::UpdateMode::ON_RENDER, "on-render"},
    {ViewportNode::RenderConfiguration::UpdateMode::ON_RENDER_CAP_FPS, "on-render-cap-fps"},
});

inline void to_json(nlohmann::json& json, const ViewportNode::RenderConfiguration& renderConfiguration) {
    json = {
        {"base_dimensions", nlohmann::json::array({renderConfiguration.mBaseDimensions.x, renderConfiguration.mBaseDimensions.y})},
        {"update_mode", renderConfiguration.mUpdateMode},
        {"resize_type", renderConfiguration.mResizeType},
        {"resize_mode", renderConfiguration.mResizeMode},
        {"render_scale", renderConfiguration.mRenderScale},
        {"fps_cap", renderConfiguration.mFPSCap},
    };
}

inline void from_json(const nlohmann::json& json, ViewportNode::RenderConfiguration& renderConfiguration) {
    assert(json.find("base_dimensions") != json.end() && "Viewport descriptions must contain the \"base_dimensions\" size 2 array of Numbers attribute");
    json.at("base_dimensions")[0].get_to(renderConfiguration.mBaseDimensions.x);
    json.at("base_dimensions")[1].get_to(renderConfiguration.mBaseDimensions.y);
    renderConfiguration.mRequestedDimensions = renderConfiguration.mBaseDimensions;
    renderConfiguration.mComputedDimensions = renderConfiguration.mBaseDimensions;
    assert(renderConfiguration.mBaseDimensions.x > 0 && renderConfiguration.mBaseDimensions.y > 0 && "Base dimensions cannot include a 0 in either dimension");

    assert(json.find("update_mode") != json.end() && "Viewport render configuration must include the \"update_mode\" enum attribute");
    json.at("update_mode").get_to(renderConfiguration.mUpdateMode);

    assert(json.find("resize_type") != json.end() && "Viewport render configuration must include the \"resize_type\" enum attribute");
    json.at("resize_type").get_to(renderConfiguration.mResizeType);

    assert(json.find("resize_mode") != json.end() && "Viewport render configuration must include the \"resize_mode\" enum attribute");
    json.at("resize_mode").get_to(renderConfiguration.mResizeMode);

    assert(json.find("render_scale") != json.end() && "Viewport render configuration must include the \"render_scale\" float attribute");
    json.at("render_scale").get_to(renderConfiguration.mRenderScale);
    assert(renderConfiguration.mRenderScale > 0.f && "Render scale must be a positive non-zero decimal number");

    assert(json.find("fps_cap") != json.end() && "Viewport must include the \"fps_cap\" float attribute");
    json.at("fps_cap").get_to(renderConfiguration.mFPSCap);
    assert(renderConfiguration.mFPSCap > 0.f && "FPS cap must be a positive non-zero decimal number");
}

#endif
