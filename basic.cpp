#include <coroutine>
#include <iostream>

struct MyCoroutine
{
    struct promise_type
    {
        MyCoroutine get_return_object()
        {
            return MyCoroutine{Handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend()
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

MyCoroutine my_coroutine()
{
    std::cout << "A\n";

    co_await std::suspend_always{};

    std::cout << "B\n";

    co_await std::suspend_always{};

    std::cout << "C\n";

    co_return;
}

int main()
{
    auto task = my_coroutine();

    std::cout << "Main 1\n";

    task.resume();

    std::cout << "Main 2\n";

    task.resume();

    std::cout << "Main 3\n";

    task.resume();

    std::cout << "Main 4\n";
}