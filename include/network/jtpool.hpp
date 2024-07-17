#include <queue>
#include <thread>

class jtpool {
    struct Task {
        int priority;
        std::function<void()> func;

        bool operator<(const Task& rhs) const {
            return priority < rhs.priority;
        }
    };

    std::vector<std::jthread> workers;
    std::priority_queue<Task> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic_bool stop{false};

public:
    jtpool() {
        for (size_t i = 0; i < 12; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.top());
                        this->tasks.pop();
                    }
                    try {
                        task.func();
                    } catch (...) {}
                }
            });
        }
    }

    template<class F> void enqueue(int priority, F&& f) {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(Task{priority, std::forward<F>(f)});
        condition.notify_one();
    }

    ~jtpool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }
};