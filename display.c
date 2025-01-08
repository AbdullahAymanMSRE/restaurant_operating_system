#include "shared.h"

void* refresh_display(void* arg) {
    SharedMemory* shm = (SharedMemory*)arg;
    
    while (1) {
        // Clear screen (ANSI escape sequence)
        printf("\033[2J\033[H");
        
        printf("\n===== ORDER STATUS DISPLAY =====\n");
        
        pthread_mutex_lock(&shm->orders_mutex);
        for (int i = 0; i < shm->num_orders; i++) {
            Order* order = &shm->orders[i];
            if (order->status != STATUS_COMPLETED) {
                printf("\nOrder #%d (Table %d)\n", order->order_id, order->table_number);
                printf("Status: ");
                switch (order->status) {
                    case STATUS_NEW: 
                        printf("Order received\n");
                        break;
                    case STATUS_IN_PROGRESS:
                        printf("Being prepared\n");
                        break;
                    case STATUS_READY:
                        printf("READY FOR PICKUP!\n");
                        break;
                    default:
                        printf("Unknown\n");
                }
            }
        }
        pthread_mutex_unlock(&shm->orders_mutex);
        
        printf("\n==============================\n");
        sleep(5);  // Refresh every 5 seconds
    }
    return NULL;
}

int main() {
    SharedMemory* shm = attach_shared_memory();
    
    pthread_t display_thread;
    pthread_create(&display_thread, NULL, refresh_display, shm);
    
    // Wait for Ctrl+C
    signal(SIGINT, exit);
    pause();
    
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    detach_shared_memory(shm);
    return 0;
}
