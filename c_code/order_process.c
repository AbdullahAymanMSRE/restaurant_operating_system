#include "../header_files/shared.h"

// Function that runs as a separate process for each order
void order_process(SharedMemory* shm, int order_index) {
    Order* order = &shm->orders[order_index];
    
    // Initialize the order-specific mutex
    initialize_order_mutex(&order->order_mutex);
    
    // Record the process ID
    order->process_id = getpid();
    
    while (1) {
        // Check if order is completed
        pthread_mutex_lock(&order->order_mutex);
        if (order->status == STATUS_COMPLETED) {
            pthread_mutex_unlock(&order->order_mutex);
            break;
        }
        pthread_mutex_unlock(&order->order_mutex);
        
        // Sleep briefly to prevent CPU hogging
        usleep(100000);  // 100ms
    }
    
    // Cleanup before exit
    pthread_mutex_destroy(&order->order_mutex);
    exit(0);
}

// Function to spawn a new order process
pid_t spawn_order_process(SharedMemory* shm, int order_index) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }
    
    if (pid == 0) {  // Child process
        order_process(shm, order_index);
        // Should never reach here
        exit(1);
    }
    
    return pid;  // Return process ID to parent
}
