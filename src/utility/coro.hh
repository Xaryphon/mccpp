#pragma once

#include <cassert>
#include <coroutine>
#include <cstdlib>
#include <exception>
#include <optional>
#include <utility>

namespace mccpp {

template<typename T>
concept ChainableCoroutine = requires(T a) {
    { a.promise().m_resume } -> std::convertible_to<std::coroutine_handle<>>;
};

struct task_resumer {
    constexpr bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(ChainableCoroutine auto handle) noexcept {
        return handle.promise().m_resume;
    }
    constexpr void await_resume() noexcept {}
};

template<typename T>
class task_promise_helper {
public:
    void return_value(T &&value) { m_value.emplace(std::move(value)); }
    bool has_result() { return m_value.has_value(); }
    T &result() { return m_value.value(); }

private:
    std::optional<T> m_value;
};

template<>
class task_promise_helper<void> {
public:
    void return_void() {}
    bool has_result() { return true; }
    void result() {}
};

template<typename T = void>
class task {
public:
    class promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    class promise_type : public task_promise_helper<T> {
    public:
        task get_return_object() { return task { handle_type::from_promise(*this) }; }
        std::suspend_always initial_suspend() { return {}; }
        task_resumer final_suspend() noexcept { return {}; }
        void unhandled_exception() { m_exception = std::current_exception(); }

    private:
        std::coroutine_handle<> m_resume = std::noop_coroutine();
        std::exception_ptr m_exception;
        friend class task;
        friend struct task_resumer;
    };

    task(handle_type handle)
    : m_handle(handle)
    {}

    task(const task &) = delete;

    task(task &&other)
    : m_handle(std::exchange(other.m_handle, nullptr))
    {}

    ~task() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

    handle_type handle() {
        return m_handle;
    }

    promise_type &promise() {
        return m_handle.promise();
    }

    constexpr bool await_ready() { return false; }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> resume) {
        m_handle.promise().m_resume = resume;
        return m_handle;
    }

    T await_resume() {
        assert(m_handle.done());
        if (m_handle.promise().m_exception) {
            std::rethrow_exception(m_handle.promise().m_exception);
        }
        assert(m_handle.promise().has_result());
        return m_handle.promise().result();
    }

private:
    handle_type m_handle;
};

}
