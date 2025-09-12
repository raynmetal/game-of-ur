/**
 * @file game_of_ur_data/phase.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Enums whose values represent the different phases a game can be in.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPGAMEPHASE_H
#define ZOAPPGAMEPHASE_H

/**
 * @brief A value representing the high level phase of an entire game.
 * 
 */
enum class GamePhase {
    INITIATIVE, //< A phase in which both players roll both dice to determine role-assignments and turn order.
    PLAY, //< The phase where the bulk of the game takes place, where pieces are move and counters are exchanged.
    END, //< The end of the game, reached when one player succeeds in moving all their pieces to the end of the route.
};

/**
 * @brief A value representing the phases possible in a single turn of the game.
 * 
 */
enum class TurnPhase {
    ROLL_DICE, //< The player must roll the dice now.
    MOVE_PIECE, //< The player may move a piece if possible, or roll the dice again.
    END, //< No moves are possible, and the turn should go to the next player.
};

/**
 * @brief A value representing the phase of a single round, where in a round all players take a turn once.
 * 
 * In the Royal Game of Ur, a round ends every pair of turns.
 */
enum class RoundPhase {
    IN_PROGRESS, //< The round is in progress.
    END, //< Both players have taken a turn, and the round has ended.
};

#endif
