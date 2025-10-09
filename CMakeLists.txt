cmake_minimum_required(VERSION 3.23)

project(ToyMakerDemo VERSION 0.0.2)


set(CMAKE_DEBUG_POSTFIX -d)
set(CMAKE_RELWITHDEBINFO_POSTFIX -dfast)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)


add_executable(ToyMakerDemo)
set_target_properties(ToyMakerDemo PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(ToyMakerDemo PROPERTIES RELWITHDEBINFO_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})
set_target_properties(ToyMakerDemo PROPERTIES WIN32_EXECUTABLE $<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>>)
target_compile_definitions(ToyMakerDemo PRIVATE CMAKE_EXPORT_COMPILE_COMMANDS=1)

add_library(project_compiler_flags INTERFACE)
target_compile_features(project_compiler_flags INTERFACE cxx_std_17)
set(gcc_like_cxx $<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>)
set(msvc_cxx $<COMPILE_LANG_AND_ID:CXX,MSVC>)
target_compile_options(
    project_compiler_flags INTERFACE
    "$<$<CONFIG:Debug>:$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused;-g>>"
    "$<$<CONFIG:Debug>:$<${msvc_cxx}:-W3;-g>>"
)

target_include_directories(
    ToyMakerDemo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/app ${CMAKE_CURRENT_BINARY_DIR}/src/app
)

# copy all data files over, creating dependency between copy and source
# see: https://stackoverflow.com/questions/34799916/copy-file-from-source-directory-to-binary-directory-using-cmake
file(GLOB_RECURSE SRC_FILES "src/app/*.cpp")
file(GLOB_RECURSE DATA_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "data/*")
foreach(data_file ${DATA_FILES})
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${data_file}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/${data_file}"
            "${CMAKE_CURRENT_BINARY_DIR}/${data_file}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${data_file}"
    )
endforeach()
add_custom_target(build_time_make_directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory user_data
)
add_custom_target(data_file_dependencies DEPENDS "$<LIST:TRANSFORM,${DATA_FILES},PREPEND,${CMAKE_CURRENT_BINARY_DIR}/>")

add_subdirectory(src/engine)

target_sources(
    ToyMakerDemo
    # Ur application sources
    PRIVATE
        ${SRC_FILES}
        src/main.cpp
)


find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(nlohmann_json 3.12.0 REQUIRED)
find_package(glm 1.0 REQUIRED)
find_package(assimp 6.0 REQUIRED)

if(TARGET SDL2::SDL2main)
    target_link_libraries(ToyMakerDemo PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(ToyMakerDemo PRIVATE SDL2::SDL2)
target_link_libraries(ToyMakerDemo PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(ToyMakerDemo PRIVATE glm::glm)
target_link_libraries(ToyMakerDemo PRIVATE assimp::assimp)
target_link_libraries(ToyMakerDemo PRIVATE project_compiler_flags)
target_link_libraries(ToyMakerDemo PRIVATE ToyMaker)

add_dependencies(ToyMakerDemo data_file_dependencies build_time_make_directory)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data
    DESTINATION .
)

install(
    DIRECTORY
    DESTINATION user_data
)
install(
    FILES
        "docs/systems/game-of-ur/01_design_doc.md"
        "LICENSE.txt"
        "README.md"
    DESTINATION 
        "."
)
install(
    TARGETS ToyMakerDemo
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
)
install(
    RUNTIME_DEPENDENCY_SET deps
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    DIRECTORIES $ENV{PATH}
    DESTINATION .
)


set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt)
set(CPACK_PACKAGE_VERSION_MAJOR "${ToyMakerDemo_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ToyMakerDemo_VERSION_MINOR}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
