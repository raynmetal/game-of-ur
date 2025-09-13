/**
 * @file game_of_ur_data/piece_type_id.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains enum listing the different types of pieces present in the game.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPGAMEPIECETYPEID_H
#define ZOAPPGAMEPIECETYPEID_H

#include <cstdint>

/**
 * @brief Enum listing the different types of pieces present in the game.
 * 
 */
enum PieceTypeID: uint8_t {
    SWALLOW=0,
    STORMBIRD,
    RAVEN,
    ROOSTER,
    EAGLE,
    TOTAL=5
};

#endif
