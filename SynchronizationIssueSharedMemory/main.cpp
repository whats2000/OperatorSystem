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
    std::lock_guard<std::mutex> guard(output_mutex);
    std::cout << "Produced by producer " << id << ": " << nextProduced << std::endl;
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
 * Considerations and Potential Issues
 * Concurrency Control:
 * The program lacks proper synchronization for shared variables (in, out, counter).
 * The provided code will likely result in a race condition because these variables are accessed and modified by multiple threads without locks.
 * This can lead to incorrect program behavior.
 *
 * Busy Waiting:
 * Using busy waiting (while (counter == BUFFER_SIZE); and while (counter == 0);) is inefficient as it consumes CPU cycles unnecessarily.
 * It's better to use condition variables or other signaling mechanisms to efficiently manage waiting.
 */
