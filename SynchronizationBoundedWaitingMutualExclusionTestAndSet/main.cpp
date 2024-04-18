#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

const int n = 3; // Number of threads
std::atomic<bool> lock(false);
std::vector<std::atomic<bool>> waiting(n);

/**
 * The function is used to implement the test-and-set operation.
 *
 * @param target - The target atomic variable.
 * @return - The value of the target atomic variable before the operation.
 */
bool test_and_set(std::atomic<bool> *target) {
    return target->exchange(true);
}
/**
 * The function is used to implement the acquire operation.
 * @param id - The id of the thread.
 * @param critical_section - The function pointer to the critical section.
 */
void acquire(int id, void (*critical_section)(int)) {
    waiting[id].store(true);
    bool key = true;
    while (waiting[id].load() && key) {
        key = test_and_set(&lock);
    }
    waiting[id].store(false);

    // Execute critical section
    critical_section(id);

    int j = (id + 1) % n;
    while ((j != id) && !waiting[j].load()) {
        j = (j + 1) % n;
    }

    if (j == id) {
        lock.store(false);
    } else {
        waiting[j].store(false);
    }
}

/**
 * The function is used to execute the critical section.
 *
 * @param id - The id of the thread.
 */
void critical_section(int id) {
    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Delay " << i + 1 << " second(s)." << " Thread id: " << id << std::endl;
    }

    std::cout << "Critical section is executed." << " Thread id: " << id << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    threads.reserve(n);
    for (int i = 0; i < n; i++) {
        threads.emplace_back(acquire, i, critical_section);
    }

    // Join all threads to the main thread
    for (auto &t: threads) {
        t.join();
    }

    return 0;
}

