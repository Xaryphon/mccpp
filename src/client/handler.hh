#pragma once

#include "client.hh"

namespace mccpp::client {

template<typename T>
class packet_handler : public proto::packet_handler {
public:
    using packet_info = T;
    using packet_type = proto::packet<T>;

    packet_handler(class client &c)
    : m_client(c)
    {}

    void read(proto::packet_reader &s) override {
        m_packet.read(s);
    }

    void process() override;

    class client &client() { return m_client; }
    packet_type &packet() { return m_packet; }

private:
    class client &m_client;
    packet_type m_packet;
};

}
