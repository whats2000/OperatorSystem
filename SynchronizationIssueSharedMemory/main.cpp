/**
 * Note: The code provided below have some issues and is not a complete solution.
 *       This code is intended to be used for educational purposes to help you identify the issues and improve the code.
 */

#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <mutex>

#define BUFFER_SIZE 10
int buffer[BUFFER_SIZE];   // 共用的buffer
int in = 0;                // 下一個要寫入的位置
int out = 0;               // 下一個要讀取的位置
int counter = 0;           // buffer中的項目數

// Mutex for protecting output
std::mutex output_mutex;

// Random number generator
std::random_device rd;
std::mt19937 gen(rd());

/**
 * Producer function that writes to the shared buffer.
 * Continuously produces an item and writes it to the buffer.
 * Waits if buffer is full.
 *
 * @param id The id of the producer thread
 */
void producer(int id) {
    // Random number generator for producing items
    int nextProduced = static_cast<int>(gen() % 100);

    // Wait while buffer is full
    while (counter == BUFFER_SIZE);

    // Produce an item
    buffer[in] = nextProduced;
    in = (in + 1) % BUFFER_SIZE;
    counter++;
    /**
     * 實際上 counter++ 在 CPU 中是分成三個步驟
     *
     * 1. 讀取 counter 的值 (move ax, counter)
     * 2. 將 counter 的值加 1 (add ax, 1)
     * 3. 將加 1 後的值寫入 counter (move counter, ax)
     *
     * 如果在這三個步驟中被其他執行緒打斷，可能會導致 counter 的值不正確
     */
    std::lock_guard<std::mutex> guard(output_mutex);
    std::cout << "Produced by producer " << id << ": " << nextProduced << std::endl;
    /**
     * 這裡的 cout 在 CPU 中也是分成多個步驟
     * 你可以試試不加鎖的情況下執行，這可以更好的理解同步的重要性
     */
}

/**
 * Consumer function that reads from the shared buffer.
 * Continuously consumes an item from the buffer.
 * Waits if buffer is empty.
 */
void consumer(int id) {
    int nextConsumed;
    // Wait while buffer is empty
    while (counter == 0);

    // Consume an item
    nextConsumed = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    counter--;
    /**
     * 同理，counter-- 在 CPU 中是分成三個步驟
     *
     * 1. 讀取 counter 的值 (move bx, counter)
     * 2. 將 counter 的值減 1 (sub bx, 1)
     * 3. 將減 1 後的值寫入 counter (move counter, bx)
     *
     * 這也可能導致 counter 的值不正確
     */

    std::lock_guard<std::mutex> guard(output_mutex);
    std::cout << "Consumed by consumer " << id << ": " << nextConsumed << std::endl;
}

/**
 * Main function to launch producer and consumer threads.
 */
int main() {
    const int num_producers = 3;  // Number of producer threads
    const int num_consumers = 3;  // Number of consumer threads

    std::vector<std::thread> threads;

    // Create producer threads
    threads.reserve(num_producers);
    for (int i = 0; i < num_producers; i++) {
        threads.emplace_back(producer, i + 1);
    }

    // Create consumer threads
    for (int i = 0; i < num_consumers; i++) {
        threads.emplace_back(consumer, i + 1);
    }

    // Join all threads to the main thread
    for (auto &thread: threads) {
        thread.join();
    }

    return 0;
}

/**
 * 考慮事項和潛在問題
 * 並發控制：
 *   程序缺乏對共享變量（in，out，counter）的適當同步。
 *   提供的代碼可能會導致競爭條件，因為這些變量被多個線程無鎖訪問和修改。
 *   這可能導致程序行為不正確。
 *
 * 忙等待：
 *   使用忙等待（while (counter == BUFFER_SIZE); 和 while (counter == 0);）是低效的，因為它不必要地消耗CPU週期。
 *   最好使用條件變量或其他信號機制來有效管理等待。
 *
 * 示例:
 *   1. producer: move ax, counter; -> ax = 5
 *   2. producer: add ax, 1; -> ax = 6
 *   3. context switch
 *   4. consumer: move bx, counter; -> bx = 5
 *   5. consumer: sub bx, 1; -> bx = 4
 *   6. context switch
 *   7. producer: move counter, ax; -> counter = 6
 *   8. context switch
 *   9. consumer: move counter, bx; -> counter = 4
 *
 *   這樣可能導致 counter 的值不正確，這是 Race Condition 的一種情況
 *   即最後 counter 的會依據最後一個操作的值來決定
 *
 *   可以的解決方法是使用 阻止中斷（Interrupt Disable），但這樣會導致系統無法及時響應中斷
 *   我們應該定義這是個 Critical Section 的問題
 *   即:
 *   1. N processes 使用共享資源
 *   2. 每個 process 都有一段 code segment，這段程式碼在執行時，會存取共享資源，這段程式碼稱為 Critical Section
 *   3. 這些 process 之間，不應該同時進入 Critical Section，即 Mutual Exclusion
 *
 *   Critical Section 的需求:
 *   1. Mutual Exclusion: 一次只能有一個 process 進入 Critical Section
 *   2. Progress: 如果沒有 process 進入 Critical Section，則只有那些不在 Critical Section 的 process 可以決定哪個 process 進入 Critical Section
 *   3. Bounded Waiting: 進入 Critical Section 的 process 不能無限等待
 *
 *   有兩種方法取決於 Kernel 是搶先式 (Preemptive) 或非搶先式 (Non-Preemptive)
 *   1. Preemption: 進入 Critical Section 的 process 可能被中斷，並且被其他 process 取代
 *   2. Non-Preemption: 進入 Critical Section 的 process 不會被中斷，直到它自己退出，在 kernel-mode 中不存在 Race Condition
 *
 *   接續到 Peterson's Solution
 */
