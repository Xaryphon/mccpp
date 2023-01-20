add_subdirectory(client)
add_subdirectory(input)
add_subdirectory(proto)
add_subdirectory(renderer)
add_subdirectory(resource)
add_subdirectory(world)

target_sources(mccpp
    PRIVATE
        application.cc
        cvar.cc
        logger.cc
        main.cc
)
