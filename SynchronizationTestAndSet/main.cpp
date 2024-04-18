#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

/**
 * Non-Atomic lock Variable:
 * - The lock variable is a regular bool, which is not atomic.
 * - This means that read and write operations on this variable are not atomic and can be interrupted by thread switches,
 *   leading to race conditions.
 *
 * Solution:
 * - Use an atomic variable to ensure that the read and write operations on the lock variable are atomic.
 */
// bool lock = false;
std::atomic<bool> lock(false);
/**
 * The function is used to test and set the value of an atomic boolean variable.
 *
 * @return - The value of the atomic boolean variable before it is set.
 */
bool test_and_set() {
    /**
     * The following code is not thread-safe.
     */
    //    bool rv = lock;
    //    lock = true;
    //    return rv;

    /**
     * The following code is thread-safe.
     * For the atomic boolean variable, the value is read and set atomically.
     */
    return lock.exchange(true);
}

/**
 * The function is used to acquire a lock.
 *
 * @param critical_section - The critical section to be executed.
 * @param id - The id of the thread.
 */
void acquire(void (*critical_section)(int), int id) {
    // Attempt to acquire lock until successful
    while (test_and_set()) {
        /**
         * The following code is used to reduce the CPU usage.
         * To prevent the busy-waiting, the thread yields the CPU.
         */
        std::this_thread::yield();
    }

    // Critical section
    critical_section(id);

    // Release the lock
    lock.store(false); // lock = false;
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
    std::thread t1(acquire, critical_section, 1);
    std::thread t2(acquire, critical_section, 2);

    t1.join();
    t2.join();

    return 0;
}
