#ifndef ZOAPPPLAYER_H
#define ZOAPPPLAYER_H

#include <memory>
#include <array>

#include <glm/glm.hpp>

#include "game_piece.hpp"
#include "player_role_id.hpp"

class Player {
public:
    void initializeRole(PlayerRoleID role);

    uint8_t deductCounters(uint8_t counters);
    void depositCounters(uint8_t counters);

    uint8_t getNPieces(GamePiece::State inState) const;
    inline uint8_t getNCounters() const { return mCounters; }
    inline PlayerRoleID getRole() const { return mRole; }

    std::weak_ptr<GamePiece> getPiece(GamePieceTypeID pieceType);
    const GamePiece& cGetPiece(GamePieceTypeID pieceType) const;

private:
    PlayerRoleID mRole { NA };
    std::array<std::shared_ptr<GamePiece>, 5> mPieces { nullptr };
    uint8_t mCounters { 25 };
};

#endif
