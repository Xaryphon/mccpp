add_subdirectory(renderer)
add_subdirectory(resource)
add_subdirectory(world)

target_sources(mccpp
    PRIVATE
        application.cc
        logger.cc
        main.cc
)
