#pragma once

#include<future>
#include<thread>
#include<iostream>
#include<queue>
#include<memory>
#include<functional>
class ThreadPool {
private:
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> workers;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
public:
    template<typename F, typename... Args>
    auto enqueue(F &&f, Args && ...args);
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();
};


template<typename F, typename...Args>
auto ThreadPool::enqueue(F&& f, Args&& ...args) {
    using return_type = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<return_type()>> (
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        if(stop) {
            throw std::runtime_error("enqueue on stopped Thread pool");
        }

        tasks.emplace([task = std::move(task)]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}
