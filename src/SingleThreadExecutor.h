#pragma once

#include <queue>
#include <functional>
#include <condition_variable>
#include <thread>
#include <mutex>

class SingleThreadExecutor {
    std::queue<std::move_only_function<void()>> queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    bool stop = false;

public:
    SingleThreadExecutor() {
        worker = std::thread([this] {
            for (;;) {
                std::move_only_function<void()> task;
                {
                    std::unique_lock lock(mtx);
                    cv.wait(lock, [this] { return stop || !queue.empty(); });
                    if (stop && queue.empty()) break;
                    task = std::move(queue.front());
                    queue.pop();
                }
                task();
            }
            });
    }

    ~SingleThreadExecutor() {
        { std::lock_guard lock(mtx); stop = true; }
        cv.notify_one();
        if (worker.joinable()) worker.join();
    }

    template<typename F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        using R = std::invoke_result_t<F>;
        auto task = std::packaged_task<R()>(std::forward<F>(f));
        auto future = task.get_future();
        {
            std::lock_guard lock(mtx);
            queue.push(std::move(task));
        }
        cv.notify_one();
        return future;
    }
};