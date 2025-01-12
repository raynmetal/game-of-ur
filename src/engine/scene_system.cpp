#include <vector>
#include <set>
#include <cassert>
#include <memory>

#include "util.hpp"
#include "simple_ecs.hpp"
#include "scene_system.hpp"

SceneNodeCore::~SceneNodeCore() {
    onDestroyed();
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
    std::shared_ptr<SceneNodeCore> newSceneNode{ new SceneNodeCore{*this} };
    newSceneNode->onCreated();
    return newSceneNode;
}

SceneNodeCore::SceneNodeCore(const nlohmann::json& sceneNodeDescription)
{
    validateName(sceneNodeDescription.at("name").get<std::string>());
    mName = sceneNodeDescription.at("name").get<std::string>();
    mEntity = std::make_shared<Entity>(
        SimpleECS::createEntity()
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
        std::make_shared<Entity>(SimpleECS::createEntity())
    };
    newEntity->copy(*(other.mEntity));
    mEntity = newEntity;
    mEnabled = other.mEnabled;
    mChildren.clear();
    mName = other.mName;
    mParent = nullptr;
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
    std::shared_ptr<SceneNodeCore> fast { slow->mParent };
    while(fast != nullptr && fast->mParent != nullptr && slow != fast) {
        slow = slow->mParent;
        fast = fast->mParent->mParent;
    }

    if(slow == fast) return true;
    return false;
}

bool SceneNodeCore::inScene() const {
    return SimpleECS::getSystem<SceneSystem>()->inScene(shared_from_this());
}

bool SceneNodeCore::isActive() const {
    return SimpleECS::getSystem<SceneSystem>()->isActive(shared_from_this());
}

bool SceneNodeCore::isAncestorOf(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    if(!sceneNode || sceneNode.get() == this) return false;

    std::shared_ptr<const SceneNodeCore> currentNode { sceneNode };
    while(currentNode != nullptr && currentNode.get() != this) {
        currentNode = currentNode->mParent;
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
    assert(node->mParent == nullptr && "Node must not have a parent");
    if(where == "/") {
        assert(mChildren.find(node->mName) == mChildren.end() && "A node with this name already exists at this location");
        mChildren[node->mName] = node;
        node->mParent = shared_from_this();
        assert(!detectCycle(node) && "Cycle detected, ancestor node added as child to its descendant.");
        SimpleECS::getSystem<SceneSystem>()->nodeAdded(node);
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

std::shared_ptr<SceneNodeCore> SceneNodeCore::getParentNode() {
    // TODO: Find a more efficient way to prevent access to the scene root
    // Guard against indirect access to scene root owned by the scene system via
    // its descendants.
    if(mParent) assert(mParent->getName() != "" && "Cannot retrieve reference to root node of the scene");
    return mParent;
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::disconnectNode(std::shared_ptr<SceneNodeCore> node) {
    // let system know that this node is being disconnected, in case it was part
    // of the scene tree
    SimpleECS::getSystem<SceneSystem>()->nodeRemoved(node);

    //disconnect this node from its parent
    std::shared_ptr<SceneNodeCore> parent { node->mParent };
    if(parent) {
        parent->mChildren.erase(node->mName);
    }
    node->mParent = nullptr;
    return node;
}

std::shared_ptr<SceneNodeCore> SceneNodeCore::removeNode(const std::string& where) {
    if(where == "/") {
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
    if(mEntity) {
        return mEntity->getID();
    } return SpecialEntity::ENTITY_ROOT;
}

void SceneSystem::ApploopEventHandler::onApplicationEnd() {
    mSystem->mRootNode->removeChildren();
}

bool SceneSystem::inScene(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    return mEntityToNode.find(sceneNode->getEntityID()) != mEntityToNode.end();
}

bool SceneSystem::isActive(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    return isActive(sceneNode->getEntityID());
}
bool SceneSystem::isActive(EntityID entityID) const {
    return mActiveEntities.find(entityID) != mActiveEntities.end();
}

std::shared_ptr<SceneNodeCore> SceneSystem::getNode(const std::string& where) {
    assert(where != "/" && "Cannot retrieve scene system's root node");
    return mRootNode->getNode(where);
}

std::shared_ptr<SceneNodeCore> SceneSystem::removeNode(const std::string& where) {
    assert(where != "/" && "Cannot remove scene system's root node");
    return mRootNode->removeNode(where);
}

void SceneSystem::addNode(std::shared_ptr<SceneNodeCore> sceneNode, const std::string& where) {
    mRootNode->addNode(sceneNode, where);
}

void SceneSystem::nodeAdded(std::shared_ptr<SceneNodeCore> sceneNode) {
    if(!inScene(sceneNode->mParent)) return;

    mEntityToNode[sceneNode->getEntityID()] = sceneNode;
    // when a node is added to the scene, all its children should
    // be in the scene also, so have them registered
    for(auto& descendant: sceneNode->getDescendants()) {
        mEntityToNode[descendant->getEntityID()] = descendant;
    }

    // let the scene system enable systems on those nodes that
    // should be enabled
    nodeActivationChanged(sceneNode, sceneNode->mEnabled);
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
            mEntityToNode.erase(descendant->getEntityID());
        }
        mEntityToNode.erase(sceneNode->getEntityID());
    }
}

void SceneSystem::nodeActivationChanged(std::shared_ptr<SceneNodeCore> sceneNode, bool state) {
    assert(sceneNode && "Null node reference cannot be enabled");

    // early exit if this node isn't in the scene, or if its parent
    // isn't active, or node is already in requested state
    if(
        !sceneNode->inScene() 
        || !isActive(sceneNode->mParent)
        || (isActive(sceneNode) && state) 
        || (!isActive(sceneNode) && !state)
    ) { return; }

    if(state) activateSubtree(sceneNode);
    else deactivateSubtree(sceneNode);
}

void SceneSystem::activateSubtree(std::shared_ptr<SceneNodeCore> rootNode) {
    for(auto& childNode: rootNode->getChildren()) {
        if(childNode->mEnabled) activateSubtree(childNode);
    }

    rootNode->mEntity->enableSystems(rootNode->mSystemMask);
    mActiveEntities.emplace(rootNode->getEntityID());
    mComputeTransformQueue.emplace(rootNode->getEntityID());

    rootNode->onActivated();
}

void SceneSystem::deactivateSubtree(std::shared_ptr<SceneNodeCore> rootNode) {
    rootNode->onDeactivated();

    rootNode->mEntity->disableSystems();
    mActiveEntities.erase(rootNode->getEntityID());
    mComputeTransformQueue.erase(rootNode->getEntityID());

    for(auto& childNode: rootNode->getChildren()) {
        if(childNode->mEnabled) deactivateSubtree(childNode);
    }
}

void SceneSystem::updateTransforms() {
    // Prune the transform queue of nodes that will be
    // covered by their ancestor's update
    std::set<EntityID> entitiesToIgnore {};
    for(EntityID entity: mComputeTransformQueue) {
        std::shared_ptr<SceneNodeCore> sceneNode { mEntityToNode.at(entity)->mParent };
        while(sceneNode != nullptr) {
            if(mComputeTransformQueue.find(sceneNode->getEntityID()) != mComputeTransformQueue.end()) {
                entitiesToIgnore.emplace(entity);
                break;
            }
            sceneNode = sceneNode->mParent;
        }
    }
    for(EntityID entity: entitiesToIgnore) {
        mComputeTransformQueue.erase(entity);
    }

    // Apply transform updates to all subtrees present in the queue
    for(EntityID entityID: mComputeTransformQueue) {
        std::vector<std::shared_ptr<SceneNodeCore>> toVisit { {mEntityToNode.at(entityID)} };
        while(!toVisit.empty()) {
            std::shared_ptr<SceneNodeCore> currentNode { toVisit.back() };
            toVisit.pop_back();

            glm::mat4 localModelMatrix { getLocalTransform(currentNode).mModelMatrix };
            glm::mat4 worldMatrix { getCachedWorldTransform(currentNode->mParent).mModelMatrix };
            updateComponent<Transform>(currentNode->getEntityID(), {worldMatrix * localModelMatrix});

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
    if(
        !sceneNode
        || sceneNode->getEntityID() == SpecialEntity::ENTITY_ROOT
    ) { return rootTransform; }
    Placement nodePlacement { sceneNode->getComponent<Placement>() };
    return Transform{ buildModelMatrix (
        nodePlacement.mPosition,
        nodePlacement.mOrientation,
        nodePlacement.mScale
    )};
}

Transform SceneSystem::getCachedWorldTransform(std::shared_ptr<const SceneNodeCore> sceneNode) const {
    constexpr Transform rootTransform {
        glm::mat4{1.f}
    };
    if(
        !sceneNode 
        || sceneNode->getEntityID() == SpecialEntity::ENTITY_ROOT
    ) { return rootTransform; }
    return sceneNode->getComponent<Transform>();
}

void SceneSystem::markDirty(EntityID entityID) {
    if(!isActive(entityID)) return;
    mComputeTransformQueue.emplace(entityID);
}

void SceneSystem::onEntityUpdated(EntityID entityID) {
    markDirty(entityID);
}

void SceneSystem::ApploopEventHandler::onPreRenderStep(float simulationProgress) {
    mSystem->updateTransforms();
}

void SceneSystem::ApploopEventHandler::onApplicationStart() {
    mSystem->updateTransforms();
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
