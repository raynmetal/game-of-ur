#include <iostream>

#include "game_of_ur_controller.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> GameOfUrController::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<GameOfUrController> controller { std::make_shared<GameOfUrController>() };
    return controller;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> GameOfUrController::clone() const {
    return std::make_shared<GameOfUrController>();
}
