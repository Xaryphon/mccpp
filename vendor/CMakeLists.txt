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
