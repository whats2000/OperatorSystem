#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>

// Print Lock
std::mutex printMutex;

// Random Number Generator
std::random_device rd;
std::mt19937 gen(rd());

class DiningPhilosophers {
    std::vector<bool> forks;
    std::vector<std::condition_variable> conditions;
    std::mutex mtx;

public:
    explicit DiningPhilosophers(int n) : forks(n, false), conditions(n) {}

    void pickUp(int philId) {
        std::unique_lock<std::mutex> lock(mtx);
        int left = philId;
        int right = static_cast<int>((philId + 1) % forks.size());

        // Wait until both left and right forks are available
        conditions[philId].wait(lock, [&] { return !forks[left] && !forks[right]; });

        // Take both forks
        forks[left] = true;
        forks[right] = true;
    }

    void putDown(int philId) {
        std::unique_lock<std::mutex> lock(mtx);
        int left = philId;
        int right = static_cast<int>((philId + 1) % forks.size());

        // Release both forks
        forks[left] = false;
        forks[right] = false;

        // Notify the next philosophers
        conditions[left].notify_one();
        conditions[right].notify_one();
    }
};

/**
 * @brief Philosopher thread function
 * @param id - Philosopher ID
 * @param table - DiningPhilosophers object
 * @param n - Number of times to eat
 */
void philosopher(int id, DiningPhilosophers& table, int n = 3) {
    for (int i = 0; i < n; i++) {
        std::unique_lock<std::mutex> lock(printMutex);
        std::cout << "Philosopher " << id << " is thinking.\n";
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(gen() % 3));

        table.pickUp(id);

        /* Critical Section */
        lock.lock();
        std::cout << "Philosopher " << id << " is eating.\n";
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(gen() % 3));
        /* End of Critical Section */

        table.putDown(id);
    }
}

int main() {
    int numPhilosophers = 5;
    DiningPhilosophers table(numPhilosophers);

    std::vector<std::thread> philosophers;
    philosophers.reserve(numPhilosophers);
    for (int i = 0; i < numPhilosophers; i++) {
        philosophers.emplace_back(philosopher, i, std::ref(table), 20);
    }

    for (auto& p : philosophers) {
        p.join();
    }

    return 0;
}
