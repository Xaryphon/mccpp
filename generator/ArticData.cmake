FetchContent_Declare(
    artic_data
    GIT_REPOSITORY https://github.com/Articdive/ArticData.git
    GIT_TAG        3c3d6007810dc0ef90373be46a9e49a4668067d8 # branch 1.19.3 2023-02-24
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
