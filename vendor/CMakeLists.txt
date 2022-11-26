include(FetchContent)

#:: glad

set(gl_exts
)
string(REPLACE ";" "," gl_exts "${gl_exts}")

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.6" CACHE STRING "" FORCE)
set(GLAD_GENERATOR "c" CACHE STRING "" FORCE)
set(GLAD_EXTENSIONS "${gl_exts}" CACHE STRING "" FORCE)
set(GLAD_NO_LOADER ON CACHE STRING "" FORCE)

# Not sure if using FetchContent (especially this way) is the best choice here.
# But it works, so *shrug*
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        1ecd45775d96f35170458e6b148eb0708967e402 # v0.1.36
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(glad)

set_property(TARGET glad PROPERTY POSITION_INDEPENDENT_CODE ON)


#:: imgui

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        a8df192df022ed6ac447e7b7ada718c4c4824b41 # v1.89.1
  CONFIGURE_COMMAND
  BUILD_COMMAND
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
)
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")


#:: imgui sdl

add_library(imgui_sdl STATIC
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp"
)
target_include_directories(imgui_sdl PUBLIC "${imgui_SOURCE_DIR}/backends")
target_link_libraries(imgui_sdl
    PRIVATE
        SDL2::SDL2
        imgui
)
add_library(imgui::sdl ALIAS imgui_sdl)


#:: imgui gl

add_library(imgui_gl STATIC
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui_gl PUBLIC "${imgui_SOURCE_DIR}/backends")
target_link_libraries(imgui_gl
    PRIVATE
        imgui
)
add_library(imgui::gl ALIAS imgui_gl)
