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


#:: spng

CPMAddPackage(
    NAME spng
    GIT_REPOSITORY https://github.com/randy408/libspng.git
    GIT_TAG        4baa1b1384da4ae9ddbe42a90e5f6300afb44fe7 # v0.7.2
    OPTIONS
        "SPNG_SHARED OFF"
        "SPNG_STATIC ON"
        "BUILD_EXAMPLES OFF"
)

if (spng_ADDED)
    # spng doesn't add the include directory for some reason
    target_include_directories(spng_static PUBLIC "${spng_SOURCE_DIR}/spng")
    # and apparently it doesn't publically link with zlib either
    find_package(ZLIB REQUIRED)
    target_link_libraries(spng_static PUBLIC ZLIB::ZLIB)
endif()


#:: glm

CPMAddPackage(
    NAME glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
    OPTIONS
        "BUILD_STATIC_LIBS OFF"
        "BUILD_SHARED_LIBS OFF"
        "BUILD_TESTING OFF"
)


#:: fmt

CPMAddPackage(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        a33701196adfad74917046096bf5a2aa0ab0bb50 # 9.1.0
    OPTIONS
        "FMT_DOC OFF"
        "FMT_TEST OFF"
)


#:: Catch2

CPMAddPackage(
    NAME Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        ab6c7375be9a8e71ee84c6f8537113f9f47daf99 # v3.2.1
)


#:: asio

CPMAddPackage(
  NAME asio
  VERSION 1.16.1
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        147f7225a96d45a2807a64e443177f621844e51c # asio-1-24-0
)

if(asio_ADDED)
    add_library(asio INTERFACE)

    target_include_directories(asio
        INTERFACE ${asio_SOURCE_DIR}/asio/include
    )

    target_compile_definitions(asio
        INTERFACE
            ASIO_STANDALONE
            ASIO_NO_DEPRECATED
    )

    #target_link_libraries(asio
    #  INTERFACE
    #    Threads::Threads
    #)

    if(WIN32)
        # macro see @ https://stackoverflow.com/a/40217291/1746503
        macro(get_win32_winnt version)
            if (CMAKE_SYSTEM_VERSION)
                set(ver ${CMAKE_SYSTEM_VERSION})
                string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
                string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
                if ("${verMajor}" MATCHES "10")
                    set(verMajor "A")
                    string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
                endif ("${verMajor}" MATCHES "10")
                string(REPLACE "." "" ver ${ver})
                string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
                set(${version} "0x${ver}")
            endif()
        endmacro()

        if(NOT DEFINED _WIN32_WINNT)
            get_win32_winnt(ver)
            set(_WIN32_WINNT ${ver})
        endif()

        message(STATUS "Set _WIN32_WINNT=${_WIN32_WINNT}")

        target_compile_definitions(asio
            INTERFACE
                _WIN32_WINNT=${_WIN32_WINNT}
                WIN32_LEAN_AND_MEAN
        )
    endif()
endif()


#:: nlohmann/json

CPMAddPackage(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d # v3.11.2
    OPTIONS
        "JSON_BuildTests OFF"
)
