/**
 * @file game_of_ur_data/piece.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains definitions for pieces belonging to different sets used by competing players.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPGAMEPIECES_H
#define ZOAPPGAMEPIECES_H

#include <string>

#include <glm/glm.hpp>

#include "role_id.hpp"
#include "piece_type_id.hpp"

/**
 * @brief Data uniquely identifying a piece used in the game.
 * 
 * If PieceIdentity::mOwner has a value of RoleID::NA, this PieceIdentity should be treated as null.
 * 
 */
struct PieceIdentity {
    /**
     * @brief The type of the piece.
     * 
     */
    PieceTypeID mType;

    /**
     * @brief The set to which this piece belongs.
     * 
     */
    RoleID mOwner;
};

bool operator<(const PieceIdentity& one, const PieceIdentity& two);
bool operator==(const PieceIdentity& one, const PieceIdentity& two);
bool operator!=(const PieceIdentity& one, const PieceIdentity& two);

/**
 * @brief The state of a single piece of the game.
 * 
 */
class Piece {
public:
    /**
     * @brief A value representing the (high level) state of this piece.
     * 
     */
    enum class State: uint8_t {
        UNLAUNCHED, //< This piece hasn't been launched to the board yet.
        ON_BOARD, //< This piece is currently on the board.
        FINISHED, //< This piece has completed its route.
    };

    /**
     * @brief Constructs a new Piece.
     * 
     * @param type The type of piece represented by this object.
     * @param owner The set to which this piece belongs (black or white).
     */
    Piece(PieceTypeID type, RoleID owner):
    mIdentity{ .mType { type }, .mOwner { owner } }
    {}

    /**
     * @brief Gets the owner of this piece, the set to which this piece belongs.
     * 
     * @return RoleID The owner of this piece or the set to which it belongs.
     */
    RoleID getOwner() const { return mIdentity.mOwner; }

    /**
     * @brief Gets the type of piece this piece is.
     * 
     * @return PieceTypeID This piece's type.
     */
    PieceTypeID getType() const { return mIdentity.mType; }

    /**
     * @brief Gets the identity of this piece, both its type and owner.
     * 
     * @return PieceIdentity The identity of this piece.
     */
    PieceIdentity getIdentity() const { return mIdentity; }

    /**
     * @brief Gets the (high level) state of this piece.
     * 
     * @return State This piece's state.
     */
    State getState() const { return mState; }

    /**
     * @brief Gets the current location of this piece on the board.
     * 
     * @return glm::u8vec2 The location of this piece on the board.
     */
    glm::u8vec2 getLocation() const { return mLocation; }

    /**
     * @brief Sets the (high level) state of this piece.
     * 
     * @param state This piece's new state.
     */
    void setState(State state) { mState = state; }

    /**
     * @brief Sets the location of this piece on the board.
     * 
     * @param location This piece's new location.
     */
    void setLocation(glm::u8vec2 location) { mLocation = location; }

    /**
     * @brief Tests whether this piece can be moved at all.
     * 
     * For this piece to report that it can move, it should either be on the board or unlaunched, and if unlaunched the roll must correspond to this piece type's launch roll.
     * 
     * @param roll The roll with which we'd like to move this piece.
     * @param player The player requesting the move.
     * @retval true This piece may move.
     * @retval false This piece can't move.
     */
    bool canMove(uint8_t roll, RoleID player) const;

private:
    /**
     * @brief The current (high level) state of this piece.
     * 
     */
    State mState { State::UNLAUNCHED };

    /**
     * @brief The unique identity of this piece.
     * 
     */
    PieceIdentity mIdentity;

    /**
     * @brief This piece's current location with respect to the game board.
     * 
     */
    glm::u8vec2 mLocation {0, 0};
};

#endif
