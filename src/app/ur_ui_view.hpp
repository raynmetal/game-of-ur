#ifndef ZOAPPURUIVIEW_H
#define ZOAPPURUIVIEW_H

#include "../engine/sim_system.hpp"
#include "../engine/signals.hpp"
#include "../engine/text_render.hpp"

#include "game_of_ur_data/model.hpp"

class UrUIView: public ToyMakersEngine::SimObjectAspect<UrUIView> {
public:
    UrUIView(): ToyMakersEngine::SimObjectAspect<UrUIView>{0} {}

    inline static std::string getSimObjectAspectTypeName() { return "UrUIView"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    void onActivated() override;
    const GameOfUrModel& getModel() const;

private:
    std::weak_ptr<ToyMakersEngine::SimObject> mGameOfUrController {};
    std::string mControllerPath {};

public:
};

#endif
