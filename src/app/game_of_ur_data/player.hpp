#ifndef ZOAPPPLAYER_H
#define ZOAPPPLAYER_H

#include <memory>
#include <array>

#include <glm/glm.hpp>

#include "piece.hpp"
#include "role_id.hpp"

class Player {
public:
    void initializeWithRole(RoleID role);

    uint8_t deductCounters(uint8_t counters);
    void depositCounters(uint8_t counters);

    uint8_t getNPieces(Piece::State inState) const;
    inline uint8_t getNCounters() const { return mCounters; }
    inline RoleID getRole() const { return mRole; }

    std::weak_ptr<Piece> getPiece(PieceTypeID pieceType);
    const Piece& cGetPiece(PieceTypeID pieceType) const;

private:
    RoleID mRole { NA };
    std::array<std::shared_ptr<Piece>, 5> mPieces { nullptr };
    uint8_t mCounters { 25 };
};

#endif
