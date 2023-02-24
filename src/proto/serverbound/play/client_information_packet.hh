#pragma once

#include "generated/proto/misc.hh"
#include "generated/proto/serverbound/types.hh"
#include "../../packet.hh"

namespace mccpp::proto {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Client_Information
template<>
struct packet<generated::serverbound::play::client_information_packet> {
    using packet_type = generated::serverbound::play::client_information_packet;

    std::string locale;
    int8_t view_distance;
    int32_t chat_mode;
    bool chat_colors;
    uint8_t displayed_skin_parts;
    int32_t main_hand;
    bool enable_text_filtering;
    bool allow_server_listings;

    void write(packet_writer &s) const {
        s.write_string<16>(locale);
        s.write_i8(view_distance);
        s.write_varint(chat_mode);
        s.write_bool(chat_colors);
        s.write_u8(displayed_skin_parts);
        s.write_varint(main_hand);
        s.write_bool(enable_text_filtering);
        s.write_bool(allow_server_listings);
    }
};

}
