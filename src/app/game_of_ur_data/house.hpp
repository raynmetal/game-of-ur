/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/house.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains classes and enums that define a house and describe its state.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPHOUSE_H
#define ZOAPPHOUSE_H

#include <memory>

#include <glm/glm.hpp>

#include "piece.hpp"


/**
 * @ingroup UrGameDataModel
 * @brief The representation of a single house on the Game of Ur Board, in other words a tile.
 * 
 * Also stores a reference to the game piece presently occupying this house.
 * 
 */
class House {
public:
    /**
     * @brief The type of a house.
     * 
     */
    enum Type: uint8_t {
        REGULAR, //< A regular house, with no special rules.
        ROSETTE, //< A rosette house, granting its occupant special immunity, and deciding whether counters are won, lost, or neither, by the active player during GamePhase::PLAY.
    };

    /**
     * @brief The region this house belongs to.
     * 
     */
    enum Region: uint8_t {
        BATTLEFIELD, //< A tile that can be occupied by either player, and making up the main part of the route they compete over.
        BLACK=RoleID::BLACK, //< A small region only usable by the player playing black.
        WHITE=RoleID::WHITE, //< A small region only usable by the player playing white.
    };

    /**
     * @brief Constructs a new House.
     * 
     * @param nextCell The direction to the next House relative to this one.
     * @param houseType The type of house this is.
     * @param region The region in which this house is present.
     */
    House(glm::i8vec2 nextCell, Type houseType, Region region):
    mNextCell { nextCell },
    mType { houseType },
    mRegion { region }
    {}

    /**
     * @brief Validates and applies a move, replacing the reference to the currently occupying piece with a new one.  
     * 
     * Passing an empty pointer causes this house to lose its reference to its current occupant.
     * 
     * @warning This method assumes that any higher level game logic checks were already performed before it was called.
     * 
     * @param gamePiece The piece to now occupy this, or a nullptr if the piece here is to be moved.
     */
    void move(std::shared_ptr<Piece> gamePiece);

    /**
     * @brief Gets a reference to the piece already occupying this house, if any.
     * 
     * @return std::weak_ptr<Piece> A reference to the piece occupying this house.
     */
    std::weak_ptr<Piece> getOccupantReference() { return mOccupant; }

    /**
     * @brief Gets the identity of the piece occupying this house.
     * 
     * @return PieceIdentity The ID of the piece occupying this house.
     */
    PieceIdentity getOccupant() const;

    /**
     * @brief Tests whether this house is already occupied by a game piece.
     * 
     * @retval true There is a game piece on this house.
     * @retval false There is no game piece on this house.
     */
    bool isOccupied() const { return !mOccupant.expired(); }

    /**
     * @brief Tests whether this house is a rosette house.
     * 
     * @retval true This house is a rosette house.
     * @retval false This house is not a rosette house. 
     */
    bool isRosette() const { return mType == Type::ROSETTE; };

    /**
     * @brief Gets the region this house belongs to.
     * 
     * @return Region The region this house belongs to.
     */
    Region getRegion() const { return mRegion; }

    /**
     * @brief Gets which type of house this house is.
     * 
     * @return Type This house's type.
     */
    Type getType() const { return mType; }

    /**
     * @brief Gets an integral vector pointing to the location of the next House, relative to this one.
     * 
     * @return glm::i8vec2 A vector pointing in the direction of the next house from this one.
     */
    glm::i8vec2 getNextCellDirection() const { return mNextCell; }

    /**
     * @brief Tests whether a given piece can move into this house.
     * 
     * The conditions determining this are as follows:
     * 
     * - This house must be a region accessible by the piece's owner (i.e., their own region or the battlefield)
     * 
     * - This house must be unoccupied.
     * 
     * - If this house is occupied, then it must be a non-rosette house occupied by a piece belonging to an opponent.
     * 
     * In all other cases, the house can't be occupied by this piece.
     * 
     * @param gamePiece The piece trying to occupy this house.
     * 
     * @retval true This piece may move into this house.
     * @retval false This piece can't move into this house.
     * 
     * @todo We shouldn't need Piece here, just PieceIdentity should do.
     * 
     */
    bool canMove(const Piece& gamePiece) const;

private:
    /**
     * @brief Direction to the location of the next House, relative to this one.
     * 
     */
    glm::i8vec2 mNextCell;

    /**
     * @brief The type of House this one is.
     * 
     */
    Type mType;

    /**
     * @brief The region in which this House lies.
     * 
     */
    Region mRegion;

    /**
     * @brief The piece occupying this house, if any.
     * 
     */
    std::weak_ptr<Piece> mOccupant {};
};

#endif
