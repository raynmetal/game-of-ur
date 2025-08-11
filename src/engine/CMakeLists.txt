# list(APPEND engine_headers
#     # Core sources
#     ${PROJECT_BINARY_DIR}/core/ecs_world_resource_ext.hpp
#     ${PROJECT_BINARY_DIR}/core/ecs_world.hpp
#     ${PROJECT_BINARY_DIR}/core/resource_database.hpp

#     # Input system sources
#     ${PROJECT_BINARY_DIR}/input_system/input_data.hpp
#     ${PROJECT_BINARY_DIR}/input_system/input_system.hpp

#     # Misc. internals 
#     ${PROJECT_BINARY_DIR}/application.hpp
#     ${PROJECT_BINARY_DIR}/camera_system.hpp
#     ${PROJECT_BINARY_DIR}/framebuffer.hpp
#     ${PROJECT_BINARY_DIR}/instance.hpp
#     ${PROJECT_BINARY_DIR}/light.hpp
#     ${PROJECT_BINARY_DIR}/material.hpp
#     ${PROJECT_BINARY_DIR}/mesh.hpp
#     ${PROJECT_BINARY_DIR}/model.hpp
#     ${PROJECT_BINARY_DIR}/registrator.hpp
#     ${PROJECT_BINARY_DIR}/render_stage.hpp
#     ${PROJECT_BINARY_DIR}/render_system.hpp
#     ${PROJECT_BINARY_DIR}/scene_components.hpp
#     ${PROJECT_BINARY_DIR}/scene_loading.hpp
#     ${PROJECT_BINARY_DIR}/scene_system.hpp
#     ${PROJECT_BINARY_DIR}/shader_program.hpp
#     ${PROJECT_BINARY_DIR}/shapegen.hpp
#     ${PROJECT_BINARY_DIR}/signals.hpp
#     ${PROJECT_BINARY_DIR}/sim_system.hpp
#     ${PROJECT_BINARY_DIR}/spatial_query_basic_types.hpp
#     ${PROJECT_BINARY_DIR}/spatial_query_math.hpp
#     ${PROJECT_BINARY_DIR}/spatial_query_octree.hpp
#     ${PROJECT_BINARY_DIR}/spatial_query_system.hpp
#     ${PROJECT_BINARY_DIR}/text_render.hpp
#     ${PROJECT_BINARY_DIR}/texture.hpp
#     ${PROJECT_BINARY_DIR}/util.hpp
#     ${PROJECT_BINARY_DIR}/vertex.hpp
#     ${PROJECT_BINARY_DIR}/window_context_manager.hpp
# )

list(APPEND engine_sources
    # Core sources
        core/ecs_world.cpp
        core/resource_database.cpp
   
    # Input system sources
        input_system/action_context.cpp
        input_system/input_manager.cpp

    # Everything else
        application.cpp
        camera_system.cpp
        framebuffer.cpp
        instance.cpp
        light.cpp
        material.cpp
        mesh.cpp
        model.cpp
        render_stage.cpp
        render_system.cpp
        scene_loading.cpp
        scene_system.cpp
        shader_program.cpp
        shapegen.cpp
        signals.cpp
        sim_system.cpp
        spatial_query_math.cpp
        spatial_query_octree.cpp
        spatial_query_system.cpp
        text_render.cpp
        texture.cpp
        util.cpp
        window_context_manager.cpp
)

option(ZO_DUMB_DELAY "Relinquish CPU for 10 milliseconds between frames" ON)
configure_file(application.hpp.in application.hpp)

add_library(ToyMakersEngine STATIC ${engine_sources})
set_property(TARGET ToyMakersEngine PROPERTY VERSION "0.0.2")

target_include_directories(ToyMakersEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glm 1.0 REQUIRED)
find_package(assimp 6.0 REQUIRED)
find_package(nlohmann_json 3.12.0 REQUIRED)

target_link_libraries(ToyMakersEngine PUBLIC OpenGL::GL)
target_link_libraries(ToyMakersEngine PUBLIC SDL2::SDL2)
target_link_libraries(ToyMakersEngine PUBLIC GLEW::GLEW)
target_link_libraries(ToyMakersEngine PUBLIC glm::glm)
target_link_libraries(ToyMakersEngine PUBLIC SDL2_image::SDL2_image)
target_link_libraries(ToyMakersEngine PUBLIC SDL2_ttf::SDL2_ttf)
target_link_libraries(ToyMakersEngine PUBLIC assimp::assimp)
target_link_libraries(ToyMakersEngine PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(ToyMakersEngine PUBLIC project_compiler_flags)
