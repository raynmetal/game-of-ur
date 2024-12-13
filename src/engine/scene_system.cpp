#include <vector>
#include <set>
#include <stack>
#include <cassert>
#include <memory>

#include "util.hpp"
#include "simple_ecs.hpp"
#include "scene_system.hpp"


SceneNode::SceneNode(SceneNode&& sceneObject) {
    mEntity = sceneObject.mEntity;
    mParent = sceneObject.mParent;
    mEnabled = sceneObject.mEnabled;
    mName = sceneObject.mName;
    mChildren = sceneObject.mChildren;
    mSystemMask = sceneObject.mSystemMask;

    sceneObject.mEntity = nullptr;
    sceneObject.mParent = nullptr;
    sceneObject.mEnabled = false;
    sceneObject.mChildren.clear();
    sceneObject.mSystemMask = 0;
    sceneObject.mName = "";
}

SceneNode& SceneNode::operator=(SceneNode&& sceneObject) {
    if(this == &sceneObject) return *this;

    mEntity = std::move(sceneObject.mEntity);
    mParent = std::move(sceneObject.mParent);
    mEnabled = sceneObject.mEnabled;
    mName = std::move(sceneObject.mName);
    mChildren = std::move(sceneObject.mChildren);
    mSystemMask = sceneObject.mSystemMask;

    sceneObject.mEntity = nullptr;
    sceneObject.mParent = nullptr;
    sceneObject.mEnabled = false;
    sceneObject.mChildren.clear();
    sceneObject.mSystemMask = 0;
    sceneObject.mName = "";

    return *this;
}

SceneNode::SceneNode(const SceneNode& other) {
    std::shared_ptr<Entity> newEntity { 
        std::make_shared<Entity>(SimpleECS::createEntity())
    };
    newEntity->copy(*(other.mEntity));

    mEntity = newEntity;
    mEnabled = other.mEnabled;
    mName = other.mName;
    mParent = nullptr;
    mSystemMask = other.mSystemMask;

    for(auto& childPair: other.mChildren) {
        const std::string& childName { childPair.first };
        const std::shared_ptr<const SceneNode> childNode { childPair.second };
        mChildren[childName] = std::shared_ptr<SceneNode>(new SceneNode{*childNode});
        mChildren[childName]->mParent = shared_from_this();
    }
}

void SceneNode::addComponent(const nlohmann::json& jsonComponent) {
    mEntity->addComponent(jsonComponent);
}

SceneNode& SceneNode::operator=(const SceneNode& other) {
    // avoid doing anything if other is just a reference to self
    if(this == &other) return *this;

    // disconnect self from parent, and if we were 
    // part of the scene, make scene system forget about
    // us.
    std::shared_ptr<SceneNode> parent{ mParent };
    removeNode("/");

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

    // copy descendant nodes too, attach them to self
    for(auto& childPair: other.mChildren) {
        const std::string& childName { childPair.first };
        const std::shared_ptr<const SceneNode> childNode { childPair.second };
        mChildren[childName] = std::shared_ptr<SceneNode>(new SceneNode{*childNode});
        mChildren[childName]->mParent = shared_from_this();
    }

    // add self back to our parent node complete with our new subtree
    if(parent) {
        parent->addNode(shared_from_this(), "/");
    }

    return *this;
}

bool SceneNode::detectCycle(std::shared_ptr<SceneNode> node) {
    if(!node) return false;

    std::shared_ptr<SceneNode> slow { node };
    std::shared_ptr<SceneNode> fast { slow->mParent };
    while(fast != nullptr && fast->mParent != nullptr && slow != fast) {
        slow = slow->mParent;
        fast = fast->mParent->mParent;
    }

    if(slow == fast) return true;
    return false;
}

bool SceneNode::inScene() const {
    return SimpleECS::getSystem<SceneSystem>()->inScene(shared_from_this());
}

bool SceneNode::isActive() const {
    return SimpleECS::getSystem<SceneSystem>()->isActive(shared_from_this());
}

bool SceneNode::isAncestorOf(std::shared_ptr<const SceneNode> sceneNode) const {
    if(!sceneNode || sceneNode.get() == this) return false;

    std::shared_ptr<const SceneNode> currentNode { sceneNode };
    while(currentNode != nullptr && currentNode.get() != this) {
        currentNode = currentNode->mParent;
    }
    return static_cast<bool>(currentNode);
}

const std::string SceneNode::getName() const {
    return mName;
}

std::shared_ptr<SceneNode> SceneNode::copy(const std::shared_ptr<const SceneNode> sceneNode) {
    if(!sceneNode) return nullptr;
    return std::shared_ptr<SceneNode>(new SceneNode{*sceneNode});
}

std::tuple<std::string, std::string> SceneNode::nextInPath(const std::string& where) {
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

void SceneNode::addNode(std::shared_ptr<SceneNode> node, const std::string& where) {
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

std::vector<std::shared_ptr<SceneNode>> SceneNode::getChildren() {
    std::vector<std::shared_ptr<SceneNode>> children {};
    for(auto& pair: mChildren) {
        children.push_back(pair.second);
    }
    return children;
}

std::shared_ptr<SceneNode> SceneNode::getNode(const std::string& where) {
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

std::shared_ptr<SceneNode> SceneNode::getParentNode() {
    return mParent;
}

std::shared_ptr<SceneNode> SceneNode::removeNode(const std::string& where) {
    if(where == "/") {
        // let scene system know that this node has been removed
        std::shared_ptr<SceneNode> removedNode{shared_from_this()};
        SimpleECS::getSystem<SceneSystem>()->nodeRemoved(removedNode);

        //disconnect this node from its parent
        std::shared_ptr<SceneNode> parent { removedNode->mParent };
        if(parent) {
            parent->mChildren.erase(removedNode->mName);
        }
        removedNode->mParent = nullptr;

        return removedNode;
    }

    // descend to the next node in the path
    std::tuple<std::string, std::string> nextPair{ nextInPath(where) };
    const std::string nextNodeName { std::get<0>(nextPair) };
    const std::string remainingWhere { std::get<1>(nextPair) };
    assert(mChildren.find(nextNodeName) != mChildren.end() && "No child node with this name is known");

    return mChildren.at(nextNodeName)->removeNode(remainingWhere);
}

std::vector<std::shared_ptr<SceneNode>> SceneNode::getDescendants() {
    std::vector<std::shared_ptr<SceneNode>> descendants {};
    for(auto& pair: mChildren) {
        descendants.push_back(pair.second);
        std::vector<std::shared_ptr<SceneNode>> childDescendants {pair.second->getDescendants()};
        descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
    }
    return descendants;
}

EntityID SceneNode::getEntityID() const {
    if(mEntity) {
        return mEntity->getID();
    } return SpecialEntity::ENTITY_ROOT;
}

bool SceneSystem::inScene(std::shared_ptr<const SceneNode> sceneNode) const {
    return mEntityToNode.find(sceneNode->getEntityID()) != mEntityToNode.end();
}

bool SceneSystem::isActive(std::shared_ptr<const SceneNode> sceneNode) const {
    return isActive(sceneNode->getEntityID());
}
bool SceneSystem::isActive(EntityID entityID) const {
    return mActiveEntities.find(entityID) != mActiveEntities.end();
}

std::shared_ptr<SceneNode> SceneSystem::getNode(const std::string& where) {
    assert(where != "/" && "Cannot retrieve scene system's root node");
    return mRootNode->getNode(where);
}

std::shared_ptr<SceneNode> SceneSystem::removeNode(const std::string& where) {
    assert(where != "/" && "Cannot remove scene system's root node");
    return mRootNode->removeNode(where);
}

void SceneSystem::addNode(std::shared_ptr<SceneNode> sceneNode, const std::string& where) {
    mRootNode->addNode(sceneNode, where);
}

void SceneSystem::nodeAdded(std::shared_ptr<SceneNode> sceneNode) {
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

void SceneSystem::nodeRemoved(std::shared_ptr<SceneNode> sceneNode) {
    // forget about keeping this node alive if it used to be 
    // a part of the scene tree
    if(inScene(sceneNode)) {
        // disable the node and its children so that it is no
        // longer seen by any system
        nodeActivationChanged(sceneNode, false);

        // lose all references to the node
        for(auto& descendant: sceneNode->getDescendants()) {
            mEntityToNode.erase(descendant->getEntityID());
        }
        mEntityToNode.erase(sceneNode->getEntityID());
    }
}

void SceneSystem::nodeActivationChanged(std::shared_ptr<SceneNode> sceneNode, bool state) {
    assert(sceneNode && "Null node reference cannot be enabled");

    // early exit if this node isn't in the scene
    if(!sceneNode->inScene()) { return; }

    if(state==true && isActive(sceneNode->mParent)) {
        sceneNode->mEntity->enableSystems(sceneNode->mSystemMask);
        mActiveEntities.emplace(sceneNode->getEntityID());
        mComputeTransformQueue.emplace(sceneNode->getEntityID());
    }
    else {
        sceneNode->mEntity->disableSystems();
        mActiveEntities.erase(sceneNode->getEntityID());
        mComputeTransformQueue.erase(sceneNode->getEntityID());
    }
    std::vector<std::shared_ptr<SceneNode>> toVisit { sceneNode->getChildren() };

    // propagate the change in this node's state down to its descendants
    while(!toVisit.empty()) {
        std::shared_ptr<SceneNode> currentNode { toVisit.back() };
        toVisit.pop_back();

        // if this node is marked enabled, and an unbroken chain of
        // alive nodes exists to the scene root, enable this node
        if(currentNode->mEnabled && currentNode->mParent->isActive()) {
            currentNode->mEntity->enableSystems(currentNode->mSystemMask);
            mActiveEntities.emplace(currentNode->getEntityID());
            mComputeTransformQueue.emplace(currentNode->getEntityID());
        // not enabled or parent is dead, disable this node
        } else {
            currentNode->mEntity->disableSystems();
            mActiveEntities.erase(currentNode->getEntityID());
            mComputeTransformQueue.erase(currentNode->getEntityID());
        }
        for(auto& node: currentNode->getChildren()) {
            toVisit.push_back(node);
        }
    }
}

void SceneSystem::updateTransforms() {
    // Prune the transform queue of nodes that will be
    // covered by their ancestor's update
    std::set<EntityID> entitiesToIgnore {};
    for(EntityID entity: mComputeTransformQueue) {
        std::shared_ptr<SceneNode> sceneNode { mEntityToNode.at(entity)->mParent };
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
        std::vector<std::shared_ptr<SceneNode>> toVisit { {mEntityToNode.at(entityID)} };
        while(!toVisit.empty()) {
            std::shared_ptr<SceneNode> currentNode { toVisit.back() };
            toVisit.pop_back();

            glm::mat4 localModelMatrix { getLocalTransform(currentNode).mModelMatrix };
            glm::mat4 worldMatrix { getCachedWorldTransform(currentNode->mParent).mModelMatrix };
            updateComponent<Transform>(currentNode->getEntityID(), {worldMatrix * localModelMatrix});

            for(std::shared_ptr<SceneNode> child: currentNode->getChildren()) {
                toVisit.push_back(child);
            }
        }
    }
}

Transform SceneSystem::getLocalTransform(std::shared_ptr<const SceneNode> sceneNode) const {
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

Transform SceneSystem::getCachedWorldTransform(std::shared_ptr<const SceneNode> sceneNode) const {
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
