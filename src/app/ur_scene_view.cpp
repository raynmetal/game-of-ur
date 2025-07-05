#include "game_of_ur_data/serialize.hpp"

#include "../engine/core/resource_database.hpp"

#include "ur_controller.hpp"
#include "ur_scene_view.hpp"


std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneView::clone() const {
    std::shared_ptr<UrSceneView> sceneView{ std::make_shared<UrSceneView>() };
    sceneView->mControllerPath = mControllerPath;
    sceneView->mPieceModelMap = mPieceModelMap;
    return sceneView;
}

std::shared_ptr<ToyMakersEngine::BaseSimObjectAspect> UrSceneView::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSceneView> sceneView { std::make_shared<UrSceneView>() };
    sceneView->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    for(const auto& pieceDescription: jsonAspectProperties.at("piece_to_model")) {
        const PieceIdentity pieceIdentity { pieceDescription.at("piece").get<PieceIdentity>() };
        sceneView->mPieceModelMap.insert_or_assign(pieceIdentity, pieceDescription.at("model").get<std::string>());
    }
    for(uint8_t role { RoleID::ONE }; role <= RoleID::TWO; ++role) {
        for(uint8_t piece { PieceTypeID::SWALLOW }; piece < PieceTypeID::TOTAL; ++piece) {
            const PieceIdentity pieceIdentity { .mType=static_cast<PieceTypeID>(piece), .mOwner=static_cast<RoleID>(role) };
            assert(
                sceneView->mPieceModelMap.find(pieceIdentity) != sceneView->mPieceModelMap.end()
                    && "A certain piece type does not have a corresponding model assigned."
            );
            assert(
                ToyMakersEngine::ResourceDatabase::HasResourceDescription(
                    sceneView->mPieceModelMap.at(pieceIdentity)
                ) && "There is no static model corresponding to the resource name specified in the piece-model map"
            );
        }
    }
    return sceneView;
}

void UrSceneView::onActivated() {
    mGameOfUrController = (
        ToyMakersEngine::ECSWorld::getSingletonSystem<ToyMakersEngine::SceneSystem>()
        ->getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>(mControllerPath)
    );
    mGameOfUrBoard = getSimObject().getByPath<std::shared_ptr<ToyMakersEngine::SimObject>>("/viewport_3D/gameboard/");
}

const GameOfUrModel& UrSceneView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}

const BoardLocations& UrSceneView::getBoard() const {
    return mGameOfUrBoard.lock()->getAspect<BoardLocations>();
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

    if(getModel().getCurrentPhase().mGamePhase != GamePhase::PLAY) {
        return;
    }

    if(mMode == Mode::LAUNCH_POSITION_SELECTION) {
        mMode = Mode::GENERAL;
        std::cout << "UrSceneView: Attempting to launch swallow to " << glm::to_string(boardLocation) << "\n";
        mSigLaunchPieceAttempted.emit(
            PieceTypeID::SWALLOW,
            boardLocation
        );
        return;
    }

    const PieceIdentity& selectedPiece { houseData.mOccupant };
    if(houseData.mOccupant.mOwner != RoleID::NA) {
        mSigMovePieceAttempted.emit(selectedPiece);
    }
}

void UrSceneView::onLaunchPieceInitiated(PieceTypeID pieceType) {
    std::cout << "UrSceneView: Launch piece initiated\n";
    if(getModel().getCurrentPhase().mGamePhase != GamePhase::PLAY) return;
    const PieceIdentity pieceIdentity {
        .mType { pieceType },
        .mOwner{ getModel().getCurrentPlayer().mRole },
    };

    const std::vector<glm::u8vec2> launchPositions { getModel().getLaunchPositions(pieceIdentity) };
    assert(launchPositions.size() && "Every piece must have at least one launch position associated with it");
    if(launchPositions.size() > 1) {
        std::cout << "UrSceneView: Switched to launch position selection mode\n";
        mMode = Mode::LAUNCH_POSITION_SELECTION;
        return;
    }

    mMode = Mode::GENERAL;
    const glm::u8vec2 launchLocation { launchPositions[0] };
    mSigLaunchPieceAttempted.emit(
        pieceType,
        launchLocation
    );
}

void UrSceneView::onLaunchPieceCanceled() {
    std::cout << "UrSceneView: Launch piece canceled\n";
    mMode = Mode::GENERAL;
}

void UrSceneView::onMoveMade(const MoveResultData& moveResultData) {
    std::cout << "UrSceneView: move made\n";
    mMode = Mode::GENERAL;

    const PieceIdentity& displacedPieceIdentity { moveResultData.mDisplacedPiece.mIdentity };
    const PieceIdentity& movedPieceIdentity { moveResultData.mMovedPiece.mIdentity };

    if(displacedPieceIdentity.mOwner != RoleID::NA) {
        mPieceNodeMap.at(displacedPieceIdentity)->removeNode("/");
        mPieceNodeMap.erase(displacedPieceIdentity);
    }

    if(moveResultData.mFlags & MoveResultData::COMPLETES_ROUTE) {
        mPieceNodeMap.at(movedPieceIdentity)->removeNode("/");
        mPieceNodeMap.erase(movedPieceIdentity);
        return;
    }

    if(!mPieceNodeMap[movedPieceIdentity]) {
        std::shared_ptr<ToyMakersEngine::StaticModel> model {
            ToyMakersEngine::ResourceDatabase::GetRegisteredResource<ToyMakersEngine::StaticModel>(
                mPieceModelMap.at(movedPieceIdentity)
            )
        };
        mPieceNodeMap[movedPieceIdentity] = ToyMakersEngine::SceneNode::create(
            ToyMakersEngine::Placement{},
            mPieceModelMap.at(movedPieceIdentity),
            model
        );
        getSimObject().addNode(
            mPieceNodeMap[movedPieceIdentity],
            "/viewport_3D/"
        );
    }

    const ToyMakersEngine::Placement piecePlacement {
        .mPosition { getBoard().gridIndicesToBoardPoint(moveResultData.mMovedPiece.mLocation) }
    };
    mPieceNodeMap[movedPieceIdentity]->updateComponent<ToyMakersEngine::Placement>(piecePlacement);
}
