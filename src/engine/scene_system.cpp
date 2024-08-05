#include <vector>
#include <set>
#include <stack>
#include <cassert>
#include <memory>

#include "util.hpp"
#include "simple_ecs.hpp"
#include "scene_system.hpp"

void SceneSystem::updateTransforms() {
    if(mComputeTransformQueue.empty()) return;

    std::stack<EntityID> toLook {};
    for(EntityID entity: mComputeTransformQueue){
        toLook.push(entity);
        while(!toLook.empty()) {
            EntityID currentEntity { toLook.top() };
            toLook.pop();

            SceneNode currentSceneNode;

            // If the present entity is a real entity, update its transform
            if(currentEntity != kMaxEntities){
                currentSceneNode = gComponentManager.getComponent<SceneNode>(currentEntity);

                Placement& currentPlacement { gComponentManager.getComponent<Placement>(currentEntity) };
                Transform& currentTransform { gComponentManager.getComponent<Transform>(currentEntity) };
                Transform parentTransform { 
                    currentSceneNode.mParent != kMaxEntities? 
                        gComponentManager.getComponent<Transform>(currentSceneNode.mParent):
                        Transform{ glm::mat4{1.f} }
                };

                currentTransform = {
                    (currentSceneNode.mRelativeTo == RelativeTo::Parent?
                        parentTransform.mModelMatrix:
                        glm::mat4{ 1.f }
                    )
                    * buildModelMatrix(
                        currentPlacement.mPosition,
                        currentPlacement.mOrientation,
                        currentPlacement.mScale
                    )
                };
            
            // Otherwise, we just want to push all the root node's 
            // children onto the stack
            } else {
                currentSceneNode = mRootNode;
            }

            for(EntityID child: currentSceneNode.mChildren) {
                toLook.push(child);
            }
        }
    }
    mComputeTransformQueue.clear();
}

void SceneSystem::rebuildGraph() {
    // clear all entities, which are likely no longer valid 
    mValidatedEntities.clear();
    mValidatedEntities.emplace(kMaxEntities);
    mRootNode.mChildren.clear();
    markDirty(kMaxEntities); // makes it so that all this object's children are updated

    std::set<EntityID> visitedEntities {};
    visitedEntities.emplace(kMaxEntities); // always consider root visited

    // Connect each enabled node to its immediate parent
    for(EntityID currEntity: getEnabledEntities()) {
        assert(!cycleDetected(currEntity) && "A cycle was detected in this graph.");

        // Retrieve references to this node (and its parents') scene
        // nodes
        SceneNode& currSceneNode { gComponentManager.getComponent<SceneNode>(currEntity) };
        SceneNode& parentSceneNode {
            currSceneNode.mParent == kMaxEntities? mRootNode :
                gComponentManager.getComponent<SceneNode>(currSceneNode.mParent)
        };

        // Flush this entity's list of children (as it may contain references to
        // children that were present the last time the graph was built, which 
        // may now be invalid)
        if(visitedEntities.find(currEntity) == visitedEntities.end()) {
            currSceneNode.mChildren.clear();
        }
        // Flush the parent's children as well, if the parent hasn't been visited yet
        if(visitedEntities.find(currSceneNode.mParent) == visitedEntities.end()) {
            parentSceneNode.mChildren.clear();
        }

        // Mark both nodes visited to avoid erasing their child
        // connections later on
        visitedEntities.emplace(currEntity);
        visitedEntities.emplace(currSceneNode.mParent);

        // Connect this node to its parent
        parentSceneNode.mChildren.emplace(currEntity);
    }
}

void SceneSystem::markDirty(EntityID entity) {
    assert((entity == kMaxEntities || isEnabled(entity)) && "This entity's management is currently disabled for this system.");

    EntityID topmostDirtyEntity { entity };
    EntityID currEntity { entity };

    // Find a dirty predecessor if it exists
    while(currEntity != kMaxEntities && mComputeTransformQueue.find(currEntity) == mComputeTransformQueue.end()) {
        SceneNode& currSceneNode { gComponentManager.getComponent<SceneNode>(currEntity) };
        currEntity = currSceneNode.mParent;
    }
    if(mComputeTransformQueue.find(currEntity) != mComputeTransformQueue.end()) {
        topmostDirtyEntity = currEntity;
    }
    mComputeTransformQueue.emplace(topmostDirtyEntity);

    // Nothing more needs to be done if we found a dirty ancestor
    if(topmostDirtyEntity != currEntity) return;

    // Remove children from the dirty queue if they were
    // present there.
    std::stack<EntityID> descendants { {entity} };
    while(!descendants.empty()) {
        currEntity = descendants.top();
        descendants.pop();
        SceneNode& currSceneNode { currEntity != kMaxEntities?
            gComponentManager.getComponent<SceneNode>(currEntity):
            mRootNode
        };
        for(EntityID child: currSceneNode.mChildren) {
            mComputeTransformQueue.erase(child);
            descendants.push(child);
        }
    }

}

bool SceneSystem::cycleDetected(EntityID entityID) {
    std::set<EntityID> visitedEntities {};
    EntityID currentEntity { entityID };

    while(mValidatedEntities.find(currentEntity) == mValidatedEntities.end() && currentEntity != kMaxEntities) {
        assert(getEnabledEntities().find(entityID) != getEnabledEntities().end() && "This entity is not managed by the scene system");

        const SceneNode& currentSceneNode { gComponentManager.getComponent<SceneNode>(entityID) };

        // if this node entity has already been visited, we've found a cycle
        if(visitedEntities.find(currentEntity) != visitedEntities.end()) return true;

        // if this node is its own parent, we have a cycle
        if(currentSceneNode.mParent == currentEntity) return true;

        // Try the next entity
        visitedEntities.emplace(currentEntity);
        currentEntity = currentSceneNode.mParent;
    }

    mValidatedEntities.insert(visitedEntities.begin(), visitedEntities.end());
    return false;
}
