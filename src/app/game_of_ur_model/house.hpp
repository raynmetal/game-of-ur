#ifndef ZOAPPHOUSE_H
#define ZOAPPHOUSE_H

#include <memory>

#include <glm/glm.hpp>

#include "game_piece.hpp"


class House {
public:
    enum Type: uint8_t {
        REGULAR,
        ROSETTE,
    };
    enum Region: uint8_t {
        BATTLEFIELD,
        ONE=PlayerRoleID::ONE,
        TWO=PlayerRoleID::TWO,
    };
    House(glm::i8vec2 nextCell, Type houseType, Region region):
    mNextCell { nextCell },
    mType { houseType },
    mRegion { region }
    {}

    void move(std::shared_ptr<GamePiece> gamePiece);
    std::weak_ptr<GamePiece> getOccupantReference() { return mOccupant; }
    GamePieceIdentity getOccupant() const;

    bool isOccupied() const { return !mOccupant.expired(); }
    bool isRosette() const { return mType == Type::ROSETTE; };

    Region getRegion() const { return mRegion; }
    Type getType() const { return mType; }
    glm::i8vec2 getNextCellDirection() const { return mNextCell; }

    bool canMove(const GamePiece& gamePiece) const;

private:
    glm::i8vec2 mNextCell;
    Type mType;
    Region mRegion;
    std::weak_ptr<GamePiece> mOccupant {};
};

#endif
