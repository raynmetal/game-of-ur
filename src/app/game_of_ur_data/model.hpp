/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/model.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief The class containing the interface to the data model for the whole game (Game of Ur).
 * @version 0.3.2
 * @date 2025-09-12
 * 
 */

/**
 * @defgroup UrGame Game of Ur
 * 
 */

/**
 * @defgroup UrGameDataModel Data Model
 * @ingroup UrGame
 * 
 */

#ifndef ZOAPPGAMEOFURMODEL_H
#define ZOAPPGAMEOFURMODEL_H

#include <cstdint>
#include <array>
#include <memory>


#include "phase.hpp"
#include "board.hpp"
#include "player.hpp"
#include "dice.hpp"

/**
 * @ingroup UrGameDataModel
 * @brief The two players playing the game, as known to GameOfUrModel.
 * 
 */
enum PlayerID: uint8_t {
    PLAYER_A=0, //< Player one, usually a real human player.
    PLAYER_B, // Player two, either a computer or a human depending on the game mode.
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried about the state of a House.
 * 
 */
struct HouseData {
    /**
     * @brief The house's type.
     * 
     */
    House::Type mType;

    /**
     * @brief The region the house is in.
     * 
     */
    House::Region mRegion;

    /**
     * @brief The identity of the piece occupying this house, if any.
     * 
     */
    PieceIdentity mOccupant;

    /**
     * @brief The location of this house as per the game board.
     * 
     */
    glm::u8vec2 mLocation;

    /**
     * @brief The direction from this house to the next house (or to the end of the route, if this house is the last one).
     * 
     */
    glm::i8vec2 mNextCellDirection;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried about the state of a single game piece.
 * 
 */
struct GamePieceData {
    /**
     * @brief The identity of this piece, its owner and type.
     * 
     */
    PieceIdentity mIdentity;

    /**
     * @brief The state of this piece.
     * 
     */
    Piece::State mState;

    /**
     * @brief The location of this piece on the board (if it is on the board).
     * 
     */
    glm::u8vec2 mLocation;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried about the current phase of the game.
 * 
 */
struct GamePhaseData {
    /**
     * @brief The current high level phase of the game.
     * 
     */
    GamePhase mGamePhase;

    /**
     * @brief The part of the round the game is currently in.
     * 
     * The end of every second turn is considered the end of a round.
     * 
     */
    RoundPhase mRoundPhase;

    /**
     * @brief The part of the turn the game is currently in.
     * 
     */
    TurnPhase mTurnPhase;

    /**
     * @brief The player whose turn it is, presently.
     * 
     */
    PlayerID mTurn;

    /**
     * @brief The role, black or white, of the winner of the game (when the game has ended).
     * 
     */
    RoleID mWinner;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried for scores.
 * 
 */
struct GameScoreData {
    /**
     * @brief The number of counters available to be won by the player claiming victory.
     * 
     */
    uint8_t mCommonPoolCounters;

    /**
     * @brief The number of counters held by player A.
     * 
     * @todo Make naming of Player A and B consistent.
     * 
     */
    uint8_t mPlayerOneCounters;

    /**
     * @brief The number of counters currently held by player B.
     * 
     * @todo Make naming of Player A and B consistent.
     * 
     */
    uint8_t mPlayerTwoCounters;

    /**
     * @brief The number of Player A's pieces that have reached the end of the route.
     * 
     */
    uint8_t mPlayerOneVictoryPieces;

    /**
     * @brief The number of Player B's pieces that have reached the end of the route.
     * 
     */
    uint8_t mPlayerTwoVictoryPieces;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried for dice related information.
 * 
 */
struct DiceData {
    /**
     * @brief The state of the dice.
     * 
     */
    Dice::State mState;

    /**
     * @brief The value shown by the primary die.
     * 
     */
    uint8_t mPrimaryRoll;

    /**
     * @brief The value shown by the secondary die.
     * 
     */
    bool mSecondaryRoll;

    /**
     * @brief The result score the two die together produce in the current game phase.
     * 
     */
    uint8_t mResultScore;

    /**
     * @brief The result dice score from the previous turn.
     * 
     */
    uint8_t mPreviousResult;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by GameOfUrModel when queried about a player.
 * 
 */
struct PlayerData {
    /**
     * @brief The player (A or B) this data is about.
     * 
     */
    PlayerID mPlayer;

    /**
     * @brief The piece set this player is using this game (black or white).
     * 
     */
    RoleID mRole;

    /**
     * @brief Whether this player is the winner of the game.
     * 
     */
    bool mIsWinner;

    /**
     * @brief The number of counters currently held by this player.
     * 
     */
    uint8_t mCounters;

    /**
     * @brief The number of pieces this player hasn't yet launched.
     * 
     */
    uint8_t mNUnlaunchedPieces;

    /**
     * @brief The number of this player's pieces on the board.
     * 
     */
    uint8_t mNBoardPieces;

    /**
     * @brief The number of this player's pieces which have completed the route.
     * 
     */
    uint8_t mNVictoryPieces;
};

/**
 * @ingroup UrGameDataModel
 * @brief Data returned by the GameOfUrModel when making a move, or querying possible moves.
 * 
 */
struct MoveResultData {
    /**
     * @brief The underlying type of the variable in which move related flags will be stored.
     * 
     */
    using flags=uint8_t;

    /**
     * @brief Enum values corresponding to masks used on flags to answer questions about the move represented by MoveResultData.
     * 
     */
    enum Flags: flags {
        IS_POSSIBLE=0x1, //< Whether this move can even happen.
        PASSES_ROSETTE=0x2, //< Whether, on making this move, a rosette is passed.
        LANDS_ON_ROSETTE=0x4, //< Whether at the end of the move, the moved piece is on a rosette.
        COMPLETES_ROUTE=0x8, //< Whether this move results in the piece completing the route.
        ENDS_GAME=0x10, //< Whether this move will end the game (will co-occur with Flags::COMPLETES_ROUTE).
    };

    /**
     * @brief Flags answering certain questions about the effects of making this move.
     * 
     */
    flags mFlags;

    /**
     * @brief Data relating to the piece knocked out of the board, if any.
     * 
     */
    GamePieceData mDisplacedPiece;

    /**
     * @brief Data relating to the piece being moved.
     * 
     */
    GamePieceData mMovedPiece;

    /**
     * @brief The number of counters the maker of the move will gain when it is performed.
     * 
     */
    uint8_t mCountersWon;

    /**
     * @brief The number of counters the maker of the move will lose when it is performed.
     * 
     */
    uint8_t mCountersLost;
};

/**
 * @ingroup UrGameDataModel
 * @brief The data model representing one instance of Game of Ur.
 * 
 * Maintains information and references to the game board, its pieces, its players, the counters possessed by each player and held in common.  Provides methods for advancing the game and querying its current state.
 * 
 * An instance of this object should be, at all times, treated as the single source of truth about the game.
 * 
 */
class GameOfUrModel {
public:
    /**
     * @brief Constructs a new game of ur model.
     * 
     */
    GameOfUrModel()=default;

    /**
     * @brief Constructs a new game of ur model by stealing resources from another instance.
     * 
     * @param other The model being stolen from.
     */
    GameOfUrModel(GameOfUrModel&& other)=default;

    /**
     * @brief Replaces resources owned by this model with those stolen from another instance.
     * 
     * @param other The model being stolen from.
     * @return GameOfUrModel& An instance to this object, after it has taken possession of other's resources.
     */
    inline GameOfUrModel& operator=(GameOfUrModel&& other);

    GameOfUrModel(const GameOfUrModel& other)=delete;
    GameOfUrModel& operator=(const GameOfUrModel& other)=delete;

    /**
     * @brief Resets the game to its initial state, losing track of its current one.
     * 
     */
    void reset();

    /**
     * @brief Starts the play phase of the game, where counters are placed into the common pile, roles are assigned to each player, and the turn for the player playing black is begun.
     * 
     */
    void startPhasePlay();

    /**
     * @brief Rolls dice on behalf of the requester.
     * 
     * @warning This method assumes that the dice roll has already been validated before it was called.  It will throw an error if rolling dice is not possible currently.
     * 
     * @param requester The player attempting to roll the dice.
     */
    void rollDice(PlayerID requester);

    /**
     * @brief Moves a piece from its current state or location to the one requested.
     * 
     * @warning This method assumes the move was already validated, and will throw an error if it is invalid.
     * 
     * @param piece The identity of the piece being moved.
     * @param toLocation The location (on the board or at the end of the route) the piece is to be moved to.
     * @param requester The player requesting to move the piece.
     */
    void movePiece(PieceIdentity piece, glm::u8vec2 toLocation, PlayerID requester);

    /**
     * @brief Advances the game by one turn, usually starting the turn of the next player.
     * 
     * @warning This method assumes that the fact that the turn can be advanced has already been verified, and will throw an error if it can't.
     * 
     * @param requester The player requesting a move to the next turn.
     */
    void advanceOneTurn(PlayerID requester);

    /**
     * @brief Gets the types of the pieces that this player hasn't yet launched.
     * 
     * @warning This method assumes that roles have already been assigned and that the game is no longer in the initiative phase, and will throw an exception if it isn't.
     * 
     * @param player The player whose unlaunched piece types we want to know.
     * @return std::vector<PieceTypeID> The list of this player's unlaunched pieces.
     */
    std::vector<PieceTypeID> getUnlaunchedPieceTypes(PlayerID player) const;

    /**
     * @brief Gets a list of every possible move that can be made given the game's current state.
     * 
     * @return std::vector<std::pair<PieceIdentity, glm::u8vec2>> A list of pieces that may be moved along with the locations they may be moved to.
     */
    std::vector<std::pair<PieceIdentity, glm::u8vec2>> getAllPossibleMoves() const;

    /**
     * @brief Gets a list of positions a particular piece may be launched to.
     * 
     * @warning Throws an exception if PieceIdentity::mOwner is RoleID::NA.
     * 
     * @param pieceIdentity The identity of the piece whose launch positions we want.
     * @return std::vector<glm::u8vec2> A list of board positions this piece may be launched to.
     */
    std::vector<glm::u8vec2> getLaunchPositions(const PieceIdentity& pieceIdentity) const;

    /**
     * @brief Gets the number of counters held in the common pile.
     * 
     * @return uint8_t The number of counters in the common pile.
     */
    uint8_t getNCounters() const { return mCounters; }

    /**
     * @brief Gets a description of the current phase of the game.
     * 
     * @return GamePhaseData Data describing the current phase of the game.
     */
    GamePhaseData getCurrentPhase() const;
    
    /**
     * @brief Gets the current scores for the game.
     * 
     * @return GameScoreData The current scores for the game.
     */
    GameScoreData getScore() const;

    /**
     * @brief Gets the data relating to the house present at a given location.
     * 
     * @param location The location of the house.
     * @return HouseData Data describing the house.
     */
    HouseData getHouseData(glm::u8vec2 location) const;

    /**
     * @brief Gets information about a particular piece.
     * 
     * @param gamePiece The piece whose description we want.
     * @return GamePieceData The data describing the piece.
     */
    GamePieceData getPieceData(PieceIdentity gamePiece) const;

    /**
     * @brief Gets information about a particular piece.
     * 
     * @param player The player owning the piece.
     * @param pieceType The type of the piece whose description we want.
     * @return GamePieceData The data describing the piece.
     */
    GamePieceData getPieceData(PlayerID player, PieceTypeID pieceType) const;

    /**
     * @brief Gets information about a player.
     * 
     * @param player The ID corresponding to a player of the game.
     * @return PlayerData Information about the player.
     */
    PlayerData getPlayerData(PlayerID player) const;

    /**
     * @brief Gets information about a player.
     * 
     * @warning Will throw an error if RoleID::NA is passed.
     * 
     * @param player The role assigned to the player whose information we want.
     * @return PlayerData Information about the player.
     */
    PlayerData getPlayerData(RoleID player) const;

    /**
     * @brief Gets information about the player whose turn it currently is.
     * 
     * @return PlayerData Information about this turn's player.
     */
    PlayerData getCurrentPlayer() const;

    /**
     * @brief Gets information about the current state of this game's dice.
     * 
     * @return DiceData Data describing the state of this game's dice.
     */
    DiceData getDiceData() const;

    /**
     * @brief Gets data about the results of making a move with the current dice roll with a piece present on the board.
     * 
     * @param piece The piece being enquired about.
     * @return MoveResultData The result of moving the piece (including whether such a move is even possible with the current roll).
     */
    MoveResultData getBoardMoveData(PieceIdentity piece) const;

    /**
     * @brief Gets data about the results of launching a piece that hasn't yet been launched with the current dice roll.
     * 
     * @param piece The piece we want to launch.
     * @param launchLocation The location to which we want to launch the piece.
     * @return MoveResultData The result of launching the piece (including whether such a move is even possible presently).
     */
    MoveResultData getLaunchMoveData(PieceIdentity piece, glm::u8vec2 launchLocation) const;

    /**
     * @brief Tests whether the game dice can be rolled now.
     * 
     * @param requester The player wanting to roll the dice.
     * @retval true The dice may be rolled.
     * @retval false The dice can't be rolled.
     */
    bool canRollDice(PlayerID requester) const;

    /**
     * @brief Tests whether a piece can be launched to a particular location.
     * 
     * @param pieceIdentity The identity of the piece being launched.
     * @param toLocation The location to which the piece should be launched.
     * @param requester The player requesting the launch of the piece.
     * @retval true Launching the piece is possible.
     * @retval false Launching the piece is not possible.
     */
    bool canLaunchPieceTo(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const;

    /**
     * @brief Tests whether a particular piece can be launched (at all).
     * 
     * @param pieceIdentity The identity of the piece being launched.
     * @param requester The player requesting the launch.
     * @retval true Launching the piece is possible.
     * @retval false Launching the piece is impossible.
     */
    bool canLaunchPiece(PieceIdentity pieceIdentity, PlayerID requester) const;

    /**
     * @brief Tests whether a piece already on the board can be moved to a new location.
     * 
     * @param pieceIdentity The identity of the piece being moved.
     * @param requester The player requesting the move.
     * @retval true The piece may be moved.
     * @retval false The piece can't be moved.
     */
    bool canMoveBoardPiece(PieceIdentity pieceIdentity, PlayerID requester) const;

    /**
     * @brief Tests whether the end of the current turn has been reached, and that the next one can begin.
     * 
     * @param requester The player requesting the end of the turn.
     * @retval true The game may be advanced by a turn.
     * @retval false The game cannot move to the next turn.
     */
    bool canAdvanceOneTurn(PlayerID requester) const;

    /**
     * @brief Tests whether the end of the initiative phase has been reached, and whether the play phase may now begin.
     * 
     * @retval true The play phase of the game can begin.
     * @retval false The play phase of the game can't begin.
     */
    bool canStartPhasePlay() const;

private:
    /**
     * @brief The underlying implementation for canLaunchPiece(), canLaunchPieceTo(), and canMoveBoardPiece().
     * 
     * @param pieceIdentity The identity of the piece being moved.
     * @param toLocation The location to which the player wants the piece to move.
     * @param requester The player making the request for the move.
     * @retval true The piece can be moved to the house at the indicated location.
     * @retval false The piece can't be moved to the house at the indicated location.
     * 
     */
    bool canMovePiece(PieceIdentity pieceIdentity, glm::u8vec2 toLocation, PlayerID requester) const;

    /**
     * @brief Gets the role of the winner of the game, and NA if there isn't one yet.
     * 
     * @return RoleID The role (black or white) of the winner of the game.
     */
    RoleID getWinner() const;

    /**
     * @brief Gets the role assigned to a particular player.
     * 
     * @param player The player whose role is being queried.
     * @return RoleID The role assigned to this player.
     */
    RoleID getRole(PlayerID player) const;

    /**
     * @brief Gets the ID of the player to whom a particular role is assigned.
     * 
     * @param role The role of the player.
     * @return PlayerID The ID of the player to whom the role was assigned.
     */
    PlayerID getPlayer(RoleID role) const;

    /**
     * @brief Gets a reference to (the sole instance of) a particular piece.
     * 
     * @param pieceIdentity The identity of the piece whose state we want to know.
     * @return const Piece& The reference to the piece.
     */
    const Piece& getPiece(const PieceIdentity& pieceIdentity) const;

    /**
     * @brief Gets data about a move which may be made with some piece (including whether a move is even possible).
     * 
     * @param piece The piece we want to make a move with.
     * @param moveLocation The location to which the piece should be moved.
     * @return MoveResultData The result of moving the piece to the specified location.
     */
    MoveResultData getMoveData(PieceIdentity piece, glm::u8vec2 moveLocation) const;

    /**
     * @brief Moves some number of counters from a player into the common pile.
     * 
     * @param counters The number of counters to be drawn and placed in the common pile.
     * @param fromPlayer The player from whom the counters are removed.
     */
    void deductCounters(uint8_t counters, PlayerID fromPlayer);

    /**
     * @brief Moves some number of counters from the common pile into a player's possession.
     * 
     * @param counters The number of counters to be paid to the player.
     * @param toPlayer The player to whom the counters are paid.
     */
    void payCounters(uint8_t counters, PlayerID toPlayer);

    /**
     * @brief Ends the current turn.
     * 
     */
    void endTurn();

    /**
     * @brief The current phase of the game as a whole.
     * 
     */
    GamePhase mGamePhase { GamePhase::INITIATIVE };

    /**
     * @brief The current phase of the turn the game is in.
     * 
     */
    TurnPhase mTurnPhase { TurnPhase::ROLL_DICE };

    /**
     * @brief The current phase of the round (every pair of turns).
     * 
     */
    RoundPhase mRoundPhase { RoundPhase::IN_PROGRESS };

    /**
     * @brief The player appointed to take action during this turn.
     * 
     */
    PlayerID mCurrentPlayer { PLAYER_A };

    /**
     * @brief The number of counters held in the game's common pile.
     * 
     */
    uint8_t mCounters { 0 };

    /**
     * @brief A reference to the data model of the dice used in this game.
     * 
     */
    std::unique_ptr<Dice> mDice { std::make_unique<Dice>() };

    /**
     * @brief The data model for the board used by this game.
     * 
     */
    Board mBoard {};

    /**
     * @brief Data models for the two players of the game.
     * 
     */
    std::array<Player, 2> mPlayers {};

    /**
     * @brief The final result of the dice roll in the previous turn of this game.
     * 
     */
    uint8_t mPreviousRoll {0};
};

inline GameOfUrModel& GameOfUrModel::operator=(GameOfUrModel&& other)=default;
#endif
