#ifndef ZOAPPGAMEOFURMODEL_H
#define ZOAPPGAMEOFURMODEL_H

#include <cstdint>
#include <array>
#include <memory>


#include "phase.hpp"
#include "board.hpp"
#include "player.hpp"
#include "dice.hpp"

enum PlayerID: uint8_t {
    PLAYER_A=0,
    PLAYER_B,
};

struct HouseData {
    House::Type mType;
    House::Region mRegion;
    PieceIdentity mOccupant;
    glm::u8vec2 mLocation;
    glm::i8vec2 mNextCellDirection;
};

struct GamePieceData {
    PieceIdentity mIdentity;
    Piece::State mState;
    glm::u8vec2 mLocation;
};

struct GamePhaseData {
    GamePhase mGamePhase;
    RoundPhase mRoundPhase;
    TurnPhase mTurnPhase;
    PlayerID mTurn;
    RoleID mWinner;
};

struct GameScoreData {
    uint8_t mCommonPoolCounters;
    uint8_t mPlayerOneCounters;
    uint8_t mPlayerTwoCounters;

    uint8_t mPlayerOneVictoryPieces;
    uint8_t mPlayerTwoVictoryPieces;
};

struct DiceData {
    Dice::State mState;
    uint8_t mPrimaryRoll;
    bool mSecondaryRoll;
    uint8_t mResultScore;
    uint8_t mPreviousResult;
};

struct PlayerData {
    PlayerID mPlayer;
    RoleID mRole;
    bool mIsWinner;
    uint8_t mCounters;

    uint8_t mNUnlaunchedPieces;
    uint8_t mNBoardPieces;
    uint8_t mNVictoryPieces;
};

struct MoveResultData {
    using flags=uint8_t;
    enum Flags: flags {
        IS_POSSIBLE=0x1,
        PASSES_ROSETTE=0x2,
        LANDS_ON_ROSETTE=0x4,
        COMPLETES_ROUTE=0x8,
        ENDS_GAME=0x10,
    };
    flags mFlags;
    GamePieceData mDisplacedPiece;
    GamePieceData mMovedPiece;
    uint8_t mCountersWon;
    uint8_t mCountersLost;
};

class GameOfUrModel {
public:
    GameOfUrModel()=default;
    GameOfUrModel(GameOfUrModel&& other)=default;
    inline GameOfUrModel& operator=(GameOfUrModel&& other);

    GameOfUrModel(const GameOfUrModel& other)=delete;
    GameOfUrModel& operator=(const GameOfUrModel& other)=delete;

    void reset();
    void startPhasePlay();

    void rollDice(PlayerID requester);
    void movePiece(PieceIdentity piece, glm::u8vec2 toLocation, PlayerID requester);
    void advanceOneTurn(PlayerID requester);

    std::vector<std::pair<PieceIdentity, glm::u8vec2>> getAllPossibleMoves() const;
    uint8_t getNCounters() const { return mCounters; }
    GamePhaseData getCurrentPhase() const;
    GameScoreData getScore() const;
    HouseData getHouseData(glm::u8vec2 location) const;
    GamePieceData getPieceData(PieceIdentity gamePiece) const;
    GamePieceData getPieceData(PlayerID player, PieceTypeID pieceType) const;
    PlayerData getPlayerData(PlayerID player) const;
    PlayerData getPlayerData(RoleID player) const;
    PlayerData getCurrentPlayer() const;
    DiceData getDiceData() const;
    MoveResultData getBoardMoveData(PieceIdentity piece) const;
    MoveResultData getLaunchMoveData(PieceIdentity piece, glm::u8vec2 launchLocation) const;

    bool canRollDice(PlayerID requester) const;
    bool canLaunchPiece(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const;
    bool canMoveBoardPiece(PieceIdentity pieceIdentity, PlayerID requester) const;
    bool canAdvanceOneTurn(PlayerID requester) const;
    bool canStartPhasePlay() const;

private:
    bool canMovePiece(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const;
    RoleID getWinner() const;
    RoleID getRole(PlayerID player) const;
    PlayerID getPlayer(RoleID role) const;
    const Piece& getPiece(const PieceIdentity& pieceIdentity) const;
    MoveResultData getMoveData(PieceIdentity piece, glm::u8vec2 moveLocation) const;

    void deductCounters(uint8_t counters, PlayerID fromPlayer);
    void payCounters(uint8_t counters, PlayerID toPlayer);

    GamePhase mGamePhase { GamePhase::INITIATIVE };
    TurnPhase mTurnPhase { TurnPhase::ROLL_DICE };
    RoundPhase mRoundPhase { RoundPhase::IN_PROGRESS };

    PlayerID mCurrentPlayer { PLAYER_A };
    uint8_t mCounters { 0 };
    std::unique_ptr<Dice> mDice { std::make_unique<Dice>() };
    Board mBoard {};
    std::array<Player, 2> mPlayers {};
    uint8_t mPreviousRoll {0};
};

inline GameOfUrModel& GameOfUrModel::operator=(GameOfUrModel&& other)=default;
#endif
