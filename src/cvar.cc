#include "cvar.hh"

#include <cassert>

#include "logger.hh"

namespace mccpp {

cvar &cvar::create(std::string_view name,
                   float value,
                   std::string_view help,
                   callback callback)
{
    auto result = s_cvars.emplace(name, help, callback, value);
    assert(result.second);
    // this hurts my soul but combined with
    // - operator<=> only comparing m_name
    // - and m_name being const
    // this should probably hopefully be fine
    return const_cast<cvar &>(*result.first);
}

bool cvar::set_value(float new_value) {
    if (m_callback(new_value)) {
        m_value = new_value;
        return true;
    }
    MCCPP_W("Failed to set cvar {} to {:3f} (from {:3f})", m_name, new_value, m_value);
    return false;
}

cvar::storage cvar::s_cvars = {};

}
