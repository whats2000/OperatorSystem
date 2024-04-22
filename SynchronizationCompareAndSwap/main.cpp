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
std::atomic<bool> lock(false);

/**
 * Function to atomically compare and swap the value.
 *
 * @param value - The atomic boolean to be compared and swapped.
 * @param expected - The expected value.
 * @param new_value - The new value to be set.
 * @return True if the operation was successful, otherwise false.
 */
bool compare_and_swap(std::atomic<bool>* value, bool expected, bool new_value) {
    /**
     * In hardware, the compare-and-swap operation is implemented as a single atomic instruction.
     *
     *  int temp = *value;
     *  if (*value == expected)
     *      *value = new_value;
     *  return temp;
     */

    // Atomically compares the value with expected and if they are equivalent, sets it to new_value.
    return value->compare_exchange_strong(expected, new_value);
}

/**
 * The function is used to acquire a lock.
 *
 * @param critical_section - The critical section to be executed.
 * @param id - The id of the thread.
 */
void acquire(void (*critical_section)(int), int id) {
    while (!compare_and_swap(&lock, false, true)) {
        // Yield the thread to the scheduler
        std::this_thread::yield();
    }

    // Execute the critical section
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
    std::thread t1(acquire, critical_section, 1);
    std::thread t2(acquire, critical_section, 2);

    t1.join();
    t2.join();

    return 0;
}

/**
 * - Mutual exclusion is preserved 互斥執行
 * - Progress requirement is satisfied 在沒人執行時可以直接進入
 * - 不滿足 bounded waiting requirement
 *   - 雖然compare_and_swap提供了一個公平的機會讓所有線程嘗試獲得鎖，但因為這裡使用了忙等待（busy waiting），若某些線程調度不均可能導致有界等待條件不被完全保證。
 *   - 這是因為沒有控制等待的線程順序或限制。
 *
 * 接下來，我們考慮一個更加複雜的問題，即如何實現一個可以支持多個 Process 的 Mutual Exclusion。
 * 對於多個 Process 的話，可以使用 Bound Waiting Mutual Exclusion with Test-and-Set
 */