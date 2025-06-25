#include <cmath>

#include "model.hpp"
#include "piece_type.hpp"

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
    mPlayers[PlayerID::PLAYER_A].initializeWithRole(playerAGoesFirst? RoleID::ONE: RoleID::TWO);
    mPlayers[PlayerID::PLAYER_B].initializeWithRole(playerAGoesFirst? RoleID::TWO: RoleID::ONE);

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
        mRoundPhase = getRole(requester) == RoleID::TWO? RoundPhase::END: RoundPhase::IN_PROGRESS;
    }
}

void GameOfUrModel::movePiece(PieceIdentity piece, glm::u8vec2 toLocation, PlayerID requester) {
    assert(canMovePiece(piece, toLocation, requester) && "this player may not move this piece at the present time");
    const MoveResultData moveResults { getMoveData(piece, toLocation) };

    // update moved piece state
    std::shared_ptr<Piece> movedPiece { mPlayers[requester].getPiece(piece.mType) };
    movedPiece->setState(moveResults.mMovedPiece.mState);

    // update displaced piece state, if necessary
    std::weak_ptr<Piece> weakPtrDisplacedPiece {mBoard.move(getRole(requester), movedPiece, toLocation, mDice->getResult(mGamePhase))};
    if(std::shared_ptr<Piece> displacedPiece = weakPtrDisplacedPiece.lock()) {
        assert(moveResults.mDisplacedPiece.mState == Piece::State::UNLAUNCHED && "Results should indicate that the piece is in the UNLAUNCHED state after move");
        displacedPiece->setState(Piece::State::UNLAUNCHED);
    }

    // update counters for the player who moved
    deductCounters(moveResults.mCountersLost, requester);
    payCounters(moveResults.mCountersWon, requester);

    // update phase data
    mTurnPhase = TurnPhase::END;
    mRoundPhase = piece.mOwner == RoleID::TWO? RoundPhase::END: RoundPhase::IN_PROGRESS;

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

const Piece& GameOfUrModel::getPiece(const PieceIdentity& pieceIdentity) const {
    assert(pieceIdentity.mOwner != RoleID::NA && "A piece without an owner role is invalid");
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

RoleID GameOfUrModel::getWinner() const {
    if(mGamePhase != GamePhase::END) return RoleID::NA;
    const uint8_t nVictoryPiecesA { mPlayers[PLAYER_A].getNPieces(Piece::State::FINISHED) };
    const uint8_t nVictoryPiecesB { mPlayers[PLAYER_B].getNPieces(Piece::State::FINISHED) };
    assert(
        nVictoryPiecesA != nVictoryPiecesB
        && (
            nVictoryPiecesA == 5
            || nVictoryPiecesB == 5
        ) && "Invalid victory state; requires review"
    );
    return nVictoryPiecesA == 5? getRole(PLAYER_A): getRole(PLAYER_B);
}

RoleID GameOfUrModel::getRole(PlayerID player) const {
    return mPlayers[player].getRole();
}

PlayerID GameOfUrModel::getPlayer(RoleID role) const {
    assert(mGamePhase != GamePhase::INITIATIVE && "Player rolls are not assigned until after the initiative phase ends");
    assert(role != RoleID::NA && "Cannot retrieve a player without a role through this method");
    for(uint8_t player{0}; player < 2; ++player){
        if(mPlayers[player].getRole() == role) {
            return static_cast<PlayerID>(player);
        }
    }
    assert(false && "After the initiative phase, both players should have roles assigned");
}

bool GameOfUrModel::canMovePiece(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const {
    assert(pieceIdentity.mOwner != RoleID::NA && "A piece without an owner role is invalid");
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
bool GameOfUrModel::canMoveBoardPiece(PieceIdentity pieceIdentity, PlayerID requester) const {
    assert(pieceIdentity.mOwner != RoleID::NA && "A piece without an owner role is invalid");
    return (
        canMovePiece(
            pieceIdentity, 
            getBoardMoveData(pieceIdentity).mMovedPiece.mLocation,
            requester
        ) && getPiece(pieceIdentity).getState() == Piece::State::ON_BOARD
    );
}
bool GameOfUrModel::canLaunchPiece(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const  {
    assert(pieceIdentity.mOwner != RoleID::NA && "A piece without an owner role is invalid");
    return (
        canMovePiece(
            pieceIdentity, 
            getBoardMoveData(pieceIdentity).mMovedPiece.mLocation,
            requester
        ) && getPiece(pieceIdentity).getState() == Piece::State::UNLAUNCHED
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

GameScoreData GameOfUrModel::getScore() const {
    return {
        .mCommonPoolCounters { mCounters },
        .mPlayerOneCounters { static_cast<uint8_t>(mGamePhase != GamePhase::INITIATIVE? mPlayers[getPlayer(RoleID::ONE)].getNCounters(): 0) },
        .mPlayerTwoCounters { static_cast<uint8_t>(mGamePhase != GamePhase::INITIATIVE? mPlayers[getPlayer(RoleID::TWO)].getNCounters(): 0) },
        .mPlayerOneVictoryPieces { static_cast<uint8_t>(mGamePhase != GamePhase::INITIATIVE? mPlayers[getPlayer(RoleID::ONE)].getNPieces(Piece::State::FINISHED): 0) },
        .mPlayerTwoVictoryPieces { static_cast<uint8_t>(mGamePhase != GamePhase::INITIATIVE? mPlayers[getPlayer(RoleID::TWO)].getNPieces(Piece::State::FINISHED): 0) },
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

GamePieceData GameOfUrModel::getPieceData(PieceIdentity gamePiece) const {
    assert(gamePiece.mOwner != RoleID::NA && "Game pieces without owners are not valid");
    assert(mGamePhase != GamePhase::INITIATIVE && "Game piece data does not exist until after the initiative phase is over");
    return getPieceData(getPlayer(gamePiece.mOwner), gamePiece.mType);
}

GamePieceData GameOfUrModel::getPieceData(PlayerID player, PieceTypeID pieceType) const {
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
        .mNUnlaunchedPieces { mPlayers[player].getNPieces(Piece::State::UNLAUNCHED) },
        .mNBoardPieces { mPlayers[player].getNPieces(Piece::State::ON_BOARD) },
        .mNVictoryPieces { mPlayers[player].getNPieces(Piece::State::FINISHED) },
    };
}

PlayerData GameOfUrModel::getCurrentPlayer() const {
    return getPlayerData(mCurrentPlayer);
}

PlayerData GameOfUrModel::getPlayerData(RoleID role) const {
    return getPlayerData(getPlayer(role));
}

DiceData GameOfUrModel::getDiceData() const {
    return {
        .mState { mDice->getState() },
        .mPrimaryRoll { mDice->getSecondaryRoll() },
        .mSecondaryRoll { mDice->getSecondaryRoll() },
        .mResultScore { mDice->getResult(mGamePhase) },
        .mPreviousResult { mPreviousRoll },
    };
}

MoveResultData GameOfUrModel::getBoardMoveData(PieceIdentity pieceID) const {
    assert(pieceID.mOwner != RoleID::NA && "Pieces without owners are invalid");
    if(mGamePhase != GamePhase::PLAY || mTurnPhase != TurnPhase::MOVE_PIECE) {
        return { };
    }

    const Piece& piece { mPlayers[getPlayer(pieceID.mOwner)].cGetPiece(pieceID.mType) };
    const glm::u8vec2 moveLocation { mBoard.computeMoveLocation(piece, mDice->getResult(mGamePhase)) };
    if(!canMovePiece(pieceID, moveLocation, getPlayer(pieceID.mOwner))) {
        return { };
    }

    return getMoveData(pieceID, moveLocation);
}

MoveResultData GameOfUrModel::getLaunchMoveData(PieceIdentity pieceID, glm::u8vec2 launchLocation) const {
    assert(pieceID.mOwner != RoleID::NA && "Pieces without owners are invalid");
    if(mGamePhase != GamePhase::PLAY || mTurnPhase != TurnPhase::MOVE_PIECE) {
        return {};
    }

    if(!canMovePiece(pieceID, launchLocation, getPlayer(pieceID.mOwner))) {
        return {};
    }

    return getMoveData(pieceID, launchLocation);
}

MoveResultData GameOfUrModel::getMoveData(PieceIdentity pieceID, glm::u8vec2 moveLocation) const {
    const Piece& piece { mPlayers[getPlayer(pieceID.mOwner)].cGetPiece(pieceID.mType) };
    MoveResultData::flags moveFlags {
        MoveResultData::IS_POSSIBLE
    };

    GamePieceData displacedPieceData { .mIdentity{ .mOwner=RoleID::NA } };
    if(mBoard.houseIsOccupied(moveLocation)) {
        displacedPieceData.mIdentity = mBoard.getOccupant(moveLocation);
        displacedPieceData.mState = Piece::State::UNLAUNCHED;
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
                mPlayers[getPlayer(pieceID.mOwner)].getNPieces(Piece::State::FINISHED)
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
        )? static_cast<uint8_t>(std::min(kGamePieceTypes[piece.getType()].mCost, mPlayers[getPlayer(pieceID.mOwner)].getNCounters())):
        static_cast<uint8_t>(0)
    };
    const uint8_t nCountersWon { 
        (moveFlags&MoveResultData::LANDS_ON_ROSETTE)?
        static_cast<uint8_t>(std::min(kGamePieceTypes[piece.getType()].mCost, mCounters)):
        moveFlags&MoveResultData::ENDS_GAME? static_cast<uint8_t>(mCounters): static_cast<uint8_t>(0)
    };

    return {
        .mFlags { moveFlags },
        .mDisplacedPiece { displacedPieceData },
        .mMovedPiece { movedPieceData },
        .mCountersWon { nCountersWon },
        .mCountersLost { nCountersLost },
    };
}

std::vector<std::pair<PieceIdentity, glm::u8vec2>> GameOfUrModel::getAllPossibleMoves() const {
    if(
        mGamePhase != GamePhase::PLAY
        || mTurnPhase != TurnPhase::MOVE_PIECE
    ) return {};

    std::vector<std::pair<PieceIdentity, glm::u8vec2>> possibleMoves {};

    for(uint8_t type {0}; type < PieceTypeID::TOTAL; ++type) {

        const PieceIdentity pieceIdentity { PieceIdentity{.mType { static_cast<PieceTypeID>(type) }, .mOwner {getRole(mCurrentPlayer)} } };
        const Piece& piece { getPiece(PieceIdentity{.mType { static_cast<PieceTypeID>(type) }, .mOwner { getRole(mCurrentPlayer) }}) };

        switch(piece.getState()) {

            case Piece::State::UNLAUNCHED:
                for(glm::u8vec2 launchPosition: mBoard.getValidLaunchPositions(piece.getIdentity())) {
                    possibleMoves.push_back({pieceIdentity, launchPosition});
                }
                break;

            case Piece::State::ON_BOARD: 
                {
                    const glm::u8vec2 movePosition { mBoard.computeMoveLocation(piece, mDice->getResult(GamePhase::PLAY)) };
                    if(mBoard.canMove(piece.getOwner(), piece, movePosition, mDice->getResult(GamePhase::PLAY))) {
                        possibleMoves.push_back({pieceIdentity, movePosition});
                    }
                }
                break;

            case Piece::State::FINISHED:
                break;
        }
    }

    return possibleMoves;
}
