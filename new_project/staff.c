#include "shared.h"

void* order_monitor_thread(void* arg) {
    SharedMemory* shm = (SharedMemory*)arg;
    
    while (1) {
        printf("\n===== CURRENT ORDERS =====\n");
        
        pthread_mutex_lock(&shm->orders_mutex);
        for (int i = 0; i < shm->num_orders; i++) {
            Order* order = &shm->orders[i];
            
            // Lock individual order mutex for reading
            pthread_mutex_lock(&order->order_mutex);
            if (order->status != STATUS_COMPLETED) {
                printf("\nOrder #%d (Table %d)\n", order->order_id, order->table_number);
                printf("Status: ");
                switch (order->status) {
                    case STATUS_NEW: printf("New\n"); break;
                    case STATUS_IN_PROGRESS: printf("In Progress\n"); break;
                    case STATUS_READY: printf("Ready\n"); break;
                    default: printf("Unknown\n");
                }
                
                printf("Items:\n");
                for (int j = 0; j < order->num_items; j++) {
                    for (int k = 0; k < shm->num_menu_items; k++) {
                        if (shm->menu[k].id == order->items[j].menu_item_id) {
                            printf("- %dx %s\n", 
                                order->items[j].quantity, 
                                shm->menu[k].name);
                            break;
                        }
                    }
                }
            }
            pthread_mutex_unlock(&order->order_mutex);
        }
        pthread_mutex_unlock(&shm->orders_mutex);
        
        printf("========================\n");
        sleep(3);  // Refresh every 3 seconds
    }
    return NULL;
}

void* status_update_thread(void* arg) {
    SharedMemory* shm = (SharedMemory*)arg;
    
    while (1) {
        printf("\nEnter order ID to update (0 to exit): ");
        int order_id;
        scanf("%d", &order_id);
        
        if (order_id == 0) break;

        printf("Update status to:\n");
        printf("1. In Progress\n");
        printf("2. Ready\n");
        printf("3. Completed\n");
        printf("Choice: ");
        
        int choice;
        scanf("%d", &choice);

        pthread_mutex_lock(&shm->orders_mutex);
        for (int i = 0; i < shm->num_orders; i++) {
            if (shm->orders[i].order_id == order_id) {
                pthread_mutex_lock(&shm->orders[i].order_mutex);
                
                switch (choice) {
                    case 1:
                        shm->orders[i].status = STATUS_IN_PROGRESS;
                        break;
                    case 2:
                        shm->orders[i].status = STATUS_READY;
                        break;
                    case 3:
                        shm->orders[i].status = STATUS_COMPLETED;
                        // Send SIGTERM to the order process
                        kill(shm->orders[i].process_id, SIGTERM);
                        break;
                }
                
                pthread_mutex_unlock(&shm->orders[i].order_mutex);
                break;
            }
        }
        pthread_mutex_unlock(&shm->orders_mutex);
    }
    
    return NULL;
}

int main() {
    SharedMemory* shm = attach_shared_memory();
    
    // Create threads for monitoring and updating orders
    pthread_t monitor_thread, update_thread;
    pthread_create(&monitor_thread, NULL, order_monitor_thread, shm);
    pthread_create(&update_thread, NULL, status_update_thread, shm);
    
    // Wait for update thread to complete
    pthread_join(update_thread, NULL);
    
    // Cancel monitor thread
    pthread_cancel(monitor_thread);
    pthread_join(monitor_thread, NULL);
    
    detach_shared_memory(shm);
    return 0;
}
