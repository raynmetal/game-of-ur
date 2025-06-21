#ifndef ZOAPPGAMEOFURMODEL_H
#define ZOAPPGAMEOFURMODEL_H

#include <cstdint>
#include <array>
#include <memory>


#include "game_phase.hpp"
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
    GamePieceIdentity mOccupant;
    glm::u8vec2 mLocation;
    glm::i8vec2 mNextCellDirection;
};

struct GamePieceData {
    GamePieceIdentity mIdentity;
    GamePiece::State mState;
    glm::u8vec2 mLocation;
};

struct GamePhaseData {
    GamePhase mGamePhase;
    RoundPhase mRoundPhase;
    TurnPhase mTurnPhase;
    PlayerID mTurn;
    PlayerRoleID mWinner;
};

struct DiceData {
    Dice::State mState;
    uint8_t mPrimaryRoll;
    bool mSecondaryRoll;
    uint8_t mResultScore;
};

struct PlayerData {
    PlayerID mPlayer;
    PlayerRoleID mRole;
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
    void movePiece(GamePieceIdentity piece, glm::u8vec2 toLocation, PlayerID requester);
    void advanceOneTurn(PlayerID requester);

    std::vector<std::pair<GamePieceIdentity, glm::u8vec2>> getAllPossibleMoves() const;
    uint8_t getNCounters() const { return mCounters; }
    GamePhaseData getCurrentPhase() const;
    HouseData getHouseData(glm::u8vec2 location) const;
    GamePieceData getPieceData(GamePieceIdentity gamePiece) const;
    GamePieceData getPieceData(PlayerID player, GamePieceTypeID pieceType) const;
    PlayerData getPlayerData(PlayerID player) const;
    PlayerData getPlayerData(PlayerRoleID player) const;
    PlayerData getCurrentPlayer() const;
    DiceData getDiceData() const;
    MoveResultData getBoardMoveData(GamePieceIdentity piece) const;
    MoveResultData getLaunchMoveData(GamePieceIdentity piece, glm::u8vec2 launchLocation) const;

    bool canRollDice(PlayerID requester) const;
    bool canMovePiece(GamePieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const;
    bool canAdvanceOneTurn(PlayerID requester) const;
    bool canStartPhasePlay() const;
private:

    PlayerRoleID getWinner() const;
    PlayerRoleID getRole(PlayerID player) const;
    PlayerID getPlayer(PlayerRoleID role) const;
    const GamePiece& getPiece(const GamePieceIdentity& pieceIdentity) const;
    MoveResultData getMoveData(GamePieceIdentity piece, glm::u8vec2 moveLocation) const;

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
