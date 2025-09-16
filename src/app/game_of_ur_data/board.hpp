/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/board.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains the data model class representing the 20-square board for the Royal Game of Ur.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPBOARD_H
#define ZOAPPBOARD_H

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "house.hpp"

/**
 * @ingroup UrGameDataModel
 * @brief The data model class representing the 20-square board for the Royal Game of Ur.
 * 
 * Implements various methods for querying the state of the board, eg., the region a particular tile (house) on the board belongs to, any piece currently occupying a tile, whether a move on the board is possible, and so on.
 * 
 */
class Board {
public:

    /**
     * @brief Applies a move after validating it.
     * 
     * @param role The role (black or white) initiating the move.
     * @param gamePiece The game piece to be moved.
     * @param toLocation The target location for the game piece.
     * @param roll The dice score.
     * 
     * @return std::weak_ptr<Piece> A reference to the piece moved.
     * 
     * @see canMove()
     */
    std::weak_ptr<Piece> move(RoleID role, std::weak_ptr<Piece> gamePiece, glm::u8vec2 toLocation, uint8_t roll);

    /**
     * @brief Fetches a reference to the piece currently occupying a house pointed to by `location`.
     * 
     * @param location The location of the house whose occupant we want to get.
     * @return std::weak_ptr<Piece> A reference to the occupying game piece, if any.
     */
    std::weak_ptr<Piece> getOccupantReference(glm::u8vec2 location);

    /**
     * @brief Tests whether a given move is possible, per the state of the board.
     * 
     * The following conditions are checked in order to determine whether a given move is possible:
     * 
     * - Whether the destination house is unoccupied, or if it is occupied, if it is a non-rosette house occupied by the opponent's piece.
     * 
     * - Whether the game piece belongs to the role responsible for initiating the move, and whether the game piece is in a valid state to make a move (i.e., not a piece that has already completed the route).
     * 
     * - Whether the move matches the dice score provided (for unlaunched pieces, launch score; for board pieces, number of steps forward to destination house from current house).
     * 
     * @param role The role (black or white) initiating the move.
     * @param gamePiece A reference to the piece being moved.
     * @param toLocation The destination house to which the piece will be moved.
     * @param roll The score on the dice being used to make the move.
     * 
     * @retval true The move is possible per the current state of the board.
     * @retval false The move is not possible per the current state of the board.
     */
    bool canMove(RoleID role, const Piece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const;

    /**
     * @brief Tests whether a move, if made, will pass over a rosette without landing on it.
     * 
     * It determines this by:
     * 
     * - Checking the status of the piece.  Pieces not on the board fail the test.
     * - Stepping through the houses the piece will pass over.  If one is a rosette house and NOT the destination house, the condition succeeds.
     * 
     * @param gamePiece The piece being moved.
     * @param toLocation The location of the destination house.
     * 
     * @retval true The move will cause a piece to pass over a rosette house.
     * @retval false The move won't make the piece pass over a rosette.
     */
    bool movePassesRosette(const Piece& gamePiece, glm::u8vec2 toLocation) const;

    /**
     * @brief Tests whether a move will cause an opponent piece to be knocked off the board.
     * 
     * This is done by determining:
     * 
     * - Whether the move is a valid one.
     * 
     * - Whether the destination is valid, but occupied.
     * 
     * @param role The role (black or white) initiating this move.
     * @param gamePiece The game piece being moved.
     * @param toLocation The board location of the destination house.
     * @param roll The dice roll being used to make the move.
     * 
     * @retval true This move is valid and will displace an opponent's piece.
     * @retval false This move is not valid, or won't displace an opponent's piece.
     */
    bool moveDisplacesOpponent(RoleID role, const Piece& gamePiece, glm::u8vec2 toLocation, uint8_t roll) const;

    /**
     * @brief Gets the type of the house at a board location.
     * 
     * @param location The location whose house type is being retrieved.
     * @return House::Type The type of house at the board location queried.
     */
    House::Type getType(glm::u8vec2 location) const;

    /**
     * @brief Gets the region of the house at a board location.
     * 
     * @param location The location whose house type is being retrieved.
     * @return House::Region The region of the house at the queried board location.
     */
    House::Region getRegion(glm::u8vec2 location) const;

    /**
     * @brief Gets the identity of the piece occupying the house at a given board location.
     * 
     * @param location The board location whose house's occupant is being retrieved.
     * @return PieceIdentity The identity of the occupant.
     */
    PieceIdentity getOccupant(glm::u8vec2 location) const;

    /**
     * @brief Gets the 2-component vector denoting the step to the next house in the game route, relative to the house at `location`.
     * 
     * @param location The location of the house whose "next" vector is being retrieved.
     * @return glm::i8vec2 The value added to `location` for the location of the next house on the route.
     */
    glm::i8vec2 getNextCellDirection(glm::u8vec2 location) const;

    /**
     * @brief Gets a list of launch positions available to different pieces.
     * 
     * Most pieces have a launch position corresponding exactly to their launch rolls, i.e., houses V, VI, VII, X relative to each RoleID's start location.
     * 
     * The Swallow may launch in the house just before any house with a Rosette on it.
     * 
     * @param pieceIdentity The identity of the piece being launched.
     * @return std::vector<glm::u8vec2> A list of valid launch locations for the piece.
     */
    std::vector<glm::u8vec2> getLaunchPositions(PieceIdentity pieceIdentity) const;

    /**
     * @brief Tests whether the house at a given location is occupied.
     * 
     * @param location The location of the house being tested.
     * @retval true The house at this location is occupied.
     * @retval false The house at this location is not occupied.
     */
    bool houseIsOccupied(glm::u8vec2 location) const;

    /**
     * @brief Tests whether a location corresponds to a real house on the game board.
     * 
     * @param location The location being tested.
     * @retval true The location corresponds to a real house.
     * @retval false The location does not correspond to a real house.
     */
    bool isValidHouse(glm::u8vec2 location) const;

    /**
     * @brief Tests whether the house corresponding to a location is a valid launch house for a given piece.
     * 
     * @param location The location being tested.
     * @param gamePiece The game piece to be launched.
     * @retval true This house is a valid launch house for this piece.
     * @retval false This house is not a valid launch house for this piece.
     */
    bool isValidLaunchHouse(glm::u8vec2 location, const Piece& gamePiece) const;

    /**
     * @brief Tests whether the house at a given location is a Rosette house.
     * 
     * @param location The location of the house.
     * @retval true The house at this location is a rosette house.
     * @retval false The house at this location is not a rosette house.
     */
    bool isRosette(glm::u8vec2 location) const;

    /**
     * @brief Tests whether this location corresponds with what this board considers the end of the route (i.e., one past the last house in the route).
     * 
     * @param location The location being tested.
     * @retval true This location is the end of the route.
     * @retval false This location is not the end of the route.
     */
    bool isRouteEnd(glm::u8vec2 location) const;

    /**
     * @brief Given a game piece present on the board and a dice roll, computes the new board location for that piece when the roll is used to make it move.
     * 
     * @param gamePiece The piece present on the board potentially being made to move.
     * @param roll The roll with which to make the piece move.
     * @return glm::u8vec2 The destination location corresponding to the roll.
     */
    glm::u8vec2 computeMoveLocation(const Piece& gamePiece, uint8_t roll) const;
private:

    /**
     * @brief A list of vectors, where each element in a vector corresponds to one house on the board.
     * 
     */
    std::array<std::vector<House>, 3> mGrid {{
        {
            House{{1, 0}, House::ROSETTE, House::BLACK},
            House{{0, -1}, House::REGULAR, House::BLACK},
            House{{0, -1}, House::REGULAR, House::BLACK},
            House{{0, -1}, House::REGULAR, House::BLACK},
        },
        {
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::REGULAR, House::BATTLEFIELD},
            House{{0, 1}, House::ROSETTE, House::BATTLEFIELD},
        },
        {
            House{{-1, 0}, House::ROSETTE, House::WHITE},
            House{{0, -1}, House::REGULAR, House::WHITE},
            House{{0, -1}, House::REGULAR, House::WHITE},
            House{{0, -1}, House::REGULAR, House::WHITE},
        },
    }};

    /**
     * @brief Given a (non-swallow) piece type, gets the location of its launch house.
     * 
     * @param pieceType The type of the piece whose launch position is being retrieved.
     * @return glm::u8vec2 The piece's launch position.
     */
    glm::u8vec2 getLaunchPosition(PieceTypeID pieceType) const;
};

#endif
