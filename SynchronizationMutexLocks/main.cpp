#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>

// 原子布林變數來表示鎖的可用狀態
std::atomic<bool> available(true);

/**
 * 使用忙等待來獲取鎖
 */
void acquire() {
    // Busy wait: 當 available 不為 true 時, 繼續等待
    while (!available.exchange(false)) ;
}

/**
 * 釋放鎖，設置 available 為 true
 */
void release() {
    available.store(true);
}

/**
 * 模擬關鍵區段
 *
 * @param id 執行緒 ID
 */
void critical_section(int id) {
    std::cout << "Thread " << id << " entered critical section." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 模擬耗時操作
    std::cout << "Thread " << id << " exiting critical section." << std::endl;
}

/**
 * 模擬執行緒行為，每個執行緒都會多次嘗試進入關鍵區段
 *
 * @param id 執行緒 ID
 */
void thread_function(int id) {
    for (int i = 0; i < 3; i++) {
        acquire();
        critical_section(id);
        release();
    }
}

int main() {
    const int num_threads = 3;
    std::vector<std::thread> threads;

    // 創建多個執行緒
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(thread_function, i + 1);
    }

    // 等待所有執行緒完成
    for (auto &thread : threads) {
        thread.join();
    }

    return 0;
}

/**
 * 互斥執行:
 * - 由於available的原子操作保證了在任何時候只有一個執行緒能成功地將其從true變為false，這保證了關鍵區段內的互斥執行。
 * - 這是互斥鎖的基本功能。
 *
 * 進展:
 * - 當關鍵區段被釋放（即鎖被釋放）時，任何等待的執行緒都有機會獲取鎖並進入關鍵區段。
 * - 然而，因為採用忙等待，這可能導致CPU資源的不必要消耗。
 *
 * 有界等待:
 * - 這個實現未提供有界等待的保證，因為執行緒的等待時間取決於它們獲取鎖的順序和時機。
 * - 由於是忙等待，某些執行緒可能會因為鎖競爭而經歷較長的等待時間。
 *
 * 上方實現了簡單的自旋鎖，但是這種方式會有 Busy Waiting 的問題，會浪費 CPU 資源。
 *
 * 這時候可以使用 Semaphore 信號量來解決這個問題。
 * 接續到 Semaphore
 */