#pragma once

#include <string_view>

#include "../utility/runtime_array.hh"

namespace mccpp::resource::manager {

utility::runtime_array<char> read_file(const char *path);

};
