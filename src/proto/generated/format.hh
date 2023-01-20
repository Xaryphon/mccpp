// Automatically generated by generator/packet_types.py
#pragma once

#include <fmt/format.h>
#include "misc.hh"

template<>
struct fmt::formatter<mccpp::proto::generated::packet_direction> : public fmt::formatter<string_view> {
 using type = mccpp::proto::generated::packet_direction;
 template<typename FormatContext>
 auto format(const type &value, FormatContext &ctx) const -> decltype(ctx.out()) {
  switch (value) {
  case type::SERVERBOUND: return formatter<string_view>::format("serverbound", ctx);
  case type::CLIENTBOUND: return formatter<string_view>::format("clientbound", ctx);
  }
  throw format_error("invalid packet_direction");
 }
};

template<>
struct fmt::formatter<mccpp::proto::generated::connection_state> : public fmt::formatter<string_view> {
 using type = mccpp::proto::generated::connection_state;
 template<typename FormatContext>
 auto format(const type &value, FormatContext &ctx) const -> decltype(ctx.out()) {
  switch (value) {
  case type::HANDSHAKING: return formatter<string_view>::format("handshaking", ctx);
  case type::PLAY: return formatter<string_view>::format("play", ctx);
  case type::STATUS: return formatter<string_view>::format("status", ctx);
  case type::LOGIN: return formatter<string_view>::format("login", ctx);
  }
  throw format_error("invalid connection_state");
 }
};
