cmake_minimum_required(VERSION 3.23)

project(Game_Of_Ur VERSION 0.3.8)

add_executable(Game_Of_Ur WIN32)

configure_file(src/app/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/app/version.h)

target_sources(
    Game_Of_Ur
    # Ur application sources
    PRIVATE
        src/app/game_of_ur_data/board.cpp
        src/app/game_of_ur_data/dice.cpp
        src/app/game_of_ur_data/house.cpp
        src/app/game_of_ur_data/model.cpp
        src/app/game_of_ur_data/piece.cpp
        src/app/game_of_ur_data/player.cpp
        src/app/game_of_ur_data/serialize.cpp

        src/app/board_locations.cpp
        src/app/ur_controller.cpp
        src/app/ur_look_at_board.cpp
        src/app/ur_player_cpu_random.cpp
        src/app/ur_player_local.cpp
        src/app/ur_records.cpp
        src/app/ur_scene_manager.cpp
        src/app/ur_scene_view.cpp
        src/app/ur_ui_navigation.cpp
        src/app/ur_ui_records_browser.cpp
        src/app/ur_ui_tutorials_browser.cpp
        src/app/ur_ui_version.cpp
        src/app/ur_ui_view.cpp

    PRIVATE
    FILE_SET HEADERS
    BASE_DIRS src/app ${CMAKE_CURRENT_BINARY_DIR}/src/app
    FILES
        # Data Model Headers
        src/app/game_of_ur_data/board.hpp
        src/app/game_of_ur_data/dice.hpp
        src/app/game_of_ur_data/house.hpp
        src/app/game_of_ur_data/model.hpp
        src/app/game_of_ur_data/phase.hpp
        src/app/game_of_ur_data/piece_type_id.hpp
        src/app/game_of_ur_data/piece_type.hpp
        src/app/game_of_ur_data/piece.hpp
        src/app/game_of_ur_data/serialize.hpp
        src/app/game_of_ur_data/player.hpp
        src/app/game_of_ur_data/role_id.hpp

        # Engine Interface Headers
        src/app/board_locations.hpp
        src/app/ur_controller.hpp
        src/app/ur_look_at_board.hpp
        src/app/ur_player_cpu_random.hpp
        src/app/ur_player_local.hpp
        src/app/ur_records.hpp
        src/app/ur_scene_manager.hpp
        src/app/ur_scene_view.hpp
        src/app/ur_ui_navigation.hpp
        src/app/ur_ui_records_browser.hpp
        src/app/ur_ui_tutorials_browser.hpp
        src/app/ur_ui_version.hpp
        src/app/ur_ui_view.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/src/app/version.h
)

find_package(ToyMaker 0.2.1 REQUIRED)

toymaker_configure_executable(Game_Of_Ur)

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_VERSION_MAJOR "${Game_Of_Ur_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Game_Of_Ur_VERSION_MINOR}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
