#pragma once

#include <functional>
#include <exception>

#include "misc.hh"

namespace mccpp::utility {

template<typename F>
class scope_exit {
public:
    scope_exit(F &&cb)
    : m_callback(cb)
    {}

    scope_exit(const scope_exit &) = delete;
    scope_exit(scope_exit &&) = delete;

    ~scope_exit()
    {
        m_callback();
    }

private:
    F m_callback;
};

template<typename F>
class scope_fail {
public:
    scope_fail(F &&cb)
    : m_callback(cb), m_uncaught(std::uncaught_exceptions())
    {}

    scope_fail(const scope_fail &) = delete;
    scope_fail(scope_fail &&) = delete;

    ~scope_fail()
    {
        if (m_uncaught != std::uncaught_exceptions())
            m_callback();
    }

private:
    F m_callback;
    int m_uncaught;
};

template<typename F>
class scope_success {
public:
    scope_success(F &&cb)
    : m_callback(cb), m_uncaught(std::uncaught_exceptions())
    {}

    scope_success(const scope_success &) = delete;
    scope_success(scope_success &&) = delete;

    ~scope_success()
    {
        if (m_uncaught == std::uncaught_exceptions())
            m_callback();
    }

private:
    F m_callback;
    int m_uncaught;
};

}

#define _MCCPP_SCOPE_GUARD(type, line) mccpp::utility::type xx_##type##_##line = [&]
#define MCCPP_SCOPE_EXIT    MCCPP_FORWARD(_MCCPP_SCOPE_GUARD, scope_exit,    __LINE__)
#define MCCPP_SCOPE_FAIL    MCCPP_FORWARD(_MCCPP_SCOPE_GUARD, scope_fail,    __LINE__)
#define MCCPP_SCOPE_SUCCESS MCCPP_FORWARD(_MCCPP_SCOPE_GUARD, scope_success, __LINE__)
