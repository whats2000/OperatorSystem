#include <iostream>
#include <windows.h>

/**
 * Write a multi-threaded program that calculates various statistical values for a list of numbers.
 * This program will be passed a series of numbers on the command line and will then create three separate worker threads.
 * One thread will determine the average of the numbers,
 * the second will determine the maximum value, and the third will determine the minimum value.
 *
 * For example, suppose your program is passed the integers 90 81 78 95 79 72 85.
 * The program will report:
 * - The average value is 82.
 * - The minimum value is 72.
 * - The maximum value is 95.
 * The variables representing the average, minimum, and maximum values will be stored globally.
 * The worker threads will set these values, and the parent thread will output the values once the workers have exited.
 */

// Global variables
int average = 0;
int min = 0;
int max = 0;

/**
 * The data structure to pass to the threads
 */
struct ThreadData {
    const int *numbers;
    int size;
};

/**
 * Calculate the average value of the numbers
 * @param param - The ThreadData structure
 * @return - The exit code
 */
DWORD WINAPI calculateAverage(LPVOID param) {
    auto *data = static_cast<ThreadData *>(param);
    double sum = 0;
    for (int i = 0; i < data->size; i++) {
        sum += data->numbers[i];
    }
    average = int(sum / data->size);
    return 0;
}

/**
 * Calculate the minimum value of the numbers
 * @param param - The ThreadData structure
 * @return - The exit code
 */
DWORD WINAPI calculateMin(LPVOID param) {
    auto *data = static_cast<ThreadData *>(param);
    min = data->numbers[0];
    for (int i = 1; i < data->size; i++) {
        if (data->numbers[i] < min) {
            min = data->numbers[i];
        }
    }
    return 0;
}

/**
 * Calculate the maximum value of the numbers
 * @param param - The ThreadData structure
 * @return - The exit code
 */
DWORD WINAPI calculateMax(LPVOID param) {
    auto *data = static_cast<ThreadData *>(param);
    max = data->numbers[0];
    for (int i = 1; i < data->size; i++) {
        if (data->numbers[i] > max) {
            max = data->numbers[i];
        }
    }
    return 0;
}

/**
 * Main function
 * @param argc - The number of arguments. Must be greater than 1
 * @param argv - The arguments array
 * @return - The exit code
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Please provide numbers as arguments." << std::endl;
        return 1;
    }

    int size = argc - 1;
    int *numbers = new int[size];

    for (int i = 0; i < size; i++) {
        numbers[i] = std::stoi(argv[i + 1]);
    }

    ThreadData data = {numbers, size};

    HANDLE threads[3];
    threads[0] = CreateThread(
            nullptr,
            0,
            calculateAverage,
            &data,
            0,
            nullptr
    );
    threads[1] = CreateThread(
            nullptr,
            0,
            calculateMin,
            &data,
            0,
            nullptr
    );
    threads[2] = CreateThread(
            nullptr,
            0,
            calculateMax,
            &data,
            0,
            nullptr
    );

    WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    std::cout << "The average value is " << average << std::endl;
    std::cout << "The minimum value is " << min << std::endl;
    std::cout << "The maximum value is " << max << std::endl;

    delete[] numbers;

    for (auto &thread: threads) {
        CloseHandle(thread);
    }

    return 0;
}
