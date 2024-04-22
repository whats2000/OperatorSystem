/**
 * Semaphore
 * - 不需要繁忙等待的同步工具
 * - Semaphore S 是整數變數
 * - 只有兩個操作：wait(S) 和 signal(S)
 *   - 一開始叫做 P(S) 和 V(S)
 * - Semaphore 可以用來解決很多不同的問題
 *
 * Semaphore 操作
 * - wait(S)：S = S - 1，如果 S < 0，則執行緒進入等待狀態
 * - signal(S)：S = S + 1，如果 S <= 0，則有一個等待的執行緒被喚醒
 *
 * Semaphore 區分為兩種
 * - Counting semaphore: S 可以是任意整數
 * - Binary semaphore: S 只能是 0 或 1 (與 mutex 類似)
 * - 可以使用 Binary semaphore 實作 Counting semaphore
 *
 * Semaphore 必須滿足的條件
 * - 必須保證沒有兩個進程可以同時在同一個信號量上執行 wait 與 signal 操作
 * => 因此，實現成為 Critical Section Problem 的一部分，wait 和 signal 必須放在 Critical Section 內
 *    - 但是這邊一樣會有 Busy Waiting 的問題，不過程式碼會簡單很多，且如果 Critical Section 很少被占用，則很少會有 Busy Waiting
 *    - 對於 Critical Section 可能很長（幾分鐘甚至幾小時）或幾乎總是被佔用的應用程式，存在完全不同的情況。在這種情況下，繁忙的等待效率極低。
 *
 * Semaphore 的另個實現方式
 * - 每個信號量都有一個相關的等待佇列
 * - 等待佇列中的每個條目都有兩個數據項：
 *   - pointer: 指向清單中下一條記錄的指標
 *   - value: 整數類型
 * - 兩個對應的 System Call：
 *   - block(S)：將呼叫執行緒放到 S 的等待佇列中
 *   - wakeup(S)：從 S 的等待佇列中移除一個執行緒，並將其放到就緒佇列中
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <sstream>

class Semaphore {
    int value;            // Tracks the number of available slots
    std::mutex mtx;       // Mutex to protect value and condition variable
    std::condition_variable cv;

public:
    explicit Semaphore(int initValue) : value(initValue) {}

    /**
     * Wait operation
     */
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        // Use a loop to handle spurious wake-ups
        while (value <= 0) {
            cv.wait(lock);
        }
        value--;          // Decrement the number of available slots only if not waiting
    }

    /**
     * Signal operation
     */
    void signal() {
        std::unique_lock<std::mutex> lock(mtx);
        value++;          // Increment the number of available slots
        cv.notify_one();  // Notify one waiting thread
    }
};

std::vector<std::string> logs;
std::mutex log_mutex;     // Protect access to logs

/**
 * Simulate some work in the critical section
 *
 * @param id - The id of the thread entering the critical section
 */
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

/**
 * Task that will be executed by each thread
 *
 * @param id - The id of the thread
 * @param sem - The semaphore to use
 */
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
 * 互斥執行:
 * - 信號量確保了關鍵區段內的互斥執行，因為它限制了同時進入關鍵區段的執行緒數量。
 * - 在這個例子中，信號量的初始值設為2，意味著最多兩個執行緒可以同時進入關鍵區段。
 *
 * 進展:
 * - 當信號量中有可用槽位時，等待的執行緒將被允許進入關鍵區段。
 * - 這保證了當條件滿足時，執行緒能夠進展。
 *
 * 有界等待:
 * - 通過使用條件變量和合適的喚醒策略（notify_one），信號量實現確保了等待的執行緒不會無限期地等待。
 * - 每當信號操作發生，都會有一個等待的執行緒被喚醒，這有助於保證等待的有界性。
 *
 * 補充:
 * - 可以使用 sleep 與 wakeup 來實現信號量的 wait 與 signal 操作，避免 while 循環中的繁忙等待。
*/