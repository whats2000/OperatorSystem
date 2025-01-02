# Operating Systems Concepts

This repository contains several C++ projects demonstrating key concepts in operating systems, including multi-threading and deadlock avoidance. Each project is implemented using CMake for build management.

## Projects

### Banker's Algorithm

This project implements the Banker's algorithm, a resource allocation algorithm that prevents deadlocks in a multi-threaded environment. It simulates a system where multiple customers request and release resources from a bank. The banker grants requests only if they leave the system in a safe state.

#### Key Features:

*   **Multithreading:** Uses multiple threads to simulate customers requesting and releasing resources.
*   **Deadlock Avoidance:** Implements the Banker's algorithm to ensure the system remains in a safe state.
*   **Resource Management:** Tracks resource availability and customer demands using data structures like `available`, `maximum`, `allocation`, and `need` arrays.
*   **Synchronization:** Uses mutex locks to prevent race conditions when accessing shared data.
*   **Command-line arguments**: Takes the number of resources of each type as command-line arguments.

#### Implementation Details:

The project is located in the `BankerAlgorithm` directory and includes the following files:

*   `main.cpp`: The main source code file.
*   `CMakeLists.txt`: CMake build configuration file.
*   `README.md`: Project description and implementation details.

### Multiple Thread Calculation

This project demonstrates multi-threading by calculating the average, minimum, and maximum values of a list of numbers provided as command-line arguments. It includes three implementations:

*   A general C++ implementation using the standard library threads.
*   A Linux-specific implementation using POSIX threads (`pthread`).
*   A Windows-specific implementation using the Windows API threads.

#### Key Features:

*   **Multithreading:** Uses three threads to calculate the average, minimum, and maximum values.
*   **Command-line Input:** Takes a series of numbers as command-line arguments.
*   **Global Variables:** Stores the results of the calculations in global variables.

#### Implementations:

*   **`MultipleThread`:**  
    *   Uses C++ standard library threads (`<thread>`).
    *   Located in the `MultipleThread` directory.
    *   Files: `main.cpp`, `CMakeLists.txt`.
*   **`MultipleThreadLinux`:**
    *   Uses POSIX threads (`pthread.h`).
    *   Located in the `MultipleThreadLinux` directory.
    *    Files: `main.cpp`, `CMakeLists.txt`.
*   **`MultipleThreadWindows`:**
    *   Uses Windows API threads (`windows.h`).
    *   Located in the `MultipleThreadWindows` directory.
    *   Files: `main.cpp`, `CMakeLists.txt`.

### Synchronization Examples

These projects demonstrate various synchronization mechanisms, such as:

*   **Mutex Locks:** Implementing mutual exclusion using mutex locks.
*   **Semaphores:** Using semaphores for thread synchronization and resource management.
*   **Monitors:** Implementing synchronization using monitors.
*   **Test and Set:** Exploring the test-and-set instruction for mutual exclusion.
*   **Compare and Swap:** Demonstrating synchronization using compare-and-swap.
*   **Peterson's Solution:** Implementing a software-based mutual exclusion solution for two threads.
*   **Bounded Waiting:** Implementing mutual exclusion with bounded waiting.
*   **Reader-Writer Problem:** Illustrating solutions for the first reader-writer problem.
*   **Shared Memory Issues:** Highlighting potential issues with shared memory in multithreaded programming.

Each of these projects is located in its respective directory, and each directory contains a `main.cpp` and `CMakeLists.txt` file.
