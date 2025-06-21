#ifndef ZOAPPGAMEPIECES_H
#define ZOAPPGAMEPIECES_H

#include <string>

#include <glm/glm.hpp>

#include "player_role_id.hpp"
#include "game_piece_type_id.hpp"

struct GamePieceIdentity {
    GamePieceTypeID mType;
    PlayerRoleID mOwner;
};

class GamePiece {
public:
    enum class State: uint8_t {
        UNLAUNCHED,
        ON_BOARD,
        FINISHED,
    };

    GamePiece(GamePieceTypeID type, PlayerRoleID owner):
    mIdentity{ .mType { type }, .mOwner { owner } }
    {}

    PlayerRoleID getOwner() const { return mIdentity.mOwner; }
    GamePieceTypeID getType() const { return mIdentity.mType; }
    GamePieceIdentity getIdentity() const { return mIdentity; }
    State getState() const { return mState; }
    glm::u8vec2 getLocation() const { return mLocation; }

    void setState(State state) { mState = state; }
    void setLocation(glm::u8vec2 location) { mLocation = location; }

    bool canMove(uint8_t roll, PlayerRoleID player) const;

private:
    State mState { State::UNLAUNCHED };
    GamePieceIdentity mIdentity;
    glm::u8vec2 mLocation {0, 0};
};

#endif
