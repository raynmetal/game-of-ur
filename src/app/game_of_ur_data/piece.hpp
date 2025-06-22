#ifndef ZOAPPGAMEPIECES_H
#define ZOAPPGAMEPIECES_H

#include <string>

#include <glm/glm.hpp>

#include "role_id.hpp"
#include "piece_type_id.hpp"

struct PieceIdentity {
    PieceTypeID mType;
    RoleID mOwner;
};

class Piece {
public:
    enum class State: uint8_t {
        UNLAUNCHED,
        ON_BOARD,
        FINISHED,
    };

    Piece(PieceTypeID type, RoleID owner):
    mIdentity{ .mType { type }, .mOwner { owner } }
    {}

    RoleID getOwner() const { return mIdentity.mOwner; }
    PieceTypeID getType() const { return mIdentity.mType; }
    PieceIdentity getIdentity() const { return mIdentity; }
    State getState() const { return mState; }
    glm::u8vec2 getLocation() const { return mLocation; }

    void setState(State state) { mState = state; }
    void setLocation(glm::u8vec2 location) { mLocation = location; }

    bool canMove(uint8_t roll, RoleID player) const;

private:
    State mState { State::UNLAUNCHED };
    PieceIdentity mIdentity;
    glm::u8vec2 mLocation {0, 0};
};

#endif
