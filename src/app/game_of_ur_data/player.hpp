/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/player.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains structs and classes representing a single player of the game and their current state.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPPLAYER_H
#define ZOAPPPLAYER_H

#include <memory>
#include <array>

#include <glm/glm.hpp>

#include "piece.hpp"
#include "role_id.hpp"

/**
 * @ingroup UrGameDataModel
 * @brief Data model for a single player of the game, tracking also the number of counters held by this player.
 * 
 */
class Player {
public:
    /**
     * @brief Assigns a role (black or white) to this player, and initializes the set of pieces corresponding to that role.
     * 
     * @param role The role (black or white) being assigned to this player.
     */
    void initializeWithRole(RoleID role);

    /**
     * @brief Deducts some quantity of counters from this player's bank.
     * 
     * @param counters The number of counters to be deducted per request.
     * @return uint8_t The number of counters actually deducted.
     */
    uint8_t deductCounters(uint8_t counters);

    /**
     * @brief Deposits some quantity of counters to this player's bank.
     * 
     * @param counters The number of counters to be added to this player's bank.
     */
    void depositCounters(uint8_t counters);

    /**
     * @brief Returns the number of pieces owned by this player in a particular state.
     * 
     * @param inState The state whose game piece count we want.
     * @return uint8_t The number of this player's pieces matching the state.
     */
    uint8_t getNPieces(Piece::State inState) const;

    /**
     * @brief Gets the number of counters currently held by this player.
     * 
     * @return uint8_t The number of counters held by this player.
     */
    inline uint8_t getNCounters() const { return mCounters; }

    /**
     * @brief Gets the role (black or white or neither) currently assigned to this player for this game.
     * 
     * @return RoleID The role assigned to this player.
     */
    inline RoleID getRole() const { return mRole; }

    /**
     * @brief Gets a reference to the data model of some piece owned by this player.
     * 
     * @param pieceType The type of piece whose reference we want.
     * @return std::weak_ptr<Piece> The reference to the requested piece.
     * 
     * @todo Why did I make this a pointer in the first place?  I forget.
     */
    std::weak_ptr<Piece> getPiece(PieceTypeID pieceType);

    /**
     * @brief Gets (a const reference to) a piece owned by this player corresponding to a requested type.
     * 
     * @param pieceType The type of this player's piece we want the data model of.
     * @return const Piece& A const reference to the requested piece.
     */
    const Piece& cGetPiece(PieceTypeID pieceType) const;

private:
    /**
     * @brief The role (black or white) assigned to this player.
     * 
     */
    RoleID mRole { NA };

    /**
     * @brief References to the data models of all the pieces owned by this player.
     * 
     * @todo Once again, why did these need to be pointers?
     * 
     */
    std::array<std::shared_ptr<Piece>, 5> mPieces { nullptr };

    /**
     * @brief The number of counters currently held by this player.
     * 
     */
    uint8_t mCounters { 25 };
};

#endif
