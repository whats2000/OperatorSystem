#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

// 這就是一個硬體級的原子操作的 C++ 實現
std::atomic<bool> lock(false);

/**
 * The function is used to test and set the value of an atomic boolean variable.
 *
 * @return - The value of the atomic boolean variable before it is set.
 */
bool test_and_set() {
    /**
     * In hardware, the test-and-set operation is implemented as a single atomic instruction.
     *
     * bool rv = lock;
     * lock = true;
     * return rv;
     */

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
    lock.store(false);
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
    std::thread t0(acquire, critical_section, 0);
    std::thread t1(acquire, critical_section, 1);

    t0.join();
    t1.join();

    return 0;
}

/**
 * 以上程式碼中，我們使用了 atomic<bool> 類型的變數 lock 來實珵了一個簡單的鎖。
 * - Mutual exclusion is preserved 互斥執行
 * - Progress requirement is satisfied 在沒人執行時可以直接進入
 * - 不滿足 bounded waiting requirement
 *   - 此實現中存在一個潛在的問題是無界等待，因為沒有限制一個線程在其他線程之前重複獲得鎖的次數。
 *   - 由於鎖的獲得是基於不確定的線程調度和鎖競爭，因此不能保證線程間獲得鎖的公平性或有界等待。
 *
 * 接下來查看 Compare-and-Swap (CAS) 操作的原子變數類型。
 */
