#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <random>

/**
 * Banker's Algorithm
 *
 * For this project, you will write a multithreading program that implements the banker's algorithm discussed in Section 7.5.3.
 * Several customers request and release resources from the bank.
 * The banker will grant a request only if it leaves the system in a safe state.
 * A request that leaves the system in an unsafe state will be denied.
 * This programming assignment combines three separate topics:
 * (1) multithreading, (2) preventing race conditions, and (3) deadlock avoidance.
 *
 * The Banker:
 * The banker will consider requests from customers for im resources types, as outlined in Section 7.5.3.
 * The banker will keep track of the resources using the following data structures:
 *
 * These may be any values >= 0
 * ```cpp
 * #define NUMBER OF CUSTOMERS 5
 * #define NUMBER OF RESOURCES 3
 * ```
 *
 * The available amount of each resource
 * ```cpp
 * int available [NUMBER OF RESOURCES];
 * ```
 *
 * The maximum demand of each customer
 * ```cpp
 * int maximum[NUMBER OF CUSTOMERS][NUMBER OF RESOURCES];
 * ```
 *
 * The amount currently allocated to each customer
 * ```cpp
 * int allocation[NUMBER OF CUSTOMERS][NUMBER OF RESOURCES];
 * ```
 *
 * The remaining need of each customer
 * ```cpp
 * int need[NUMBER OF CUSTOMERS][NUMBER OF RESOURCES];
 * ```
 *
 * The Customers:
 * Create customer threads that request and release resources from the bank.
 * The customers will continually loop, requesting and then releasing random numbers of resources.
 * The customers' requests for resources will be bounded by their respective values in the need array.
 * The banker will grant a request if it satisfies the safety algorithm outlined in Section 7.5.3.1.
 * If a request does not leave the system in a safe state, the banker will deny it.
 * Function prototypes for requesting and releasing resources are as follows:
 * ```cpp
 * int request resources (int customer_num, int request[]);
 * int release resources (int customer_num, int release[]);
 * ```
 * These two functions should return 0 if successful (the request has been granted) and-1 if unsuccessful.
 * Multiple threads (customers) will concurrently access shared data through these two functions.
 * Therefore, access must be control through mutex locks to prevent race conditions.
 * Both the Pthreads and Windows APIs provide mutex locks.
 * The use of Pthreads mutex locks is covered in Section 6.9.4;
 * Mutex locks for Windows systems are described in the project entitled "Producer-Consumer Problem" at the end of Chapter 6.
 *
 * Implementation:
 * You should invoke your program by passing the number of resources of each type on the command line.
 * For example, if there were three resource types with ten instances of the first type, five of the second type, and seven of the third type, you would invoke your program follows:
 * ```bash
 * ./a.out 10 5 7
 * ```
 * The available array would be initialized to these values.
 * You may initialize the maximum array (which holds the maximum demand of each customer) any method you find convenient.
 */

# define NUMBER_OF_CUSTOMERS 5
# define NUMBER_OF_RESOURCES 3

int available[NUMBER_OF_RESOURCES] = {0};
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {0};
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {0};
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {0};

int request_resources(int customer_num, const int request[]);
int release_resources(int customer_num, const int release[]);

bool is_safe();

void customer_thread(int customer_num);
void print_state();

// Mutex lock for the shared data
std::mutex mtx;

// Mutex lock for output
std::mutex output_mtx;

/**
 * Is safe function
 * This function checks if the system is in a safe state after the request is granted
 *
 * 1. Let Work and Finish be vectors of length m and n, respectively.
 * Initialize:
 * Work = Available
 * Finish[i] = false for i = 0, 1, …, n- 1
 *
 * 2. Find an i such that both:
 *  (a) Finish [i] = false
 *  (b) Need_i <= Work
 *  If no such i exists, go to step 4
 *
 * 3. Work = Work + Allocation_i
 *  Finish[i] = true
 *  go to step 2
 *
 * 4. If Finish [i] == true for all i, then the system is in a safe state
 *
 */
bool is_safe() {
    std::vector<int> work(NUMBER_OF_RESOURCES);
    std::vector<bool> finish(NUMBER_OF_CUSTOMERS, false);

    // Copy the available resources to the work vector
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
        work[i] = available[i];
    }

    // Algorithm to check if the system is in a safe state
    while (true) {
        bool found = false;

        // Iterate over all customers
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            // Skip if the customer is already finished
            if (finish[i]) continue;

            bool feasible = true;

            // Check if the resources needed by this customer can be satisfied with the available work
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                if (need[i][j] > work[j]) {
                    feasible = false;
                    break;
                }
            }

            // If the resources is not feasible, skip this customer
            if (!feasible) continue;

            // If the resources can be satisfied, pretend to allocate them (simulating process completion)
            for (int j2 = 0; j2 < NUMBER_OF_RESOURCES; j2++) {
                work[j2] += allocation[i][j2];
            }

            // Mark this customer as finished
            finish[i] = true;

            // Mark that we found a customer that can proceed
            found = true;
        }

        // If no unmarked processes are left that can proceed, break the loop
        if (!found) {
            break;
        }
    }

    // Check if all processes are marked as finished
    if (!std::all_of(finish.begin(), finish.end(), [](bool f){ return f; })) {
        return false;
    }

    return true;
}

/**
 * Request resources function
 * This function is called by the customer threads to request resources
 *
 * Request = request vector for process P_i.
 * If Request_i[j] = k then process Pi wants k instances of resource type R_j
 *
 * 1. If Request_i <= Need_i go to step 2.
 *    Otherwise, raise error condition, since process has exceeded its maximum claim
 *
 * 2. If Request_i <= Available, go to step 3.
 *    Otherwise Pi  must wait, since resources are not available
 *
 * 3. Pretend to allocate requested resources to Pi by modifying the state as follows:
 *    Available = Available – Request;
 *    Allocation_i = Allocation_i + Request_i;
 *    Need_i = Need_i – Request_i;
 *
 * If safe => the resources are allocated to P_i
 * If unsafe => P_i must wait, and the old resource-allocation state is restored
 *
 *
 * @param customer_num The customer number
 * @param request The request array
 * @return 0 if successful, -1 if unsuccessful
 */
int request_resources(int customer_num, const int request[]) {
    // Lock mutex to prevent other threads from entering
    std::lock_guard<std::mutex> lock(mtx);

    // Step 1: Check if the request is less than or equal to the need
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        // Check if the request is less than or equal to the need
        if (request[j] <= need[customer_num][j]) continue;

        {
            std::lock_guard<std::mutex> output_lock(output_mtx);
            std::cout << "Error: Process has exceeded its maximum claim." << std::endl;
        }

        // Process has exceeded its claim
        return -1;
    }

    // Step 2: Check if the resources are available
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        // Check if the request is less than or equal to the available resources
        if (request[j] <= available[j]) continue;
        {
            std::lock_guard<std::mutex> output_lock(output_mtx);
            std::cout << "Resources are not available for customer " << customer_num << std::endl;
        }

        // Resources are not available, must wait
        return -1;
    }

    // Step 3: Pretend to allocate resources
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        available[j] -= request[j];
        allocation[customer_num][j] += request[j];
        need[customer_num][j] -= request[j];
    }

    // Check if the new state is safe
    if (is_safe()) {
        {
            std::lock_guard<std::mutex> output_lock(output_mtx);
            std::cout << "Resources allocated to customer " << customer_num << std::endl;
        }

        // The resources are allocated
        return 0;
    }

    // Rollback the allocation if it's not safe
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        available[j] += request[j];
        allocation[customer_num][j] -= request[j];
        need[customer_num][j] += request[j];
    }

    {
        std::lock_guard<std::mutex> output_lock(output_mtx);
        std::cout << "Resources request by customer " << customer_num << " leads to unsafe state, rolling back."
                  << std::endl;
    }

    // The resources cannot be allocated, rollback
    return -1;
}

/**
 * Release resources function
 * This function is called by the customer threads to release resources
 *
 * @param customer_num The customer number
 * @param release The release array
 * @return 0 if successful, -1 if unsuccessful
 */
int release_resources(int customer_num, const int release[]) {
    // Lock mutex to prevent other threads from modifying shared resources simultaneously
    std::lock_guard<std::mutex> lock(mtx);

    // Check if the release request is valid (i.e., no release amount exceeds the current allocation)
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        // Check if the release amount is less than or equal to the current allocation
        if (release[j] <= allocation[customer_num][j]) continue;

        {
            std::lock_guard<std::mutex> output_lock(output_mtx);
            std::cout << "Error: Attempt to release more resources than allocated for customer " << customer_num
                      << std::endl;
        }

        // Invalid release request
        return -1;
    }

    // Update resource tracking arrays
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        available[j] += release[j];
        allocation[customer_num][j] -= release[j];
        need[customer_num][j] += release[j];
    }

    {
        std::lock_guard<std::mutex> output_lock(output_mtx);
        std::cout << "Resources released by customer " << customer_num << std::endl;
    }

    // Successful release
    return 0;
}

/**
 * Print state function
 * This function prints the current state of the system
 */
void print_state() {
    std::lock_guard<std::mutex> lock(output_mtx);
    std::cout << "Current State of System:\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "  Allocation     Need       Maximum     Available\n";
    std::cout << "     A B C   |   A B C   |   A B C   |   A B C\n";
    std::cout << "----------------------------------------------------------------\n";

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        std::cout << "P" << i << " ";
        // Print Allocation for each customer
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j) {
            std::cout << std::setw(2) << allocation[i][j] << " ";
        }
        std::cout << " | ";
        // Print Need for each customer
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j) {
            std::cout << std::setw(2) << need[i][j] << " ";
        }
        std::cout << " | ";
        // Print Maximum demand for each customer
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j) {
            std::cout << std::setw(2) << maximum[i][j] << " ";
        }
        std::cout << " | ";
        // Print Available resources (only on the first line)
        if (i == 0) {
            for (int j : available) {
                std::cout << std::setw(2) << j << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------------------------------\n";
}

/**
 * Customer thread function
 * This function simulates a customer requesting and releasing resources
 *
 * @param customer_num The customer number
 */
void customer_thread(int customer_num) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::uniform_int_distribution<>> distributions;
    distributions.reserve(NUMBER_OF_RESOURCES);

    // Prepare distributions for each resource according to the max need
    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
        distributions.emplace_back(0, need[customer_num][i]);
    }

    while (true) {
        int request[NUMBER_OF_RESOURCES];
        bool all_needs_met = true;

        // Create a request that is smaller or equal to the need
        for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
            if (need[customer_num][i] > 0) {
                // The max request here should be the lesser of the current need or what's left of the max allowance
                int max_request = std::min(need[customer_num][i], maximum[customer_num][i] - allocation[customer_num][i]);
                request[i] = distributions[i](gen) % (max_request + 1);
                all_needs_met = false;
            } else {
                request[i] = 0;
            }
        }

        // Exit loop if all needs are met
        if (all_needs_met) {
            {
                std::lock_guard<std::mutex> output_lock(output_mtx);
                std::cout << "Customer " << customer_num << " has all needs met and will exit." << std::endl;
            }
            release_resources(customer_num, allocation[customer_num]);
            break;
        }

        if (request_resources(customer_num, request) == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            print_state();
        } else {
            {
                std::lock_guard<std::mutex> output_lock(output_mtx);
                std::cout << "Customer " << customer_num << " must wait, resources not granted." << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char* argv[]) {
    if (argc < NUMBER_OF_RESOURCES + 1) {
        std::cerr << "Usage: " << argv[0] << " <resources...>" << std::endl;
        return 1;
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) {
        available[i] = std::strtol(argv[i + 1], nullptr, 10);
    }

    // Set maximum needs, and initial allocation
    int fixed_maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
            {7, 5, 3},
            {3, 2, 2},
            {9, 0, 2},
            {2, 2, 2},
            {4, 3, 3}
    };

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j) {
            maximum[i][j] = fixed_maximum[i][j];
            need[i][j] = maximum[i][j];  // Initial need is maximum need
            allocation[i][j] = 0;  // Initial allocation is 0
        }
    }

    std::vector<std::thread> threads;
    threads.reserve(NUMBER_OF_CUSTOMERS);
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i) {
        threads.emplace_back(customer_thread, i);
    }

    for (auto& th : threads) {
        th.join();  // Wait for all threads to finish
    }

    std::cout << "All customers have finished. Exiting program." << std::endl;
    return 0;
}
