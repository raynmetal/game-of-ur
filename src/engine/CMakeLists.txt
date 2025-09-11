list(APPEND engine_headers
    # Core sources
    core/ecs_world_resource_ext.hpp
    core/ecs_world.hpp
    core/resource_database.hpp

    # Input system sources
    input_system/input_data.hpp
    input_system/input_system.hpp

    # Misc. internals 
    # application.hpp.in
    camera_system.hpp
    framebuffer.hpp
    instance.hpp
    light.hpp
    material.hpp
    mesh.hpp
    model.hpp
    registrator.hpp
    render_stage.hpp
    render_system.hpp
    scene_components.hpp
    scene_loading.hpp
    scene_system.hpp
    shader_program.hpp
    shapegen.hpp
    signals.hpp
    sim_system.hpp
    spatial_query_basic_types.hpp
    spatial_query_math.hpp
    spatial_query_octree.hpp
    spatial_query_system.hpp
    text_render.hpp
    texture.hpp
    util.hpp
    vertex.hpp
    window_context_manager.hpp
)

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


foreach(header_file ${engine_headers})
    configure_file(${header_file} toymaker/${header_file})
endforeach()
configure_file(application.hpp.in toymaker/application.hpp)

add_library(ToyMaker STATIC ${engine_sources})
set_property(TARGET ToyMaker PROPERTY VERSION "0.0.2")

target_include_directories(ToyMaker PUBLIC ${CMAKE_CURRENT_BINARY_DIR} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/toymaker)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW 2.0 REQUIRED)
find_package(glm 1.0 REQUIRED)
find_package(assimp 6.0 REQUIRED)
find_package(nlohmann_json 3.12.0 REQUIRED)

target_link_libraries(ToyMaker PUBLIC OpenGL::GL)
target_link_libraries(ToyMaker PUBLIC SDL2::SDL2)
target_link_libraries(ToyMaker PUBLIC GLEW::GLEW)
target_link_libraries(ToyMaker PUBLIC glm::glm)
target_link_libraries(ToyMaker PUBLIC SDL2_image::SDL2_image)
target_link_libraries(ToyMaker PUBLIC SDL2_ttf::SDL2_ttf)
target_link_libraries(ToyMaker PUBLIC assimp::assimp)
target_link_libraries(ToyMaker PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(ToyMaker PUBLIC project_compiler_flags)
