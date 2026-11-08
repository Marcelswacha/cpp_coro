#include <coroutine>
#include <iostream>

struct MyAwaiter {
    bool await_ready() { return true;}

    void await_suspend(std::coroutine_handle<> handle) {
        std::cout << "Suspending" << std::endl;
    }

    int await_resume() {
        std::cout << "Resuming" << std::endl;
        return 123;
    }
};

struct MyCoroutine
{
    struct promise_type
    {
        MyCoroutine get_return_object()
        {
            return MyCoroutine{Handle::from_promise(*this)};
        }

        std::suspend_never initial_suspend()
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void return_void() {}

        void unhandled_exception()
        {
            std::terminate();
        }
    };
    
    using Handle = std::coroutine_handle<promise_type>;
    
    MyCoroutine( const Handle coroutine) : m_coroutine{coroutine}{}
    
    ~MyCoroutine()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }
    
    void resume() const {
        if (m_coroutine)
            m_coroutine.resume();
    }
    
    private:
    Handle m_coroutine;
};

MyCoroutine example() {
    std::cout << "Before\n";

    std::cout << co_await MyAwaiter{} << std::endl;

    std::cout << "After\n";

    co_return;
}

int main() {
    auto task = example();

    std::cout << "Main\n";

}