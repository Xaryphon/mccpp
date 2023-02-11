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
    status/pong_response_packet
    status/status_response_packet
)

string(REPLACE ";" ".cc;handlers/" packet_handler_files "handlers/${packet_handlers}.cc")
string(REPLACE "/" "::" packet_handlers "${packet_handlers}")

target_sources(mccpp PRIVATE ${packet_handler_files})

set(packet_handlers_file "${CMAKE_CURRENT_BINARY_DIR}/handlers.hh.tmp")

file(WRITE "${packet_handlers_file}" "#pragma once\n\n#include \"client.hh\"\n#include \"../proto/generated/clientbound/types.hh\"\n\nnamespace mccpp::client {\n\n")
foreach(packet_handler ${packet_handlers})
    file(APPEND "${packet_handlers_file}" "template<> void client::handle_packet<proto::generated::clientbound::${packet_handler}>(proto::packet_reader &);\n")
endforeach()
file(APPEND "${packet_handlers_file}" "\n}\n")

execute_process(
    COMMAND cmake -E copy_if_different "${packet_handlers_file}" handlers.hh
)
