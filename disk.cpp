#include <iostream>
#include <string>
#include <coroutine>
#include <thread>
#include <chrono>

// 1. A simple Awaitable representing an asynchronous disk read operation
struct AsyncDiskRead {
    std::string filename;
    std::string result;

    // Always suspend so we can dispatch the async operation
    bool await_ready() const noexcept { return false; }

    // This is called when the coroutine suspends
    void await_suspend(std::coroutine_handle<> handle) {
        // Simulate an asynchronous background disk read (e.g., OS thread pool, io_uring, IOCP)
        std::thread([this, handle]() {
            // Simulate disk latency (waiting for hardware / OS)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // "Read" data from the file
            result = "Contents of " + filename + " loaded asynchronously!";
            
            // Resume the coroutine once data is ready
            handle.resume();
        }).detach();
    }

    // This is what `co_await` evaluates to
    std::string await_resume() const noexcept {
        return result;
    }
};

// 2. Task type representing the asynchronous coroutine
struct Task {
    struct promise_type {
        Task get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> h;
    ~Task() { if (h) h.destroy(); }
};

// 3. The Coroutine: Uses co_await to read from disk non-blockingly
Task process_file_async(std::string filename) {
    std::cout << "[Coroutine] Requesting async read for: " << filename << '\n';

    // Execution suspends here. The thread is free to do other work 
    // while the OS/disk handles the read request.
    std::string file_data = co_await AsyncDiskRead{filename, ""};

    std::cout << "[Coroutine] Resumed! Data received: \"" << file_data << "\"\n";
}

int main() {
    std::cout << "Main thread starts.\n";

    // Launch the asynchronous coroutine
    auto task = process_file_async("./.vscode/launch.json");

    std::cout << "Main thread continues doing other work while disk reads...\n";

    // Keep main alive long enough for the background thread to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::cout << "Main thread finishes.\n";
    return 0;
}