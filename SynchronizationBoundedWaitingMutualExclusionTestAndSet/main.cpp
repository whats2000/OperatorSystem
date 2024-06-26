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

/**
 * 互斥執行:
 * - lock保證了在任意時刻只有一個線程可以進入關鍵區段。
 * - 由於test_and_set確保了lock變量的修改是原子的，這段程式碼滿足互斥的要求。
 *
 * 進展:
 * - 當一個線程釋放鎖時，它會尋找並激活下一個在等待隊列中的線程。
 * - 如果沒有其他線程在等待，則鎖被完全釋放。
 * - 這種機制確保了每個需要進入關鍵區段的線程最終都有機會進入。
 *
 * 有界等待:
 * - 這段程式碼試圖通過尋找等待隊列中的下一個線程來保證有界等待。
 * - 但是，由於循環尋找下一個等待的線程可能導致某些線程被延遲激活，尤其是在高負載或不平等調度的環境中，因此這種實現可能無法完全保證所有線程的等待時間是有界的。
 *
 * 但是這些解決方案很複雜，應用程式程式師通常無法訪問 (雖然在 C++ 中可以使用 std::atomic 類型)。
 * => 操作系統設計人員構建軟體工具來解決關鍵部分問題
 *
 * 一個典型的解決方案是使用 Mutex Locks (互斥鎖)。
 * - 互斥鎖是一種同步原語，用於保護共享資源。
 * - 互斥鎖是用 acquire() 和 release() 操作來操作的。
 *   => Boolean 變數 lock 用於表示互斥鎖的狀態。
 * - 呼叫 acquire() 與 release() 必須是原子的。
 *   => 通常是用硬體的原子指令來實現。
 *
 * 但是，這種方法有一個問題：忙碌等待。
 *   => 此類鎖又稱 Spinning Locks (自旋鎖)。
 *
 * 接續到示例 Mutex Locks (互斥鎖)。
 */