#include <cmath>

#include "game_of_ur_model.hpp"
#include "game_piece_type.hpp"

void GameOfUrModel::reset() {
    *this = GameOfUrModel{};
}

void GameOfUrModel::startPhasePlay() {
    assert(canStartPhasePlay() && "Invalid conditions for starting the play phase");

    bool playerAGoesFirst { mPreviousRoll > mDice->getResult(GamePhase::INITIATIVE) };

    // update game phase
    mGamePhase = GamePhase::PLAY;
    mTurnPhase = TurnPhase::ROLL_DICE;
    mRoundPhase = RoundPhase::IN_PROGRESS;
    mPreviousRoll = 0;

    // assign roles to each player
    mCurrentPlayer = playerAGoesFirst? PlayerID::PLAYER_A: PlayerID::PLAYER_B;
    mPlayers[PlayerID::PLAYER_A].initializeRole(playerAGoesFirst? PlayerRoleID::ONE: PlayerRoleID::TWO);
    mPlayers[PlayerID::PLAYER_B].initializeRole(playerAGoesFirst? PlayerRoleID::TWO: PlayerRoleID::ONE);

    // collect 10 counters from each player and place them in the common pool
    deductCounters(10, PlayerID::PLAYER_A);
    deductCounters(10, PlayerID::PLAYER_B);
}

void GameOfUrModel::rollDice(PlayerID requester) {
    assert(canRollDice(requester) && "This player cannot roll dice presently");
    mDice->roll();

    // If the dice have been rolled a second time, and there are no
    // pieces to move, we've reached the end of this turn
    if(
        mDice->getState() == Dice::State::SECONDARY_ROLLED
        && (
            mGamePhase == GamePhase::INITIATIVE 
            || !mDice->getResult(GamePhase::PLAY)
            || getAllPossibleMoves().empty()
        )
    ) {
        mTurnPhase = TurnPhase::END;
        mRoundPhase = getRole(requester) == PlayerRoleID::TWO? RoundPhase::END: RoundPhase::IN_PROGRESS;
    }
}

void GameOfUrModel::movePiece(GamePieceIdentity piece, glm::u8vec2 toLocation, PlayerID requester) {
    assert(canMovePiece(piece, toLocation, requester) && "this player may not move this piece at the present time");
    const MoveResultData moveResults { getMoveData(piece, toLocation) };

    // update moved piece state
    std::shared_ptr<GamePiece> movedPiece { mPlayers[requester].getPiece(piece.mType) };
    movedPiece->setState(moveResults.mMovedPiece.mState);

    // update displaced piece state, if necessary
    std::weak_ptr<GamePiece> weakPtrDisplacedPiece {mBoard.move(getRole(requester), movedPiece, toLocation, mDice->getResult(mGamePhase))};
    if(std::shared_ptr<GamePiece> displacedPiece = weakPtrDisplacedPiece.lock()) {
        assert(moveResults.mDisplacedPiece.mState == GamePiece::State::UNLAUNCHED && "Results should indicate that the piece is in the UNLAUNCHED state after move");
        displacedPiece->setState(GamePiece::State::UNLAUNCHED);
    }

    // update counters for the player who moved
    deductCounters(moveResults.mCountersLost, requester);
    payCounters(moveResults.mCountersWon, requester);

    // update phase data
    mTurnPhase = TurnPhase::END;
    mRoundPhase = piece.mOwner == PlayerRoleID::TWO? RoundPhase::END: RoundPhase::IN_PROGRESS;

    if(moveResults.mFlags&MoveResultData::ENDS_GAME) {
        mGamePhase = GamePhase::END;
    }
}

void GameOfUrModel::advanceOneTurn(PlayerID requester) {
    assert(canAdvanceOneTurn(requester) && "cannot advance to next turn at this stage");

    // store the current dice roll in case it will be needed later on
    mPreviousRoll = mDice->getResult(mGamePhase);
    mDice.reset();
    mCurrentPlayer = static_cast<PlayerID>((mCurrentPlayer + 1) % mPlayers.size());
    mRoundPhase = RoundPhase::IN_PROGRESS;
    mTurnPhase = TurnPhase::ROLL_DICE;
}

void GameOfUrModel::payCounters(uint8_t counters, PlayerID player) {
    if(counters > mCounters) counters = mCounters;
    mCounters -= counters;
    mPlayers[player].depositCounters(counters);
}

void GameOfUrModel::deductCounters(uint8_t counters, PlayerID player) {
    counters = mPlayers[player].deductCounters(counters);
    mCounters += counters;
}

bool GameOfUrModel::canRollDice(PlayerID requester) const {
    return (
        // The dice can be rolled in every phase by whoever's turn it
        // is so long as neither the game nor the turn has ended
        requester == mCurrentPlayer
        && mGamePhase != GamePhase::END
        && mTurnPhase != TurnPhase::END
        && mDice->canRoll()
    );
}

const GamePiece& GameOfUrModel::getPiece(const GamePieceIdentity& pieceIdentity) const {
    assert(pieceIdentity.mOwner != PlayerRoleID::NA && "A piece without an owner role is invalid");
    assert(mGamePhase != GamePhase::INITIATIVE && "Roles and pieces have not been assigned to players yet");

    bool ownerFound { false };
    PlayerID owner;
    for(uint8_t player { PLAYER_A }; player <= mPlayers.size(); ++player) {
        if(mPlayers[player].getRole() == pieceIdentity.mOwner) {
            owner = static_cast<PlayerID>(player);
            ownerFound = true;
            break;
        }
    }
    assert(ownerFound && "Roles and pieces have not been assigned to either player");

    return mPlayers[owner].cGetPiece(pieceIdentity.mType);
}

PlayerRoleID GameOfUrModel::getWinner() const {
    if(mGamePhase != GamePhase::END) return PlayerRoleID::NA;
    const uint8_t nVictoryPiecesA { mPlayers[PLAYER_A].getNPieces(GamePiece::State::FINISHED) };
    const uint8_t nVictoryPiecesB { mPlayers[PLAYER_B].getNPieces(GamePiece::State::FINISHED) };
    assert(
        nVictoryPiecesA != nVictoryPiecesB
        && (
            nVictoryPiecesA == 5
            || nVictoryPiecesB == 5
        ) && "Invalid victory state; requires review"
    );
    return nVictoryPiecesA == 5? getRole(PLAYER_A): getRole(PLAYER_B);
}

PlayerRoleID GameOfUrModel::getRole(PlayerID player) const {
    return mPlayers[player].getRole();
}

PlayerID GameOfUrModel::getPlayer(PlayerRoleID role) const {
    assert(mGamePhase != GamePhase::INITIATIVE && "Player rolls are not assigned until after the initiative phase ends");
    assert(role != PlayerRoleID::NA && "Cannot retrieve a player without a role through this method");
    for(uint8_t player{0}; player < 2; ++player){
        if(mPlayers[player].getRole() == role) {
            return static_cast<PlayerID>(player);
        }
    }
    assert(false && "After the initiative phase, both players should have roles assigned");
}

bool GameOfUrModel::canMovePiece(GamePieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const {
    assert(pieceIdentity.mOwner != PlayerRoleID::NA && "A piece without an owner role is invalid");
    // a piece may only be moved by a player if the piece belongs to 
    // them, the current turn phase permits it, and if there is space on
    // a valid destination to move to
    return (
        mGamePhase == GamePhase::PLAY
        && mTurnPhase == TurnPhase::MOVE_PIECE
        && requester == mCurrentPlayer
        && mBoard.canMove(mPlayers[requester].getRole(), getPiece(pieceIdentity), toLocation, mDice->getResult(mGamePhase))
    );
}

bool GameOfUrModel::canAdvanceOneTurn(PlayerID requester) const {
    // current turn may be ended only if the game hasn't ended, but
    // the turn has, with an exception made for initiative rounds
    return (
        mTurnPhase == TurnPhase::END
        && requester == mCurrentPlayer && (
            mGamePhase == GamePhase::PLAY || (
                mGamePhase == GamePhase::INITIATIVE && (
                    mRoundPhase == RoundPhase::IN_PROGRESS
                    || mPreviousRoll == mDice->getResult(GamePhase::INITIATIVE)
                )
            )
        )
    );
}

bool GameOfUrModel::canStartPhasePlay() const {
    return (
        // We can only start the play phase once the initiative round is
        // over and has been won
        mGamePhase == GamePhase::INITIATIVE
        && mRoundPhase == RoundPhase::END
        && mTurnPhase == TurnPhase::END
        && mDice->getState() == Dice::SECONDARY_ROLLED
        && mDice->getResult(GamePhase::INITIATIVE) != mPreviousRoll
    );
}

GamePhaseData GameOfUrModel::getCurrentPhase() const {
    return {
        .mGamePhase { mGamePhase },
        .mRoundPhase { mRoundPhase },
        .mTurnPhase { mTurnPhase },
        .mTurn { mCurrentPlayer },
        .mWinner { getWinner() },
    };
}

HouseData GameOfUrModel::getHouseData(glm::u8vec2 location) const {
    assert(mBoard.isValidHouse(location) && "This location does not correspond to a valid house on the board");
    return {
        .mType { mBoard.getType(location) },
        .mRegion { mBoard.getRegion(location) },
        .mOccupant { mBoard.getOccupant(location) },
        .mLocation { location },
        .mNextCellDirection { mBoard.getNextCellDirection(location) },
    };
}

GamePieceData GameOfUrModel::getPieceData(GamePieceIdentity gamePiece) const {
    assert(gamePiece.mOwner != PlayerRoleID::NA && "Game pieces without owners are not valid");
    assert(mGamePhase != GamePhase::INITIATIVE && "Game piece data does not exist until after the initiative phase is over");
    return getPieceData(getPlayer(gamePiece.mOwner), gamePiece.mType);
}

GamePieceData GameOfUrModel::getPieceData(PlayerID player, GamePieceTypeID pieceType) const {
    assert(mGamePhase != GamePhase::INITIATIVE && "Game piece data does not exist until after the initiative phase is over");
    return {
        .mIdentity { mPlayers[player].cGetPiece(pieceType).getIdentity() },
        .mState { mPlayers[player].cGetPiece(pieceType).getState() },
        .mLocation { mPlayers[player].cGetPiece(pieceType).getLocation() },
    };
}

PlayerData GameOfUrModel::getPlayerData(PlayerID player) const {
    return {
        .mPlayer { player },
        .mRole { mPlayers[player].getRole() },
        .mIsWinner { mPlayers[player].getRole() == getWinner() },
        .mCounters { mPlayers[player].getNCounters() },
        .mNUnlaunchedPieces { mPlayers[player].getNPieces(GamePiece::State::UNLAUNCHED) },
        .mNBoardPieces { mPlayers[player].getNPieces(GamePiece::State::ON_BOARD) },
        .mNVictoryPieces { mPlayers[player].getNPieces(GamePiece::State::FINISHED) },
    };
}

PlayerData GameOfUrModel::getCurrentPlayer() const {
    return getPlayerData(mCurrentPlayer);
}

PlayerData GameOfUrModel::getPlayerData(PlayerRoleID role) const {
    return getPlayerData(getPlayer(role));
}

DiceData GameOfUrModel::getDiceData() const {
    return {
        .mState { mDice->getState() },
        .mPrimaryRoll { mDice->getSecondaryRoll() },
        .mSecondaryRoll { mDice->getSecondaryRoll() },
        .mResultScore { mDice->getResult(mGamePhase) },
    };
}

MoveResultData GameOfUrModel::getBoardMoveData(GamePieceIdentity pieceID) const {
    assert(pieceID.mOwner != PlayerRoleID::NA && "Pieces without owners are invalid");
    if(mGamePhase != GamePhase::PLAY || mTurnPhase != TurnPhase::MOVE_PIECE) {
        return { };
    }

    const GamePiece& piece { mPlayers[getPlayer(pieceID.mOwner)].cGetPiece(pieceID.mType) };
    const glm::u8vec2 moveLocation { mBoard.computeMoveLocation(piece, mDice->getResult(mGamePhase)) };
    if(!canMovePiece(pieceID, moveLocation, getPlayer(pieceID.mOwner))) {
        return { };
    }

    return getMoveData(pieceID, moveLocation);
}

MoveResultData GameOfUrModel::getLaunchMoveData(GamePieceIdentity pieceID, glm::u8vec2 launchLocation) const {
    assert(pieceID.mOwner != PlayerRoleID::NA && "Pieces without owners are invalid");
    if(mGamePhase != GamePhase::PLAY || mTurnPhase != TurnPhase::MOVE_PIECE) {
        return {};
    }

    const GamePiece& piece { mPlayers[getPlayer(pieceID.mOwner)].cGetPiece(pieceID.mType) };
    if(!canMovePiece(pieceID, launchLocation, getPlayer(pieceID.mOwner))) {
        return {};
    }

    return getMoveData(pieceID, launchLocation);
}

MoveResultData GameOfUrModel::getMoveData(GamePieceIdentity pieceID, glm::u8vec2 moveLocation) const {
    const GamePiece& piece { mPlayers[getPlayer(pieceID.mOwner)].cGetPiece(pieceID.mType) };
    MoveResultData::flags moveFlags {
        MoveResultData::IS_POSSIBLE
    };

    GamePieceData displacedPieceData { .mIdentity{ .mOwner=PlayerRoleID::NA } };
    if(mBoard.houseIsOccupied(moveLocation)) {
        displacedPieceData.mIdentity = mBoard.getOccupant(moveLocation);
        displacedPieceData.mState = GamePiece::State::UNLAUNCHED;
        displacedPieceData.mLocation = glm::u8vec2 {0, 0};
    }

    GamePieceData movedPieceData { getPieceData(pieceID) };
    movedPieceData.mLocation = moveLocation;

    moveFlags |= mBoard.movePassesRosette(piece, moveLocation)? MoveResultData::PASSES_ROSETTE: 0;
    moveFlags |= mBoard.isRosette(moveLocation)? MoveResultData::LANDS_ON_ROSETTE: 0;
    moveFlags |= mBoard.isRouteEnd(moveLocation)? MoveResultData::COMPLETES_ROUTE: 0;
    moveFlags |= (
        (moveFlags&MoveResultData::COMPLETES_ROUTE) && (
            (
                mPlayers[getPlayer(pieceID.mOwner)].getNPieces(GamePiece::State::FINISHED)
                + 1 
            ) == 5
        )
    );


    const uint8_t nCountersLost { 
        (
            (moveFlags&MoveResultData::PASSES_ROSETTE) && !(
                (moveFlags&MoveResultData::LANDS_ON_ROSETTE) 
                || (moveFlags&MoveResultData::COMPLETES_ROUTE)
            )
        )? std::min(kGamePieceTypes[piece.getType()].mCost, mPlayers[getPlayer(pieceID.mOwner)].getNCounters()):
        0
    };
    const uint8_t nCountersWon { 
        (moveFlags&MoveResultData::LANDS_ON_ROSETTE)?
        std::min(kGamePieceTypes[piece.getType()].mCost, mCounters):
        moveFlags&MoveResultData::ENDS_GAME? mCounters: 0
    };

    return {
        .mFlags { moveFlags },
        .mDisplacedPiece { displacedPieceData },
        .mMovedPiece { movedPieceData },
        .mCountersWon { nCountersWon },
        .mCountersLost { nCountersLost },
    };
}

std::vector<std::pair<GamePieceIdentity, glm::u8vec2>> GameOfUrModel::getAllPossibleMoves() const {
    if(
        mGamePhase != GamePhase::PLAY
        || mTurnPhase != TurnPhase::MOVE_PIECE
    ) return {};

    std::vector<std::pair<GamePieceIdentity, glm::u8vec2>> possibleMoves {};

    for(uint8_t type {0}; type < GamePieceTypeID::TOTAL; ++type) {

        const GamePieceIdentity pieceIdentity { GamePieceIdentity{.mType { static_cast<GamePieceTypeID>(type) }, .mOwner {getRole(mCurrentPlayer)} } };
        const GamePiece& piece { getPiece(GamePieceIdentity{.mType { static_cast<GamePieceTypeID>(type) }, .mOwner { getRole(mCurrentPlayer) }}) };

        switch(piece.getState()) {

            case GamePiece::State::UNLAUNCHED:
                for(glm::u8vec2 launchPosition: mBoard.getValidLaunchPositions(piece.getIdentity())) {
                    possibleMoves.push_back({pieceIdentity, launchPosition});
                }
                break;

            case GamePiece::State::ON_BOARD: 
                {
                    const glm::u8vec2 movePosition { mBoard.computeMoveLocation(piece, mDice->getResult(GamePhase::PLAY)) };
                    if(mBoard.canMove(piece.getOwner(), piece, movePosition, mDice->getResult(GamePhase::PLAY))) {
                        possibleMoves.push_back({pieceIdentity, movePosition});
                    }
                }
                break;

            case GamePiece::State::FINISHED:
                break;
        }
    }

    return possibleMoves;
}
