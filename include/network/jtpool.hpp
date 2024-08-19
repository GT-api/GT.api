#include <queue>

class jtpool {
    struct Task 
    {
        int priority;
        std::function<void()> func;
        bool operator<(const Task& rhs) const { return priority < rhs.priority; }
    };
    std::vector<std::jthread> workers;
    std::priority_queue<Task> tasks;
    std::mutex mutex;
    std::condition_variable_any condition;
    std::atomic_bool stop{false};
public:
    jtpool() {
        for (size_t i = 0; i < std::thread::hardware_concurrency(); ++i) 
        {
            workers.emplace_back([this] 
            {
                while (true) 
                {
                    Task task;
                    {
                        std::unique_lock lock(mutex);
                        condition.wait(lock, [this] { return stop or not tasks.empty(); });
                        if (stop and tasks.empty()) return;
                        task = std::move(tasks.top());
                        tasks.pop();
                    }
                    task.func();
                }
            });
        }
    }

    template<class F>
    void enqueue(int priority, F&& f) 
    {
        { // @note race control
            std::unique_lock lock(mutex);
            tasks.emplace(Task{priority, std::forward<F>(f)});
        }
        condition.notify_one();
    }

    ~jtpool() 
    {
        { // @note race control
            std::unique_lock lock(mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers)
            if (worker.joinable())
                worker.join();
    }
};