#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <sstream>
#include <chrono>
#include <string>

std::mutex resource_mutex;
std::mutex reader_count_mutex;
std::condition_variable cv;
int reader_count = 0;

std::mutex log_mutex;                 // A mutex for protecting the log
std::vector<std::string> log_entries; // Vector to hold log entries

/**
 * @brief Logs a message with a start and end timestamp.
 * @param id - Thread identifier
 * @param role - "Reader" or "Writer"
 * @param start_time - Start time of the operation
 * @param end_time - End time of the operation
 */
void log(int id, const std::string& role, std::chrono::milliseconds start_time, std::chrono::milliseconds end_time) {
    std::stringstream ss;
    ss << role << " " << id << " started at " << start_time.count() << "ms and ended at " << end_time.count() << "ms.";

    std::lock_guard<std::mutex> guard(log_mutex);
    log_entries.push_back(ss.str());
}

/**
 * @brief Reader function
 * @param id - Reader id
 */
void reader(int id) {
    /**
     * Increment the reader count and lock the resource mutex if this is the first reader.
     */
    std::unique_lock<std::mutex> count_lock(reader_count_mutex);
    reader_count++;
    /**
     * Lock the resource mutex if this is the first reader.
     * Which means that this is the first reader and it has to lock the resource mutex.
     * Otherwise, the lock was already acquired by the a reader and the reader can continue reading.
     */
    if (reader_count == 1) {
        resource_mutex.lock();
    }
    /**
     * Unlock the count mutex to allow other readers to increment the count.
     */
    count_lock.unlock();

    /* Read the resource */
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

    log(id, "Reader", start_time, end_time);
    /* End of reading */

    /**
     * Decrement the reader count and unlock the resource mutex if this is the last reader.
     */
    count_lock.lock();
    reader_count--;
    /**
     * Unlock the resource mutex if this is the last reader.
     * Otherwise, the lock was still needed by other readers.
     */
    if (reader_count == 0) {
        resource_mutex.unlock();
        cv.notify_one();
    }
    /**
     * Unlock the count mutex to allow other readers to decrement the count.
     */
    count_lock.unlock();
}

/**
 * @brief Writer function
 * @param id - Writer id
 */
void writer(int id) {
    std::unique_lock<std::mutex> lock(resource_mutex);

    /**
     * Wait until all readers have finished reading.
     */
    cv.wait(lock, [] { return reader_count == 0; });

    /* Write to the resource */
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

    log(id, "Writer", start_time, end_time);
    /* End of writing */
}

int main() {
    std::vector<std::thread> threads;

    threads.reserve(7);
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(reader, i);
    }
    for (int i = 0; i < 2; i++) {
        threads.emplace_back(writer, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Print all log entries
    for (const auto& entry : log_entries) {
        std::cout << entry << std::endl;
    }

    return 0;
}
