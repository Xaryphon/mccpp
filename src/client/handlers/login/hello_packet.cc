#include "../../handlers.hh"

#include "../../../logger.hh"

namespace mccpp::client {

// https://wiki.vg/index.php?title=Protocol&oldid=17979#Encryption_Request
template<>
void client::handle_packet<proto::generated::clientbound::login::hello_packet>(proto::packet_reader &s) {
    (void)s;
    MCCPP_F("Protocol encryption not yet implemented");
    abort();
}

}
