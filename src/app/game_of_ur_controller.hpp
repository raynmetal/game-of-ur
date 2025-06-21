#ifndef ZOAPPGAMEOFURCONTROLLER_H
#define ZOAPPGAMEOFURCONTROLLER_H

#include "../engine/sim_system.hpp"

#include "game_of_ur_model/game_of_ur_model.hpp"

class GameOfUrController: public ToyMakersEngine::SimObjectAspect<GameOfUrController> {
public:
    GameOfUrController(): SimObjectAspect<GameOfUrController>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "GameOfUrController"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

private:
    GameOfUrModel mModel {};
};

#endif
