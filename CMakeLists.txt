cmake_minimum_required(VERSION 3.22)

project(
    MCCPP
    VERSION 0.0.1
    LANGUAGES C CXX
)

find_package(Python3 REQUIRED)
find_package(SDL2 REQUIRED)
add_subdirectory(vendor)

add_executable(mccpp)
set_property(TARGET mccpp PROPERTY CXX_STANDARD 20)
set_property(TARGET mccpp PROPERTY CXX_STANDARD_REQUIRED YES)
set_property(TARGET mccpp PROPERTY CXX_EXTENSIONS NO)
target_include_directories(mccpp
    PUBLIC
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_BINARY_DIR}"
)

target_link_libraries(mccpp
    PUBLIC
        SDL2::SDL2
        fmt::fmt
        glm::glm
        asio
    PRIVATE
        glad
        imgui
        imgui::sdl
        imgui::gl
        nlohmann_json
        spng_static
)

add_subdirectory(generator)
add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
