FetchContent_Declare(
    artic_data
    GIT_REPOSITORY https://github.com/Articdive/ArticData.git
    GIT_TAG        1.19.3 # NOTE: Using a commit hash doesn't work as upstream removes old commits
                          # Look into using the generators instead
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(artic_data)

set(PACKETS_JSON "${artic_data_SOURCE_DIR}/1_19_3_packets.json")
set(PACKET_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/packet_types.py")
set(PACKET_DEPENDS "${PACKETS_JSON}" "${PACKET_GENERATOR}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PACKET_DEPENDS})
execute_process(
    COMMAND
        "${Python3_EXECUTABLE}" "${REGENERATE_PY}"
        ${PACKET_DEPENDS}
        "${CMAKE_CURRENT_BINARY_DIR}/PACKETS_TIMESTAMP"
        "--"
        "${Python3_EXECUTABLE}" "${PACKET_GENERATOR}"
        "${PACKETS_JSON}" "${GENERATED_DIR}/proto"
    COMMAND_ERROR_IS_FATAL ANY
)

set(BLOCKS_JSON "${artic_data_SOURCE_DIR}/1_19_3_blocks.json")
set(BLOCK_PROPERTIES_JSON "${artic_data_SOURCE_DIR}/1_19_3_block_properties.json")
set(BLOCK_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/block_states.py")
set(BLOCK_OUT_CC "${GENERATED_DIR}/data/block.cc")
set(BLOCK_DEPENDS "${BLOCK_GENERATOR}" "${BLOCKS_JSON}" "${BLOCK_PROPERTIES_JSON}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${BLOCK_DEPENDS})
execute_process(
    COMMAND
        "${Python3_EXECUTABLE}" "${REGENERATE_PY}" ${BLOCK_DEPENDS}
        "${CMAKE_CURRENT_BINARY_DIR}/BLOCKS_TIMESTAMP"
        "--"
        "${Python3_EXECUTABLE}" "${BLOCK_GENERATOR}"
        "${BLOCKS_JSON}" "${BLOCK_PROPERTIES_JSON}" "${BLOCK_OUT_CC}"
    COMMAND_ERROR_IS_FATAL ANY
)
target_sources(mccpp
    PRIVATE
        "${BLOCK_OUT_CC}"
)
