#include <iostream>
#include <vector>
#include <thread>

/**
 * Peterson's Algorithm 來解決兩個執行緒的競爭問題
 * - Two process solution
 * - Assume that the load and store instructions are atomic; that is, cannot be interrupted
 *
 * The two processes share two variables:
 * - int turn;
 * - Boolean flag[2] 一個指p1另個指p2使用中
 *
 * The variable turn indicates whose turn it is to enter the critical section
 * The flag array is used to indicate if a process is ready to enter the critical section.
 * flag[i] = true implies that process Pi is ready!
 */

// Global shared variables for Peterson's Algorithm
int turn;
bool flag[2] = {false, false};  // Each index represents a thread's intent to enter the critical section

/**
 * Function to execute within the critical section.
 * @param id - The id of the thread.
 */
void critical_section(int id) {
    for (int i = 0; i < 5; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Delay " << i + 1 << " second(s)." << " Thread id: " << id << std::endl;
    }
    std::cout << "Critical section is executed by thread " << id << std::endl;
}

/**
 * Represents a process (thread) that uses Peterson's Algorithm for accessing the critical section.
 * @param id - The id of the thread (either 0 or 1).
 */
void process(int id) {
    int other = id == 0 ? 1 : 0;          // The other thread
    flag[id] = true;                      // Signal intent to enter the critical section
    turn = other;                         // Give the other thread the turn
    while (flag[other] && turn == other); // Wait until it is the thread's turn

    // Critical section
    critical_section(id);

    flag[id] = false;                     // Thread is no longer interested in the critical section
}

/**
 * Main function to launch two threads for Peterson's solution.
 */
int main() {
    std::thread t1(process, 0);
    std::thread t2(process, 1);

    t1.join();
    t2.join();

    return 0;
}

/**
 * Provable that
 * - Mutual exclusion is preserved 互斥執行
 * - Progress requirement is satisfied 在沒人執行時可以直接進入
 * - Bounded-waiting requirement is met 確保沒有無限等
 */

/**
 * 很多現代作業系統為 Critical Section 提供硬體支援
 * - 全部基於 Lock 的概念
 *
 * 1. 單處理器 – 可以禁用中斷，然而有不可廣泛拓展與效能低問題
 * 2. 多處理器 – 使用原子指令，這是一個硬體指令，可以確保指令的原子性
 *  - 測試記憶體字和設置值
 *  - 或者交換兩個記憶單詞的內容
 *
 *  接續到 Test and Set 程式碼解說
*/