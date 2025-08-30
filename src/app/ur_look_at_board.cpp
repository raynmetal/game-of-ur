#include "ur_look_at_board.hpp"

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrLookAtBoard::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrLookAtBoard> lookAtBoardAspect { std::make_shared<UrLookAtBoard>() };
    lookAtBoardAspect->mOffset = glm::vec3{
        jsonAspectProperties.at("offset")[0].get<float>(),
        jsonAspectProperties.at("offset")[1].get<float>(),
        jsonAspectProperties.at("offset")[2].get<float>(),
    };
    return lookAtBoardAspect;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrLookAtBoard::clone() const {
    std::shared_ptr<UrLookAtBoard> lookAtBoardAspect { std::make_shared<UrLookAtBoard>() };
    lookAtBoardAspect->mOffset = mOffset;
    return lookAtBoardAspect;
}

void UrLookAtBoard::onActivated() {
    std::shared_ptr<ToyMaker::SimObject> board {
        getSimObject().getParentNode()->getByPath<std::shared_ptr<ToyMaker::SimObject>>("/gameboard/")
    };
    const ToyMaker::Placement boardPlacement { board->getComponent<ToyMaker::Placement>() };
    ToyMaker::Placement ownPlacement { getComponent<ToyMaker::Placement>() };
    ownPlacement.mOrientation = glm::quatLookAt(
        glm::normalize(glm::vec3{(boardPlacement.mPosition + glm::vec4{mOffset, 0.f}) - ownPlacement.mPosition}),
        glm::vec3{0.f, 1.f, 0.f}
    );
    updateComponent<ToyMaker::Placement>(ownPlacement);
}
