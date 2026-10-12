#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>
#include <cassert>

class threadpool {
private:
    // Stores the worker threads. Each thread repeatedly takes tasks
    // from the shared queue and executes them.
    std::vector<std::thread> workers;

    // All tasks use the same type: a callable taking no arguments
    // and returning void. This keeps the queue type uniform.
    std::queue<std::function<void()>> tasks;

    // Protects shared state: tasks and stop.
    // Multiple workers and the submitting thread may access these
    // concurrently, so access must be synchronized.
    std::mutex queue_mutex;

    // Allows workers to sleep until work arrives or shutdown begins,
    // rather than repeatedly checking the queue in a busy loop.
    std::condition_variable cv;

    bool stop{false};

    void worker() {
        while (true) {
            // Each worker keeps its own local task.
            // It executes the task after removing it from the queue.
            std::function<void()> task;

            // unique_lock is required by condition_variable::wait().
            // Unlike a simple lock_guard, it can explicitly unlock
            // and is temporarily unlocked by wait() while sleeping.
            std::unique_lock<std::mutex> lock(queue_mutex);

            // Wait until shutdown is requested or work is available.
            //wait until predicate is true, i.e. either stop is true or tasks is not empty
            cv.wait(lock, [this]() {
                return (stop || !tasks.empty());
            });

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
            lock.unlock();
            task();
        }
    }

public:
    explicit threadpool(int num) {
        assert(num > 0);
        // Each lambda captures this so it can call this object's worker().
        for (int i = 0; i < num; i++) {
            workers.emplace_back(std::thread([this]() {
                worker();
            }));
        }
    }

    void execute(std::function<void()> task) {

        std::unique_lock<std::mutex> lock(queue_mutex);

        if (stop) {
            throw std::runtime_error("Threadpool is stopped");
        }

        tasks.emplace(std::move(task));
        lock.unlock();
        cv.notify_one();
    }

    void shutdown() {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // Makes sequential repeated calls to shutdown() harmless.
        if (stop) {
            return;
        }
        stop = true;
        lock.unlock();
        cv.notify_all();

        // Wait until every worker has finished.
        // A thread must be joined before its std::thread object
        // is destroyed; otherwise, destruction of a joinable thread
        // calls std::terminate().
        for (auto& w : workers) {
            w.join();
        }
    }

    ~threadpool() {
        shutdown();
    }
};

int main() {
    return 0;
}