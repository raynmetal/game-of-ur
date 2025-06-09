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
    bool onLeftClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);
    bool onRightClick(const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition);

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerLeftClick {
        declareFixedActionBinding(
            "UI",
            "Tap",
            [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onLeftClick(actionData, actionDefinition);
            }
        )
    };

    std::weak_ptr<ToyMakersEngine::FixedActionBinding> handlerRightClick {
        declareFixedActionBinding(
            "UI",
            "RightTap",
            [this](const ToyMakersEngine::ActionData& actionData, const ToyMakersEngine::ActionDefinition& actionDefinition) {
                return this->onRightClick(actionData, actionDefinition);
            }
        )
    };

private:
    QueryClick(): SimObjectAspect<QueryClick>{0} {}

    ToyMakersEngine::Ray rayFromClickCoordinates(glm::vec2 clickCoordinates);

};

#endif
