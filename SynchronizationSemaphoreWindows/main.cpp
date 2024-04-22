#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <mutex>

class Semaphore {
    HANDLE hSemaphore; // Windows semaphore handle

public:
    explicit Semaphore(int initValue) {
        hSemaphore = CreateSemaphore(nullptr, initValue, LONG_MAX, nullptr);
        if (hSemaphore == nullptr) {
            throw std::runtime_error("Failed to create semaphore");
        }
    }

    ~Semaphore() {
        CloseHandle(hSemaphore);
    }

    void wait() {
        WaitForSingleObject(hSemaphore, INFINITE);
    }

    void signal() {
        ReleaseSemaphore(hSemaphore, 1, nullptr);
    }
};

std::vector<std::string> logs;
std::mutex log_mutex;     // Protect access to logs

void critical_section(int id) {
    auto start_time = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Simulate some work
    auto end_time = std::chrono::system_clock::now();

    std::stringstream ss;
    ss << "Thread " << id << " entered critical section at "
       << std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count()
       << "ms and exited at "
       << std::chrono::duration_cast<std::chrono::milliseconds>(end_time.time_since_epoch()).count() << "ms.\n";

    std::lock_guard<std::mutex> guard(log_mutex);
    logs.push_back(ss.str());
}

void task(int id, Semaphore& sem) {
    sem.wait();
    critical_section(id);
    sem.signal();
}

int main() {
    const int num_threads = 6;
    Semaphore sem(2);  // Initialize semaphore with 2 available slots
    std::vector<std::thread> threads;

    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(task, i + 1, std::ref(sem));
    }

    std::cout << "Main thread is waiting for all threads to finish...\n";

    for (auto &thread : threads) {
        thread.join();
    }

    for (const auto& log : logs) {
        std::cout << log;
    }

    return 0;
}

/**
 * 這是在 Windows 環境下使用 Semaphore 的範例程式碼。
 */
