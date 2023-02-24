target_sources(mccpp
    PRIVATE
        client.cc
        handlers.cc
)

set(packet_handlers
    login/custom_query_packet
    login/game_profile_packet
    login/hello_packet
    login/login_compression_packet
    play/custom_payload_packet
    play/keep_alive_packet
    play/level_chunk_with_light_packet
    play/login_packet
    status/pong_response_packet
    status/status_response_packet
)

string(REPLACE ";" ".cc;handlers/" packet_handler_files "handlers/${packet_handlers}.cc")
string(REPLACE "/" "::" packet_handlers "${packet_handlers}")

target_sources(mccpp PRIVATE ${packet_handler_files})

set(packet_handlers_tmp_file "${PROJECT_BINARY_DIR}/generated/client/handlers.hh.tmp")
set(packet_handlers_file "${PROJECT_BINARY_DIR}/generated/client/handlers.hh")

file(WRITE "${packet_handlers_tmp_file}" "#pragma once\n\n#include \"client/client.hh\"\n#include \"../proto/clientbound/types.hh\"\n\nnamespace mccpp::client {\n\n")
foreach(packet_handler ${packet_handlers})
    file(APPEND "${packet_handlers_tmp_file}" "template<> void client::handle_packet<proto::generated::clientbound::${packet_handler}>(proto::packet_reader &);\n")
endforeach()
file(APPEND "${packet_handlers_tmp_file}" "\n}\n")

execute_process(
    COMMAND cmake -E copy_if_different "${packet_handlers_tmp_file}" "${packet_handlers_file}"
    COMMAND_ERROR_IS_FATAL ANY
)
