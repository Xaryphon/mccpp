add_subdirectory(handlers)

target_sources(mccpp
    PRIVATE
        client.cc
        handlers.cc
)
