#include "ur_controller.hpp"

#include "ur_scene_view.hpp"

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneView::clone() const {
    std::shared_ptr<UrSceneView> uiView{ std::make_shared<UrSceneView>() };
    uiView->mControllerPath = mControllerPath;
    return uiView;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneView::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSceneView> uiView { std::make_shared<UrSceneView>() };
    uiView->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    return uiView;
}

void UrSceneView::onActivated() {
    mGameOfUrController = (
        ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>(mControllerPath)
    );
}

const GameOfUrModel& UrSceneView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}

void UrSceneView::onBoardClicked(glm::u8vec2 boardLocation) {
    std::cout << "UrSceneView: Board location clicked: \n";
    const GameOfUrModel& model { getModel() };
    const HouseData houseData { model.getHouseData(boardLocation) };
    std::cout << "\tregion: ";
    switch(houseData.mRegion) {
        case House::Region::BATTLEFIELD: std::cout << "battlefield\n"; break;
        case House::Region::ONE: std::cout << "player one\n"; break;
        case House::Region::TWO: std::cout << "player two\n"; break;
    }
    std::cout << "\tnext cell direction: " << glm::to_string(houseData.mNextCellDirection) << "\n";
    std::cout << "\tlocation: " << glm::to_string(houseData.mLocation) << "\n";
    std::cout << "\toccupant: ";
    if(houseData.mOccupant.mOwner == RoleID::NA) { std::cout << "NA\n"; }
    else {
        std::cout << ((houseData.mOccupant.mOwner==RoleID::ONE)? "p1 ": "p2 ");
        switch(houseData.mOccupant.mType) {
            case PieceTypeID::SWALLOW: std::cout << "swallow\n"; break;
            case PieceTypeID::STORMBIRD: std::cout << "storm bird\n"; break;
            case PieceTypeID::RAVEN: std::cout << "raven\n"; break;
            case PieceTypeID::ROOSTER: std::cout << "rooster\n"; break;
            case PieceTypeID::EAGLE: std::cout << "eagle\n"; break;
            default: assert(false && "We should never get here"); break;
        }
    }
    std::cout << "\ttype: " << ((houseData.mType == House::Type::REGULAR)? "regular": "rosette") << "\n";   
}


void UrSceneView::onLaunchPieceInitiated(PieceTypeID piece) {
    std::cout << "UrSceneView: Launch piece initiated\n";
}
void UrSceneView::onLaunchPieceCanceled() {
    std::cout << "UrSceneView: Launch piece canceled\n";
}
void UrSceneView::onMoveMade(const MoveResultData& moveResultData) {
    std::cout << "UrSceneView: move made\n";
}

