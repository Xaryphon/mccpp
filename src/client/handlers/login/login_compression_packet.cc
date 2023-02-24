#include "generated/client/handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Set_Compression
template<>
void client::handle_packet<proto::generated::clientbound::login::login_compression_packet>(proto::packet_reader &s) {
    (void)s;
    MCCPP_F("Protocol compression not yet implemented");
    abort();
}

}
