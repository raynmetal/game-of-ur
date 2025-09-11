#ifndef ZOAPPQUERYCLICK_H
#define ZOAPPQUERYCLICK_H

#include "toymaker/camera_system.hpp"
#include "toymaker/sim_system.hpp"
#include "toymaker/input_system/input_system.hpp"

#include "interface_pointer_callback.hpp"

class QueryClick: public ToyMaker::SimObjectAspect<QueryClick>, public IUsePointer {
public:
    inline static std::string getSimObjectAspectTypeName() { return "QueryClick"; }
    std::shared_ptr<BaseSimObjectAspect> clone() const override;
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);

protected:
    bool onPointerMove(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition);
    bool onLeftClick(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition);
    bool onLeftRelease(const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMaker::FixedActionBinding> handlerPointerMove {
        declareFixedActionBinding(
            "UI",
            "PointerMove",
            [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
                return this->onPointerMove(actionData, actionDefinition);
            }
        )
    };
    std::weak_ptr<ToyMaker::FixedActionBinding> handlerLeftClick {
        declareFixedActionBinding(
            "UI",
            "Tap",
            [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
                return this->onLeftClick(actionData, actionDefinition);
            }
        )
    };
    std::weak_ptr<ToyMaker::FixedActionBinding> handlerLeftRelease {
        declareFixedActionBinding(
            "UI",
            "Untap",
            [this](const ToyMaker::ActionData& actionData, const ToyMaker::ActionDefinition& actionDefinition) {
                return this->onLeftRelease(actionData, actionDefinition);
            }
        )
    };

private:
    QueryClick(): SimObjectAspect<QueryClick>{0} {}

    ToyMaker::Ray rayFromClickCoordinates(glm::vec2 clickCoordinates);

    // TODO: We're storing shared pointers to nodes in the scene tree here.  There's a bug in 
    // waiting if we have a reference to a node that was taken off the scene tree between 
    // queries.  I don't really know how to think about the problem just now
    std::vector<std::shared_ptr<ToyMaker::SceneNodeCore>> mPreviousQueryResults {};
};

#endif
