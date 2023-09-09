#include "../include/ThreadPool.h"

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

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker : workers)
        worker.join();
}
