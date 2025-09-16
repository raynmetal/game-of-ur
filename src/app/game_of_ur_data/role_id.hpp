/**
 * @ingroup UrGameDataModel
 * @file game_of_ur_data/role_id.hpp
 * @author Zoheb Shujauddin (zoheb2424@gmail.com)
 * @brief Contains an enum for the roles (sets of pieces) possible within this game.
 * @version 0.3.2
 * @date 2025-09-12
 * 
 * 
 */

#ifndef ZOAPPPLAYERROLEID_H
#define ZOAPPPLAYERROLEID_H

/**
 * @ingroup UrGameDataModel
 * @brief A value representing the various roles (or sets, if preferred) possible in this game.
 * 
 */
enum RoleID {
    NA, //< Role assignment hasn't been done yet, or the data containing this represents some object considered "empty."
    BLACK, //< The role (and the corresponding set of pieces) whose player goes first in the play phase of the game.
    WHITE, //< The role (and the corresponding set of pieces) whose player goes second in the play phase of the game.
};

#endif
