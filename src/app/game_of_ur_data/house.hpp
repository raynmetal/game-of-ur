#ifndef ZOAPPHOUSE_H
#define ZOAPPHOUSE_H

#include <memory>

#include <glm/glm.hpp>

#include "piece.hpp"


class House {
public:
    enum Type: uint8_t {
        REGULAR,
        ROSETTE,
    };
    enum Region: uint8_t {
        BATTLEFIELD,
        ONE=RoleID::ONE,
        TWO=RoleID::TWO,
    };
    House(glm::i8vec2 nextCell, Type houseType, Region region):
    mNextCell { nextCell },
    mType { houseType },
    mRegion { region }
    {}

    void move(std::shared_ptr<Piece> gamePiece);
    std::weak_ptr<Piece> getOccupantReference() { return mOccupant; }
    PieceIdentity getOccupant() const;

    bool isOccupied() const { return !mOccupant.expired(); }
    bool isRosette() const { return mType == Type::ROSETTE; };

    Region getRegion() const { return mRegion; }
    Type getType() const { return mType; }
    glm::i8vec2 getNextCellDirection() const { return mNextCell; }

    bool canMove(const Piece& gamePiece) const;

private:
    glm::i8vec2 mNextCell;
    Type mType;
    Region mRegion;
    std::weak_ptr<Piece> mOccupant {};
};

#endif
