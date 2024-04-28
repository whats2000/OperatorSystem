#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

std::mutex printMutex;

class DiningPhilosophers {
    enum State { THINKING, HUNGRY, EATING };
    std::vector<State> state;
    std::vector<std::condition_variable> self;
    std::mutex mtx;

public:
    explicit DiningPhilosophers(int n) : state(n, THINKING), self(n) {}

    void pickUp(int i) {
        std::unique_lock<std::mutex> lock(mtx);
        state[i] = HUNGRY;
        test(i);
        while (state[i] != EATING) {
            self[i].wait(lock);
        }
    }

    void putDown(int i) {
        std::unique_lock<std::mutex> lock(mtx);
        state[i] = THINKING;
        // Test left and right neighbors
        test((i + 4) % 5);
        test((i + 1) % 5);
    }

    void test(int i) {
        if (state[(i + 4) % 5] != EATING &&
            state[i] == HUNGRY &&
            state[(i + 1) % 5] != EATING) {
            state[i] = EATING;
            self[i].notify_one();
        }
    }
};

void philosopher(int id, DiningPhilosophers& table, int numEats = 3) {
    for (int i = 0; i < numEats; i++) {
        // Think
        std::unique_lock<std::mutex> lock(printMutex);
        std::cout << "Philosopher " << id << " is thinking.\n";
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        // End think

        table.pickUp(id);

        // Eat
        lock.lock();
        std::cout << "Philosopher " << id << " is eating.\n";
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        // End eat

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
