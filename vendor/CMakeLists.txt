include(FetchContent)
include(get_cpm.cmake)

#:: glad

set(gl_exts
)
string(REPLACE ";" "," gl_exts "${gl_exts}")

CPMAddPackage(
    NAME glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        1ecd45775d96f35170458e6b148eb0708967e402 # v0.1.36
    OPTIONS
        "GLAD_PROFILE core"
        "GLAD_API gl=4.6"
        "GLAD_GENERATOR c"
        "GLAD_EXTENSIONS ${gl_exts}"
        "GLAD_NO_LOADER ON"
)


#:: imgui

CPMAddPackage(
  NAME imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        a8df192df022ed6ac447e7b7ada718c4c4824b41 # v1.89.1
  DOWNLOAD_ONLY  YES
)

if (imgui_ADDED)
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
endif()
