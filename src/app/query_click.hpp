#ifndef ZOAPPQUERYCLICK_H
#define ZOAPPQUERYCLICK_H

#include "../engine/camera_system.hpp"
#include "../engine/sim_system.hpp"
#include "../engine/input_system/input_system.hpp"

#include "interface_pointer_callback.hpp"

class QueryClick: public ToyMakersEngine::SimObjectAspect<QueryClick>, public IUsePointer {
public:
    inline static std::string getSimObjectAspectTypeName() { return "QueryClick"; }
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

protected:
    bool onPointerMove(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);
    bool onLeftClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);
    bool onLeftRelease(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerPointerMove {
        declareFixedActionBinding(
            "UI",
            "PointerMove",
            [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onPointerMove(actionData, actionDefinition);
            }
        )
    };
    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerLeftClick {
        declareFixedActionBinding(
            "UI",
            "Tap",
            [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onLeftClick(actionData, actionDefinition);
            }
        )
    };
    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerLeftRelease {
        declareFixedActionBinding(
            "UI",
            "Untap",
            [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onLeftRelease(actionData, actionDefinition);
            }
        )
    };

private:
    QueryClick(): SimObjectAspect<QueryClick>{0} {}

    ToyMakersEngine::Ray rayFromClickCoordinates(glm::vec2 clickCoordinates);

    // TODO: We're storing shared pointers to nodes in the scene tree here.  There's a bug in 
    // waiting if we have a reference to a node that was taken off the scene tree between 
    // queries.  I don't really know how to think about the problem just now
    std::vector<std::shared_ptr<ToyMakersEngine::SceneNodeCore>> mPreviousQueryResults {};
};

#endif
