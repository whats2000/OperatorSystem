#include <iostream>
#include <thread>

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
 * Calculate the average value of the numbers
 * @param numbers - The array of numbers
 * @param size - The size of the array
 */
void calculateAverage(const int numbers[], const int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += numbers[i];
    }
    average = int(sum / size);
}

/**
 * Calculate the minimum value of the numbers
 * @param numbers - The array of numbers
 * @param size - The size of the array
 */
void calculateMin(const int numbers[], const int size) {
    min = numbers[0];
    for (int i = 1; i < size; i++) {
        if (numbers[i] < min) {
            min = numbers[i];
        }
    }
}

/**
 * Calculate the maximum value of the numbers
 * @param numbers - The array of numbers
 * @param size - The size of the array
 */
void calculateMax(const int numbers[], const int size) {
    max = numbers[0];
    for (int i = 1; i < size; i++) {
        if (numbers[i] > max) {
            max = numbers[i];
        }
    }
}

/**
 * Main function
 * @param argc - The number of arguments. Must be greater than 1
 * @param argv - The arguments array
 * @return - The exit code
 */
int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cout << "Please provide numbers as arguments." << std::endl;
        return 1;
    }

    int size = argc - 1;
    int* numbers = new int[size];

    for(int i = 0; i < size; i++) {
        numbers[i] = std::stoi(argv[i+1]);
    }

    std::thread t1(calculateAverage, numbers, size);
    std::thread t2(calculateMin, numbers, size);
    std::thread t3(calculateMax, numbers, size);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "The average value is " << average << std::endl;
    std::cout << "The minimum value is " << min << std::endl;
    std::cout << "The maximum value is " << max << std::endl;

    delete[] numbers;

    return 0;
}
