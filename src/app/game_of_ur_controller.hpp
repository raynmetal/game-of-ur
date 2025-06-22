#ifndef ZOAPPGAMEOFURCONTROLLER_H
#define ZOAPPGAMEOFURCONTROLLER_H

#include "../engine/sim_system.hpp"

#include "game_of_ur_data/model.hpp"

class GameOfUrController: public ToyMakersEngine::SimObjectAspect<GameOfUrController> {
public:
    GameOfUrController(): SimObjectAspect<GameOfUrController>{0} {}
    inline static std::string getSimObjectAspectTypeName() { return "GameOfUrController"; }
    static std::shared_ptr<BaseSimObjectAspect> create(const nlohmann::json& jsonAspectProperties);
    std::shared_ptr<BaseSimObjectAspect> clone() const override;

    ToyMakersEngine::SignalObserver<glm::u8vec2> mObserveBoardClicked { *this, "BoardClickedObserved", [this](glm::u8vec2 boardLocation) {
        std::cout << "Game of Ur Controller: Board location clicked: \n";
        std::cout << "\trow: " << static_cast<int>(boardLocation.x) << "\n";
        std::cout << "\tcol: " << static_cast<int>(boardLocation.y) << "\n";
    }};

private:
    GameOfUrModel mModel {};
};

#endif
