#ifndef ZOAPPGAMEPHASE_H
#define ZOAPPGAMEPHASE_H

enum class GamePhase {
    INITIATIVE,
    PLAY,
    END,
};

enum class TurnPhase {
    ROLL_DICE,
    MOVE_PIECE,
    END,
};

enum class RoundPhase {
    IN_PROGRESS,
    END,
};

#endif
