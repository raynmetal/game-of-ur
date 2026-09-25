#include <iostream>

#include <toymaker/engine/core/resource_database.hpp>

#include "ur_sounds.hpp"
#include "game_of_ur_data/serialize.hpp"
#include "ur_controller.hpp"
#include "ur_scene_view.hpp"

const auto kWhiteCone { ToyMaker::LightEmissionData::MakeSpotLight(
    2, // inner angle
    10, // outer angle
    glm::vec3 { 1.5f, 1.5f, 1.5f }, // diffuse
    glm::vec3 { 0.f }, // specular
    glm::vec3 { 0.f }, // ambient
    0.02, // linear decay
    0.004  // quadratic decay
) };

const auto kYellowCone { ToyMaker::LightEmissionData::MakeSpotLight(
    2, // inner angle
    10, // outer angle
    glm::vec3 { 1.5f, 1.5f, 0.f }, // diffuse
    glm::vec3 { 0.f }, // specular
    glm::vec3 { 0.f }, // ambient
    0.02, // linear decay
    0.004  // quadratic decay
) };

const auto kGreenCone { ToyMaker::LightEmissionData::MakeSpotLight(
    2, // inner angle
    10, // outer angle
    glm::vec3 { 0.f, 1.5f, 0.f }, // diffuse
    glm::vec3 { 0.f }, // specular
    glm::vec3 { 0.f }, // ambient
    0.02, // linear decay
    0.004  // quadratic decay
) };

bool operator<(const UrPieceAnimationKey& one, const UrPieceAnimationKey& two) {
    return (
        one.mTime > two.mTime
        || (
            one.mTime == two.mTime
            && one.mPieceIdentity < two.mPieceIdentity
        )
    );
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSceneView::clone() const {
    std::shared_ptr<UrSceneView> sceneView{ std::make_shared<UrSceneView>() };
    sceneView->mControllerPath = mControllerPath;
    sceneView->mPieceModelMap = mPieceModelMap;
    return sceneView;
}

std::shared_ptr<ToyMaker::BaseSimObjectAspect> UrSceneView::create(const nlohmann::json& jsonAspectProperties) {
    std::shared_ptr<UrSceneView> sceneView { std::make_shared<UrSceneView>() };
    sceneView->mControllerPath = jsonAspectProperties.at("controller_path").get<std::string>();
    for(const auto& pieceDescription: jsonAspectProperties.at("piece_to_model")) {
        const PieceIdentity pieceIdentity { pieceDescription.at("piece").get<PieceIdentity>() };
        sceneView->mPieceModelMap.insert_or_assign(pieceIdentity, pieceDescription.at("model").get<std::string>());
    }
    for(uint8_t role { RoleID::BLACK }; role <= RoleID::WHITE; ++role) {
        for(uint8_t piece { PieceTypeID::SWALLOW }; piece < PieceTypeID::TOTAL; ++piece) {
            const PieceIdentity pieceIdentity { .mType=static_cast<PieceTypeID>(piece), .mOwner=static_cast<RoleID>(role) };
            assert(
                sceneView->mPieceModelMap.find(pieceIdentity) != sceneView->mPieceModelMap.end()
                    && "A certain piece type does not have a corresponding model assigned."
            );
            assert(
                ToyMaker::ResourceDatabase::HasResourceDescription(
                    sceneView->mPieceModelMap.at(pieceIdentity)
                ) && "There is no static model corresponding to the resource name specified in the piece-model map"
            );
        }
    }
    return sceneView;
}

void UrSceneView::onActivated() {
    mGameOfUrController = (
        ToyMaker::ECSWorld::getSingletonSystem<ToyMaker::SceneSystem>()
        ->getByPath<std::shared_ptr<ToyMaker::SimObject>>(mControllerPath)
    );
    mGameOfUrBoard = getSimObject().getByPath<std::shared_ptr<ToyMaker::SimObject>>("/viewport_3D/gameboard/");
    clearLights();
    mFocusedGridCell = kGridUnfocused;
}

const GameOfUrModel& UrSceneView::getModel() const {
    return mGameOfUrController.lock()->getAspect<UrController>().getModel();
}

const BoardLocations& UrSceneView::getBoard() const {
    return mGameOfUrBoard.lock()->getAspect<BoardLocations>();
}

void UrSceneView::clearLights() {
    if(!getSimObject().hasNode("/viewport_3D/board_lights/")) {
        return;
    }

    auto boardLights {
        getSimObject().getNode("/viewport_3D/board_lights/")
    };
    boardLights->removeChildren();
}

void UrSceneView::addLight(glm::u8vec2 gridPosition, const ToyMaker::LightEmissionData& light) {
    if(!getSimObject().hasNode("/viewport_3D/board_lights/")) {
        auto boardLights {
            ToyMaker::SceneNode::create(
                ToyMaker::Placement {},
                "board_lights"
            )
        };
        getSimObject().addNode(
            boardLights,
            "/viewport_3D/"
        );
    }
    auto boardLights {
        getSimObject().getNode("/viewport_3D/board_lights/")
    };
    const glm::vec3 boardPosition { getBoard().gridIndicesToBoardPoint(gridPosition) };
    const auto lightNode {
        ToyMaker::SceneNode::create(
            ToyMaker::Placement {
                .mPosition { boardPosition + glm::vec3 { 0.f, 4.f, 0.f }, 1.f },
                .mOrientation { 0.7071068, -0.7071068, 0, 0 }
            },
            "light__" + std::to_string(static_cast<int>(gridPosition.x)) + "_" + std::to_string(static_cast<int>(gridPosition.y)),
            light
        )
    };
    boardLights->addNode(lightNode, "/");
}

void UrSceneView::addLights(const std::vector<glm::u8vec2>& boardPositions) {
    clearLights();
    for(const auto& position: boardPositions) {
        addLight(position, kWhiteCone);
    }
}

void UrSceneView::onBoardClicked(glm::u8vec2 boardLocation) {
    std::cout << "UrSceneView: Board location clicked: \n";
    if(mMode == Mode::TRANSITION) return;
    clearLights();
    mFocusedGridCell = kGridUnfocused;

    const GameOfUrModel& model { getModel() };
    const HouseData houseData { model.getHouseData(boardLocation) };
    std::cout << "\tregion: ";
    switch(houseData.mRegion) {
        case House::Region::BATTLEFIELD: std::cout << "battlefield\n"; break;
        case House::Region::BLACK: std::cout << "player one\n"; break;
        case House::Region::WHITE: std::cout << "player two\n"; break;
    }
    std::cout << "\tnext cell direction: " << glm::to_string(houseData.mNextCellDirection) << "\n";
    std::cout << "\tlocation: " << glm::to_string(houseData.mLocation) << "\n";
    std::cout << "\toccupant: ";
    if(houseData.mOccupant.mOwner == RoleID::NA) { std::cout << "NA\n"; }
    else {
        std::cout << ((houseData.mOccupant.mOwner==RoleID::BLACK)? "p1 ": "p2 ");
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

void UrSceneView::onBoardHovered(glm::u8vec2 boardLocation) {
    std::cout << "UrSceneView: Board location hovered: " << glm::to_string(boardLocation) << "\n";

    // guard: we're not in a phase where highlights are required, or this isn't a new
    // board grid cell
    if(
        mMode == Mode::LAUNCH_POSITION_SELECTION
        || getModel().getCurrentPhase().mGamePhase != GamePhase::PLAY
        || getModel().getCurrentPhase().mTurnPhase != TurnPhase::MOVE_PIECE
        || boardLocation == mFocusedGridCell
    ) {
        return;
    }

    // guard: no occupant in this house
    const auto houseData { getModel().getHouseData(boardLocation) };
    if(houseData.mOccupant.mOwner == RoleID::NA) {
        return;
    }

    // guard: non-owner or non-active player can't query move for this piece
    const auto moveData { getModel().getBoardMoveData(houseData.mOccupant) };
    const auto currentPlayer { getModel().getCurrentPlayer() };
    if(
        currentPlayer.mPlayer != mControlledBy
        || currentPlayer.mRole != houseData.mOccupant.mOwner
    ) {
        return;
    }

    // switch focus to moves possible with piece at current location
    clearLights();
    mFocusedGridCell = boardLocation;
    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    soundPlayer.playEffect(UrSoundFX::BUTTON_HOVER);

    // no move possible, so nothing highlighted
    if(!(moveData.mFlags&MoveResultData::IS_POSSIBLE)) {
        return;
    }

    // game ending move highlighted in gold
    if(moveData.mFlags&MoveResultData::ENDS_GAME) {
        addLight(boardLocation, kYellowCone);
        return;
    }

    // piece success in green
    if(moveData.mFlags&MoveResultData::COMPLETES_ROUTE) {
        addLight(boardLocation, kGreenCone);
        return;
    }

    // regular moves in white
    addLight(moveData.mMoveLocation, kWhiteCone);
}

void UrSceneView::onLaunchPieceInitiated(PieceTypeID pieceType) {
    std::cout << "UrSceneView: Launch piece initiated\n";

    // guard: we can only consider launching pieces during the play phase
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

void UrSceneView::onLaunchPieceHovered(PieceTypeID pieceType) {
    std::cout << "Launch button hovered\n";

    // guard: we can only consider launching pieces during the play phase
    if(
        mMode == Mode::TRANSITION
        || mMode == Mode::LAUNCH_POSITION_SELECTION
        || getModel().getCurrentPhase().mGamePhase != GamePhase::PLAY
    ) return;

    clearLights();
    mFocusedGridCell = kGridUnfocused;

    const PieceIdentity pieceIdentity {
        .mType { pieceType },
        .mOwner{ getModel().getCurrentPlayer().mRole },
    };
    const std::vector<glm::u8vec2> launchPositions { getModel().getLaunchPositions(pieceIdentity) };
    assert(launchPositions.size() && "Every piece must have at least one launch position associated with it");
    std::cout << "Launch positions:\n";
    for(const auto& position: launchPositions) {
        std::cout << "\t" << static_cast<int>(position.x) << ", " << static_cast<int>(position.y) << "\n";
    }
    addLights(launchPositions);
}

void UrSceneView::onLaunchPieceCanceled() {
    std::cout << "UrSceneView: Launch piece canceled\n";
    mMode = Mode::GENERAL;
    clearLights();
    mFocusedGridCell = kGridUnfocused;
}

void UrSceneView::onMoveMade(const MoveResultData& moveResultData) {
    std::cout << "UrSceneView: move made\n";
    mMode = Mode::GENERAL;
    clearLights();
    mFocusedGridCell = kGridUnfocused;

    auto& soundPlayer {
        getSimObject().getWorld().lock()
            ->getSingletonSystem<ToyMaker::SceneSystem>()
            ->getByPath<UrSoundPlayer&>("/ur_sounds/@UrSoundPlayer")
    };
    soundPlayer.playEffect(UrSoundFX::PIECE_MOVE, 2);

    const PieceIdentity& displacedPieceIdentity { moveResultData.mDisplacedPiece.mIdentity };
    const PieceIdentity& movedPieceIdentity { moveResultData.mMovedPiece.mIdentity };
    uint32_t animationOffset { 0 };

    if(displacedPieceIdentity.mOwner != RoleID::NA) {
        // Schedule animation for this piece getting knocked off the board
        ToyMaker::Placement displacedPiecePlacement {
            mPieceNodeMap.at(displacedPieceIdentity)->getComponent<ToyMaker::Placement>()
        };
        mAnimationKeys.push(
            UrPieceAnimationKey {
                .mTime { animationOffset },
                .mPieceIdentity { displacedPieceIdentity },
                .mPlacement { displacedPiecePlacement },
            }
        );
        displacedPiecePlacement.mPosition.y += 15.f;
        animationOffset += 700;
        mAnimationKeys.push(
            UrPieceAnimationKey {
                .mTime { animationOffset },
                .mPieceIdentity{ displacedPieceIdentity },
                .mPlacement { displacedPiecePlacement },
                .mRemove { true }
            }
        );
    }

    ToyMaker::Placement startPlacement;
    ToyMaker::Placement endPlacement;
    if(!mPieceNodeMap[movedPieceIdentity]) {
        // This piece has just been launched
        endPlacement = ToyMaker::Placement {
            .mPosition {
                getBoard().gridIndicesToBoardPoint(moveResultData.mMovedPiece.mLocation)
            }
        };
        startPlacement = endPlacement;
        startPlacement.mPosition.y += 15.f;

    } else {
        startPlacement = mPieceNodeMap[movedPieceIdentity]->getComponent<ToyMaker::Placement>();

        if(moveResultData.mFlags & MoveResultData::COMPLETES_ROUTE) {
            endPlacement = startPlacement;
            endPlacement.mPosition.z += 15.f;

        } else {
            endPlacement = ToyMaker::Placement {
                .mPosition {
                    getBoard().gridIndicesToBoardPoint(
                        moveResultData.mMovedPiece.mLocation
                    )
                }
            };
        }
    }

    // Schedule animation for the launch/board move
    mAnimationKeys.push(
        UrPieceAnimationKey {
            .mTime { animationOffset },
            .mPieceIdentity { movedPieceIdentity },
            .mPlacement { startPlacement }
        }
    );
    animationOffset += 700;
    mAnimationKeys.push(
        UrPieceAnimationKey {
            .mTime { animationOffset },
            .mPieceIdentity { movedPieceIdentity },
            .mPlacement { endPlacement },
            .mRemove { static_cast<bool>(moveResultData.mFlags&MoveResultData::COMPLETES_ROUTE) }
        }
    );
}

void UrSceneView::onControlInterface(PlayerID player) {
    mControlledBy = player;
    clearLights();
    mFocusedGridCell = kGridUnfocused;
}

void UrSceneView::onControllerReady() {
    const std::string viewPath { getSimObject().getPathFromAncestor(mGameOfUrController.lock()) };

    mSigViewSubscribed.emit(viewPath);
}

void UrSceneView::onViewUpdateStarted() {
    mAnimationTimeMillis = 0;
    mMode = Mode::TRANSITION;
    clearLights();
    mFocusedGridCell = kGridUnfocused;
}

void UrSceneView::variableUpdate(uint32_t variableStepMillis) {
    if(mMode != Mode::TRANSITION) return;

    mAnimationTimeMillis += variableStepMillis;
    std::map<PieceIdentity, UrPieceAnimationKey> currentKeys {};

    // retrieve the latest key frame till now for each piece
    while(
        !mAnimationKeys.empty() 
        && mAnimationKeys.top().mTime <= mAnimationTimeMillis
    ) {
        currentKeys[mAnimationKeys.top().mPieceIdentity] = mAnimationKeys.top();
        mAnimationKeys.pop();
    }


    for(auto key: currentKeys) {

        // Load the model and place it in the scene if it isn't already loaded 
        if(!mPieceNodeMap[key.first]) {
            std::shared_ptr<ToyMaker::StaticModel> model {
                ToyMaker::ResourceDatabase::GetRegisteredResource<ToyMaker::StaticModel>(
                    mPieceModelMap.at(key.first)
                )
            };
            mPieceNodeMap[key.first] = ToyMaker::SceneNode::create(
                ToyMaker::Placement{},
                mPieceModelMap.at(key.first),
                model
            );
            getSimObject().addNode(
                mPieceNodeMap[key.first],
                "/viewport_3D/"
            );
        }

        // see if there is a future keyframe for each of the pieces
        // implicated in the current key
        std::vector<UrPieceAnimationKey> temp {};
        while(
            !mAnimationKeys.empty()
            && mAnimationKeys.top().mPieceIdentity != key.first
        ) {
            temp.push_back(mAnimationKeys.top());
            mAnimationKeys.pop();
        }
        bool endsAnimation { mAnimationKeys.empty() };

        // We've reached the end of the animation as far as this piece goes
        if(endsAnimation) {
            mPieceNodeMap[key.first]->updateComponent<ToyMaker::Placement>(
                key.second.mPlacement
            );
            if(key.second.mRemove) {
                mPieceNodeMap.at(key.first)->removeNode("/");
                mPieceNodeMap.erase(key.first);
            }

        // Otherwise, interpolate between the current frame and the next one
        } else {
            const float progress {
                (mAnimationTimeMillis - key.second.mTime) * 1.f
                / (mAnimationKeys.top().mTime - key.second.mTime)
            };
            const ToyMaker::Placement currentPlacement { ToyMaker::Interpolator<ToyMaker::Placement>{}(
                key.second.mPlacement, mAnimationKeys.top().mPlacement, progress
            ) };
            mPieceNodeMap[key.first]->updateComponent<ToyMaker::Placement>(currentPlacement);
        }

        // push keyframes unrelated to this piece back into the queue
        for(auto storedKey: temp) {
            mAnimationKeys.push(storedKey);
        }

        // only remove the present keyframe if the current key ended the animation
        if(!endsAnimation) {
            mAnimationKeys.push(key.second);
        }
    }

    // There's some animation left to go
    if(!mAnimationKeys.empty()) return;

    // We've performed all queued animations, and it's time to signal the game 
    // controller
    const std::string viewPath { getSimObject().getPathFromAncestor(mGameOfUrController.lock()) };
    mMode = Mode::GENERAL;
    mSigViewUpdateCompleted.emit(viewPath);
}
