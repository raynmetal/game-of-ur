#include <vector>
#include <set>
#include <cassert>
#include <memory>

#include "util.hpp"
#include "render_system.hpp"
#include "ecs_world.hpp"
#include "scene_system.hpp"

const std::string kSceneRootName { "" };

void SceneNodeCore::SceneNodeCore_del_(SceneNodeCore* sceneNode) {
    if(!sceneNode) return;
    sceneNode->onDestroyed();
    delete sceneNode;
}

void SceneNodeCore::onCreated(){}
void SceneNodeCore::onActivated(){};
void SceneNodeCore::onDeactivated(){};
void SceneNodeCore::onDestroyed(){};

std::shared_ptr<SceneNodeCore> SceneNodeCore::copy(const std::shared_ptr<const SceneNodeCore> other) {
    if(!other) return nullptr;
    std::shared_ptr<SceneNodeCore> newSceneNode{ other->clone() };
    newSceneNode->copyDescendants(*other);
    return newSceneNode;
}


std::shared_ptr<SceneNode> SceneNode::create(const nlohmann::json& sceneNodeDescription) {
    return BaseSceneNode<SceneNode>::create(sceneNodeDescription);
}

std::shared_ptr<SceneNode> SceneNode::copy(const std::shared_ptr<const SceneNode> sceneNode) {
    return BaseSceneNode<SceneNode>::copy(sceneNode);
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::clone() const {
    // construct a new scene node with the same components as this one
    std::shared_ptr<SceneNodeCore> newSceneNode{ new SceneNodeCore{*this}, &SceneNodeCore_del_ };
    return newSceneNode;
}


SceneNodeCore::SceneNodeCore(const nlohmann::json& sceneNodeDescription)
{
    validateName(sceneNodeDescription.at("name").get<std::string>());
    mName = sceneNodeDescription.at("name").get<std::string>();
    mEntity = std::make_shared<Entity>(
        ECSWorld::createEntityPrototype()
    );
    // bypass own implementation of addComponent. We shouldn't trigger methods that
    // require a shared pointer to this to be present
    addComponent<Transform>({}, true);
    for(const nlohmann::json& componentDescription: sceneNodeDescription.at("components")) {
        addComponent(componentDescription, true);
    }
    assert(hasComponent<Placement>() && "scene nodes must define a placement component");
}

SceneNodeCore::SceneNodeCore(const SceneNodeCore& other)
{
    copyAndReplaceAttributes(other);
}

// NOTE: save this for the day I find a use for it.
// SceneNode& SceneNode::operator=(const SceneNode& other) {
//     // avoid doing anything if other is just a reference to self
//     if(this == &other) return *this;

//     // disconnect self from parent, and if we were 
//     // part of the scene, make scene system forget about
//     // us.
//     std::shared_ptr<SceneNode> parent{ mParent };
//     removeNode("/");

//     copyAndReplaceAttributes(other);

//     // should be safe because if assignment is invoked, a shared
//     // pointer to `this` must already exist
//     copyDescendants(other); 
        

//     // add self back to our parent node complete with our new subtree
//     if(parent) {
//         // should be safe because if assignment is invoked, a shared
//         // pointer to `this` must already exist
//         parent->addNode(shared_from_this(), "/");
//     }

//     return *this;
// }

void SceneNodeCore::copyAndReplaceAttributes(const SceneNodeCore& other) {
    // copy the other entity and its components
    std::shared_ptr<Entity> newEntity { 
        std::make_shared<Entity>(mEntity->getWorld().createEntity())
    };
    newEntity->copy(*(other.mEntity));
    mEntity = newEntity;
    mStateFlags = (other.mStateFlags&StateFlags::ENABLED);
    mChildren.clear();
    mName = other.mName;
    mParent.reset();
    mParentViewport.reset();
    mSystemMask = other.mSystemMask;
}

void SceneNodeCore::copyDescendants(const SceneNodeCore& other) {
    // copy descendant nodes, attach them to self
    for(auto& childPair: other.mChildren) {
        const std::string& childName { childPair.first };
        const std::shared_ptr<const SceneNodeCore> childNode { childPair.second };
        mChildren[childName] = SceneNodeCore::copy(childNode);

        // TODO : somehow make this whole thing less delicate. Shared
        // from this depends on the existence of a shared pointer
        // to the current object
        mChildren[childName]->mParent = shared_from_this();
        mChildren[childName]->mParentViewport = getLocalViewport();
    }
}

void SceneNodeCore::addComponent(const nlohmann::json& jsonComponent, const bool bypassSceneActivityCheck) {
    mEntity->addComponent(jsonComponent);

    // NOTE: required because even though this node's entity's signature changes, it
    // is disabled by default on any systems it is eligible for. We need to activate
    // the node according to its system mask
    if(!bypassSceneActivityCheck && isActive()) {
        mEntity->enableSystems(mSystemMask);
    }
}

bool SceneNodeCore::detectCycle(std::shared_ptr<SceneNodeCore> node) {
    if(!node) return false;

    std::shared_ptr<SceneNodeCore> slow { node };
    std::shared_ptr<SceneNodeCore> fast { slow->mParent.lock() };
    while(fast != nullptr && !fast->mParent.expired()  && slow != fast) {
        slow = slow->mParent.lock();
        fast = fast->mParent.lock()->mParent.lock();
    }

    if(slow == fast) return true;
    return false;
}

bool SceneNodeCore::inScene() const {
    return mStateFlags&StateFlags::ENABLED;
}

bool SceneNodeCore::isActive() const {
    return mStateFlags&StateFlags::ACTIVE;
}

bool SceneNodeCore::isAncestorOf(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    if(!sceneNode || sceneNode.get() == this) return false;

    std::shared_ptr<const SceneNodeCore> currentNode { sceneNode };
    while(currentNode != nullptr && currentNode.get() != this) {
        currentNode = currentNode->mParent.lock();
    }
    return static_cast<bool>(currentNode);
}

const std::string SceneNodeCore::getName() const {
    return mName;
}

std::tuple<std::string, std::string> SceneNodeCore::nextInPath(const std::string& where) {
    // Search for beginning and end of the name of the next node in the specified path
    std::string::const_iterator nextBegin{ where.begin() };
    ++nextBegin;
    std::string::const_iterator nextEnd{ nextBegin };
    for(; nextEnd != where.end() && (*nextEnd) != '/'; ++nextEnd);
    assert(nextEnd != where.end() && "Invalid path not ending in '/' specified");
    assert(nextBegin != nextEnd && "Incomplete path, every node name in the path must be specified separated with '/'");

    // Form the name of the next node, as well as the remaining path left
    // to be traversed
    const std::string nextNodeName { nextBegin, nextEnd };
    const std::string remainingWhere { nextEnd, where.end() };
    return std::tuple<std::string, std::string>{nextNodeName, remainingWhere};
}

void SceneNodeCore::addNode(std::shared_ptr<SceneNodeCore> node, const std::string& where) {
    assert(node && "Must be a non null pointer to a valid scene node");
    assert(node->mParent.expired() && "Node must not have a parent");
    if(where == "/") {
        assert(mChildren.find(node->mName) == mChildren.end() && "A node with this name already exists at this location");
        mChildren[node->mName] = node;
        node->mParent = shared_from_this();
        setParentViewport(node, getLocalViewport());
        assert(!detectCycle(node) && "Cycle detected, ancestor node added as child to its descendant.");
        mEntity->getWorld().getSystem<SceneSystem>()->nodeAdded(node);
        return;
    }

    // descend to the next node in the path
    std::tuple<std::string, std::string> nextPair{ nextInPath(where) };
    const std::string& nextNodeName { std::get<0>(nextPair) };
    const std::string& remainingWhere { std::get<1>(nextPair) };
    assert(mChildren.find(nextNodeName) != mChildren.end() && "No child node with this name is known");

    mChildren.at(nextNodeName)->addNode(node, remainingWhere);
}

std::vector<std::shared_ptr<SceneNodeCore>> SceneNodeCore::getChildren() {
    std::vector<std::shared_ptr<SceneNodeCore>> children {};
    for(auto& pair: mChildren) {
        children.push_back(pair.second);
    }
    return children;
}
std::vector<std::shared_ptr<const SceneNodeCore>> SceneNodeCore::getChildren() const {
    std::vector<std::shared_ptr<const SceneNodeCore>> children {};
    for(auto& pair: mChildren) {
        children.push_back(pair.second);
    }
    return children;
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::getNode(const std::string& where) {
    if(where=="/") {
        return shared_from_this();
    }

    // descend to the next node in the path
    std::tuple<std::string, std::string> nextPair{ nextInPath(where) };
    const std::string nextNodeName { std::get<0>(nextPair) };
    const std::string remainingWhere { std::get<1>(nextPair) };
    assert(mChildren.find(nextNodeName) != mChildren.end() && "No child node with this name is known");

    return mChildren.at(nextNodeName)->getNode(remainingWhere);
}

void SceneNodeCore::setParentViewport(std::shared_ptr<SceneNodeCore> node, std::shared_ptr<ViewportNode> newViewport)  {
    assert(node && "Cannot modify the parent viewport of a nonexistent node");

    if(auto nodeAsViewport = std::dynamic_pointer_cast<ViewportNode>(node)) {
        if(auto previousParentViewport = node->mParentViewport.lock()) {
            previousParentViewport->mChildViewports.erase(nodeAsViewport);
        }
        if(newViewport) {
            newViewport->mChildViewports.insert(nodeAsViewport);
        }
    }

    node->mParentViewport = newViewport;

    for(auto& child: node->getChildren()) {
        setParentViewport(child, node->getLocalViewport());
    }
}

std::shared_ptr<ViewportNode> SceneNodeCore::getLocalViewport() {
    return mParentViewport.lock();
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::getParentNode() {
    // TODO: Find a more efficient way to prevent access to the scene root
    // Guard against indirect access to scene root owned by the scene system via
    // its descendants.
    std::shared_ptr<SceneNodeCore> parent { mParent };
    if(parent) assert(parent->getName() != kSceneRootName && "Cannot retrieve reference to root node of the scene");
    return parent;
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::disconnectNode(std::shared_ptr<SceneNodeCore> node) {
    // let system know that this node is being disconnected, in case it was part
    // of the scene tree
    node->mEntity->getWorld().getSystem<SceneSystem>()->nodeRemoved(node);

    //disconnect this node from its parent
    std::shared_ptr<SceneNodeCore> parent { node->mParent };
    if(parent) {
        parent->mChildren.erase(node->mName);
    }

    setParentViewport(node, nullptr);
    node->mParent.reset();
    return node;
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::removeNode(const std::string& where) {
    if(where == "/") {
        assert(mName != kSceneRootName && "Cannot remove the hidden scene root node");
        return disconnectNode(shared_from_this());
    }

    // descend to the next node in the path
    std::tuple<std::string, std::string> nextPair{ nextInPath(where) };
    const std::string nextNodeName { std::get<0>(nextPair) };
    const std::string remainingWhere { std::get<1>(nextPair) };
    assert(mChildren.find(nextNodeName) != mChildren.end() && "No child node with this name is known");

    return mChildren.at(nextNodeName)->removeNode(remainingWhere);
}

std::vector<std::shared_ptr<SceneNodeCore>> SceneNodeCore::removeChildren() {
    std::vector<std::shared_ptr<SceneNodeCore>> children { getChildren() };
    for(auto& child: children) {
        disconnectNode(child);
    }
    return children;
}

std::vector<std::shared_ptr<SceneNodeCore>> SceneNodeCore::getDescendants() {
    std::vector<std::shared_ptr<SceneNodeCore>> descendants {};
    for(auto& pair: mChildren) {
        descendants.push_back(pair.second);
        std::vector<std::shared_ptr<SceneNodeCore>> childDescendants {pair.second->getDescendants()};
        descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
    }
    return descendants;
}

EntityID SceneNodeCore::getEntityID() const {
    return mEntity->getID();
}
WorldID SceneNodeCore::getWorldID() const {
    return mEntity->getWorld().getID();
}
UniversalEntityID SceneNodeCore::getUniversalEntityID() const {
    return { mEntity->getWorld().getID(), mEntity->getID() };
}
ECSWorld& SceneNodeCore::getWorld() const {
    return mEntity->getWorld();
}
void SceneNodeCore::joinWorld(ECSWorld& world) {
    mEntity->joinWorld(world);
}

void SceneNodeCore::validateName(const std::string& nodeName) {
    const bool containsValidCharacters{
        std::all_of(nodeName.begin(), nodeName.end(), 
            [](char c) { 
                return (std::isalnum(c) || c == '_');
            }
        )
    };
    assert(nodeName.size() > 0 && "Scene node must have a name");
    assert(containsValidCharacters && "Scene node name may contain only alphanumeric characters and underscores");
}

std::shared_ptr<ViewportNode> ViewportNode::create(const std::string& name, bool inheritsWorld) {
    std::shared_ptr<ViewportNode> newViewport { BaseSceneNode<ViewportNode>::create(Placement{}, name) };
    if(!inheritsWorld) {
        newViewport->createAndJoinWorld();
    }
    return newViewport;
}

std::shared_ptr<ViewportNode> ViewportNode::create(const Key& key, const std::string& name, bool inheritsWorld) {
    std::shared_ptr<ViewportNode> newViewport { BaseSceneNode<ViewportNode>::create(key, Placement{}, name) };
    if(!inheritsWorld) {
        newViewport->createAndJoinWorld();
    }
    return newViewport;
}
std::shared_ptr<ViewportNode> ViewportNode::create(const nlohmann::json& viewportNodeDescription) {
    std::shared_ptr<ViewportNode> newViewport {BaseSceneNode<ViewportNode>::create(viewportNodeDescription)};
    newViewport->updateComponent<Placement>(Placement {}); // override scene file placement
    assert(viewportNodeDescription.find("inherits_world") != viewportNodeDescription.end() && "Viewport descriptions must contain the \"inherits_world\" boolean attribute");
    if(viewportNodeDescription.at("inherits_world").get<bool>() == false) {
        newViewport->createAndJoinWorld();
    }
    return newViewport;
}
std::shared_ptr<ViewportNode> ViewportNode::copy(const std::shared_ptr<const ViewportNode> viewportNode) {
    std::shared_ptr<ViewportNode> newViewport {BaseSceneNode<ViewportNode>::copy(viewportNode)};
    return newViewport;
}
std::shared_ptr<SceneNodeCore> ViewportNode::clone() const {
    std::shared_ptr<SceneNodeCore> newSceneNode { new ViewportNode{*this}, &SceneNodeCore_del_ };
    std::shared_ptr<ViewportNode> newViewport { std::static_pointer_cast<ViewportNode>(newSceneNode) };
    if(mOwnWorld) {
        newViewport->createAndJoinWorld();
    }
    return newViewport;
}

void ViewportNode::createAndJoinWorld() {
    mOwnWorld = std::unique_ptr<ECSWorld>(new ECSWorld { ECSWorld::getPrototype().instantiate() });
    joinWorld(*mOwnWorld);
    mOwnWorld->initialize();
}
void ViewportNode::onActivated() {
    if(!mOwnWorld) return;
    mOwnWorld->activateSimulation();
}
void ViewportNode::onDeactivated() {
    if(!mOwnWorld) return;
    mOwnWorld->deactivateSimulation();
}

std::shared_ptr<Texture> ViewportNode::fetchRenderResult(float simulationProgress) {
    if(mUpdateMode != UpdateMode::NEVER) {
        ECSWorld& world = getWorld();
        world.preRenderStep(simulationProgress);
        mTextureResult = world.getSystem<RenderSystem>()->execute(simulationProgress);
        world.postRenderStep(simulationProgress);
    }
    if(mUpdateMode == UpdateMode::ONCE) {
        mUpdateMode = UpdateMode::NEVER;
    }
    return mTextureResult;
}

ActionDispatch& ViewportNode::getActionDispatch() {
    return mActionDispatch;
}

std::shared_ptr<ViewportNode> ViewportNode::getLocalViewport() {
    return std::static_pointer_cast<ViewportNode>(shared_from_this());
}


void SceneSystem::simulate(uint32_t simStepMillis, std::vector<std::pair<ActionDefinition, ActionData>> triggeredActions) {
    std::queue<std::shared_ptr<ViewportNode>> viewportsToVisit { {mRootNode} };
    while(std::shared_ptr<ViewportNode> viewport = viewportsToVisit.front()) {
        viewportsToVisit.pop();
        if(!viewport->isActive()) continue;

        if(viewport->mOwnWorld) {
            viewport->mOwnWorld->preSimulationStep(simStepMillis);
        }

        for(auto& childViewport: viewport->mChildViewports) {
            viewportsToVisit.push(childViewport);
        }
    }

    mRootNode->mActionDispatch.dispatchActions(triggeredActions);

    viewportsToVisit.push(mRootNode);
    while(std::shared_ptr<ViewportNode> viewport = viewportsToVisit.front()) {
        viewportsToVisit.pop();
        if(!viewport->isActive()) continue;

        if(viewport->mOwnWorld) {
            viewport->mOwnWorld->simulationStep(simStepMillis);
        }
        for(auto& childViewport: viewport->mChildViewports) {
            viewportsToVisit.push(childViewport);
        }
    }
}
void SceneSystem::postSimulationLoop(float simulationProgress) {
    updateTransforms();
    std::queue<std::shared_ptr<ViewportNode>> viewportsToVisit { {mRootNode} };
    while(std::shared_ptr<ViewportNode> viewport = viewportsToVisit.front()) {
        viewportsToVisit.pop();
        if(!viewport->isActive()) continue;
        
        if(viewport->mOwnWorld) {
            viewport->mOwnWorld->postSimulationLoop(simulationProgress);
        }

        for(auto& childViewport: viewport->mChildViewports) {
            viewportsToVisit.push(childViewport);
        }
    }
}

void SceneSystem::render(float simulationProgress) {
    std::shared_ptr<Texture> result {mRootNode->fetchRenderResult(simulationProgress)};
    mRootNode->mOwnWorld->getSystem<RenderSystem>()->renderToScreen(result);
}

void SceneSystem::onApplicationEnd() {
    mRootNode->removeChildren();
}

bool SceneSystem::inScene(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    return inScene(sceneNode->getUniversalEntityID());
}
bool SceneSystem::inScene(UniversalEntityID UniversalEntityID) const {
    return mEntityToNode.find(UniversalEntityID) != mEntityToNode.end();
}

bool SceneSystem::isActive(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    return isActive(sceneNode->getUniversalEntityID());
}
bool SceneSystem::isActive(UniversalEntityID UniversalEntityID) const {
    return mActiveEntities.find(UniversalEntityID) != mActiveEntities.end();
}

std::shared_ptr<SceneNodeCore> SceneSystem::getNode(const std::string& where) {
    assert(where != "/" && "Cannot retrieve scene system's root node");
    return mRootNode->getNode(where);
}

std::shared_ptr<SceneNodeCore> SceneSystem::removeNode(const std::string& where) {
    assert(where != "/" && "Cannot remove scene system's root node");
    return mRootNode->removeNode(where);
}

ECSWorld& SceneSystem::getRootWorld() const {
    return mRootNode->getWorld();
}

ViewportNode& SceneSystem::getRootViewport() const {
    return *mRootNode;
}

void SceneSystem::addNode(std::shared_ptr<SceneNodeCore> sceneNode, const std::string& where) {
    mRootNode->addNode(sceneNode, where);
}

void SceneSystem::nodeAdded(std::shared_ptr<SceneNodeCore> sceneNode) {
    if(!inScene(sceneNode->mParent.lock())) return;

    // Move this node to the world to which it belongs, where viewport nodes (may) mark 
    // the boundary between worlds
    {
        std::shared_ptr<ViewportNode> viewport { std::dynamic_pointer_cast<ViewportNode>(sceneNode) };
        if(viewport && viewport->mOwnWorld) {
            viewport->joinWorld(*viewport->mOwnWorld);
        } else {
            sceneNode->joinWorld(sceneNode->mParent.lock()->getWorld());
        }
    }
    mEntityToNode[sceneNode->getUniversalEntityID()] = sceneNode;

    // when a node is added to the scene, all its children should
    // be in the scene also, so have them registered. Also move every
    // node to its world, and switch worlds if a viewport
    // node is found
    for(auto& descendant: sceneNode->getDescendants()) {
        {
            std::shared_ptr<ViewportNode> viewport { std::dynamic_pointer_cast<ViewportNode>(descendant) };
            if(viewport && viewport->mOwnWorld) {
                viewport->joinWorld(*viewport->mOwnWorld);
            } else {
                descendant->joinWorld(descendant->mParent.lock()->getWorld());
            }
        }
        mEntityToNode[descendant->getUniversalEntityID()] = descendant;
    }

    // let the scene system enable systems on those nodes that
    // should be enabled
    nodeActivationChanged(sceneNode, sceneNode->mStateFlags&SceneNodeCore::StateFlags::ENABLED);
}

void SceneSystem::nodeRemoved(std::shared_ptr<SceneNodeCore> sceneNode) {
    // forget about keeping this node alive if it used to be 
    // a part of the scene tree
    if(inScene(sceneNode)) {
        // disable the node and its children so that it is no
        // longer seen by any system
        nodeActivationChanged(sceneNode, false);

        // lose all references to the node and its descendants
        for(auto& descendant: sceneNode->getDescendants()) {
            mEntityToNode.erase(descendant->getUniversalEntityID());
        }
        mEntityToNode.erase(sceneNode->getUniversalEntityID());
    }
}

void SceneSystem::nodeActivationChanged(std::shared_ptr<SceneNodeCore> sceneNode, bool state) {
    assert(sceneNode && "Null node reference cannot be enabled");

    // early exit if this node isn't in the scene, or if its parent
    // isn't active, or node is already in requested state
    if(
        !inScene(sceneNode)
        || !isActive(sceneNode->mParent.lock())
        || (isActive(sceneNode) && state) 
        || (!isActive(sceneNode) && !state)
    ) { return; }

    if(state) activateSubtree(sceneNode);
    else deactivateSubtree(sceneNode);
}

void SceneSystem::activateSubtree(std::shared_ptr<SceneNodeCore> rootNode) {
    for(auto& childNode: rootNode->getChildren()) {
        if(childNode->mStateFlags&SceneNodeCore::StateFlags::ENABLED) activateSubtree(childNode);
    }

    rootNode->mStateFlags |= SceneNodeCore::StateFlags::ACTIVE;
    rootNode->mEntity->enableSystems(rootNode->mSystemMask);
    mActiveEntities.insert(rootNode->getUniversalEntityID());
    mComputeTransformQueue.insert(rootNode->getUniversalEntityID());

    rootNode->onActivated();
}

void SceneSystem::deactivateSubtree(std::shared_ptr<SceneNodeCore> rootNode) {
    rootNode->onDeactivated();

    rootNode->mEntity->disableSystems();
    mActiveEntities.erase(rootNode->getUniversalEntityID());
    mComputeTransformQueue.erase(rootNode->getUniversalEntityID());
    rootNode->mStateFlags &= ~SceneNodeCore::StateFlags::ACTIVE;

    for(auto& childNode: rootNode->getChildren()) {
        if(childNode->mStateFlags&SceneNodeCore::StateFlags::ENABLED) deactivateSubtree(childNode);
    }
}

void SceneSystem::updateTransforms() {
    // Prune the transform queue of nodes that will be
    // covered by their ancestor's update
    std::set<std::pair<WorldID, EntityID>> entitiesToIgnore {};
    for(std::pair<WorldID, EntityID> entityWorldPair: mComputeTransformQueue) {
        std::shared_ptr<SceneNodeCore> sceneNode { mEntityToNode.at(entityWorldPair)->mParent };
        while(sceneNode != nullptr) {
            if(mComputeTransformQueue.find(sceneNode->getUniversalEntityID()) != mComputeTransformQueue.end()) {
                entitiesToIgnore.insert(entityWorldPair);
                break;
            }
            sceneNode = sceneNode->mParent.lock();
        }
    }
    for(std::pair<WorldID, EntityID> entityWorldPair: entitiesToIgnore) {
        mComputeTransformQueue.erase(entityWorldPair);
    }

    // Apply transform updates to all subtrees present in the queue
    for(std::pair<WorldID, EntityID> entityWorldPair: mComputeTransformQueue) {
        std::vector<std::shared_ptr<SceneNodeCore>> toVisit { {mEntityToNode.at(entityWorldPair)} };
        while(!toVisit.empty()) {
            std::shared_ptr<SceneNodeCore> currentNode { toVisit.back() };
            toVisit.pop_back();

            glm::mat4 localModelMatrix { getLocalTransform(currentNode).mModelMatrix };
            glm::mat4 worldMatrix { getCachedWorldTransform(currentNode->mParent.lock()).mModelMatrix };
            currentNode->updateComponent<Transform>({worldMatrix * localModelMatrix});

            for(std::shared_ptr<SceneNodeCore> child: currentNode->getChildren()) {
                toVisit.push_back(child);
            }
        }
    }
}

Transform SceneSystem::getLocalTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    constexpr Transform rootTransform {
        glm::mat4{1.f}
    };   
    if(!sceneNode) { return rootTransform; }
    const Placement nodePlacement { sceneNode->getComponent<Placement>() };
    return Transform{ buildModelMatrix (
        nodePlacement.mPosition,
        nodePlacement.mOrientation,
        nodePlacement.mScale
    )};
}

Transform SceneSystem::getCachedWorldTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    constexpr Transform rootTransform { glm::mat4{1.f} };
    if(!sceneNode) { return rootTransform; }
    return sceneNode->getComponent<Transform>();
}

void SceneSystem::markDirty(UniversalEntityID UniversalEntityID) {
    if(!isActive(UniversalEntityID)) return;
    mComputeTransformQueue.insert(UniversalEntityID);
}

void SceneSystem::onWorldEntityUpdate(UniversalEntityID UniversalEntityID) {
    markDirty(UniversalEntityID);
}

void SceneSystem::onApplicationInitialize() {
    mRootNode = ViewportNode::create(SceneNodeCore::Key{}, kSceneRootName, false);

    // Manual setup of root node, since it skips the normal activation procedure
    mRootNode->mStateFlags |= SceneNodeCore::StateFlags::ACTIVE | SceneNodeCore::StateFlags::ENABLED;
    mEntityToNode.insert({mRootNode->getUniversalEntityID(), mRootNode});
    mActiveEntities.insert(mRootNode->getUniversalEntityID());
    mRootNode->updateComponent<Transform>({glm::mat4{1.f}});
    mRootNode->onActivated(); 
}
void SceneSystem::onApplicationStart() {
    mRootNode->mOwnWorld->preSimulationStep(0);
    updateTransforms();
}

void SceneSystem::SceneSubworld::onEntityUpdated(EntityID entityID) {
    if(getComponent<Placement>(entityID, 0.f) == getComponent<Placement>(entityID, 1.f)) return;
    mWorld.getSystem<SceneSystem>()->onWorldEntityUpdate({mWorld.getID(), entityID});
}
