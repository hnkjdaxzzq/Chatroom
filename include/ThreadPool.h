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

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    // 根据线程数创建多个线程
    for(auto i = 0; i < threads; ++i) {
        workers.emplace_back([this]() {
            while(true) {  // 工作线程死循环，不断查询任务队列并取出任务执行
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, 
                                        [this]() {
                                            return this->stop || !this->tasks.empty();
                                        } 
                    );

                    if(this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();

            }
        }
        
        );
    }
}

template<typename F, typename...Args>
auto ThreadPool::enqueue(F&& f, Args&& ...args) {
    using return_type = std::invoke_result_t<F, Args...>;
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

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker : workers)
        worker.join();
}