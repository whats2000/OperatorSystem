#include <iostream>
#include <pthread.h>
#include <cstdlib>

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

int average = 0;
int min = 0;
int max = 0;

struct ThreadData {
    int *numbers;
    int size;
};

void* calculateAverage(void* param) {
    auto* data = static_cast<ThreadData*>(param);
    int sum = 0;
    for (int i = 0; i < data->size; i++) {
        sum += data->numbers[i];
    }
    average = sum / data->size;
    pthread_exit(nullptr);
    return nullptr;
}

void* calculateMin(void* param) {
    auto* data = static_cast<ThreadData*>(param);
    min = data->numbers[0];
    for (int i = 1; i < data->size; i++) {
        if (data->numbers[i] < min) {
            min = data->numbers[i];
        }
    }
    pthread_exit(nullptr);
    return nullptr;
}

void* calculateMax(void* param) {
    auto* data = static_cast<ThreadData*>(param);
    max = data->numbers[0];
    for (int i = 1; i < data->size; i++) {
        if (data->numbers[i] > max) {
            max = data->numbers[i];
        }
    }
    pthread_exit(nullptr);
    return nullptr;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Please provide numbers as arguments." << std::endl;
        return 1;
    }

    int size = argc - 1;
    int *numbers = new int[size];
    for (int i = 0; i < size; i++) {
        numbers[i] = std::strtol(argv[i + 1], nullptr, 10);
    }

    ThreadData data = {numbers, size};

    pthread_t threads[3];
    pthread_create(&threads[0], nullptr, calculateAverage, &data);
    pthread_create(&threads[1], nullptr, calculateMin, &data);
    pthread_create(&threads[2], nullptr, calculateMax, &data);

    for (auto &thread : threads) {
        pthread_join(thread, nullptr);
    }

    std::cout << "The average value is " << average << std::endl;
    std::cout << "The minimum value is " << min << std::endl;
    std::cout << "The maximum value is " << max << std::endl;

    delete[] numbers;
    return 0;
}
