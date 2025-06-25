#ifndef ZOAPPURSCENEVIEW_H
#define ZOAPPURSCENEVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "game_of_ur_data/model.hpp"

class UrSceneView: public ToyMakersEngine::SimObjectAspect<UrSceneView> {
public:
    UrSceneView(): ToyMakersEngine::SimObjectAspect<UrSceneView>{0} {}

    inline static std::string getSimObjectAspectTypeName() { return "UrSceneView"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
    const GameOfUrModel& getModel() const;

private:
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::string mControllerPath {};

    void onBoardClicked(glm::u8vec2 boardLocation);

public:
    ToyMakersEngine::SignalObserver<glm::u8vec2> mObserveBoardClicked { 
        *this, "BoardClickedObserved",
        [this](glm::u8vec2 boardLocation) { this->onBoardClicked(boardLocation); }
    };
};

#endif
