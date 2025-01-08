#include "shared.h"
#include "order_process.h"

void* order_creation_thread(void* arg) {
    SharedMemory* shm = (SharedMemory*)arg;
    
    while (1) {
        Order new_order = {0};
        new_order.timestamp = time(NULL);
        new_order.status = STATUS_NEW;
        
        printf("Enter table number (0 to exit thread): ");
        scanf("%d", &new_order.table_number);
        
        if (new_order.table_number == 0) {
            break;
        }

        // Add items to order
        while (new_order.num_items < MAX_ITEMS) {
            printf("\nEnter item ID (0 to finish): ");
            int item_id;
            scanf("%d", &item_id);
            
            if (item_id == 0) break;
            
            printf("Enter quantity: ");
            int quantity;
            scanf("%d", &quantity);

            new_order.items[new_order.num_items].menu_item_id = item_id;
            new_order.items[new_order.num_items].quantity = quantity;
            new_order.num_items++;
        }

        // Add order to shared memory with proper synchronization
        pthread_mutex_lock(&shm->orders_mutex);
        int order_index = shm->num_orders;
        new_order.order_id = order_index + 1;
        shm->orders[order_index] = new_order;
        shm->num_orders++;
        
        // Spawn process for this order
        pid_t order_pid = spawn_order_process(shm, order_index);
        shm->orders[order_index].process_id = order_pid;
        
        pthread_mutex_unlock(&shm->orders_mutex);

        printf("\nOrder #%d created successfully!\n", new_order.order_id);
    }
    
    return NULL;
}

void* menu_display_thread(void* arg) {
    SharedMemory* shm = (SharedMemory*)arg;
    
    while (1) {
        printf("\n===== MENU =====\n");
        for (int i = 0; i < shm->num_menu_items; i++) {
            printf("%d. %-20s $%.2f\n", 
                shm->menu[i].id, 
                shm->menu[i].name, 
                shm->menu[i].price);
        }
        printf("===============\n");
        sleep(5);  // Refresh menu every 5 seconds
    }
    return NULL;
}

int main() {
    SharedMemory* shm = attach_shared_memory();

	initialize_shared_memory(shm);

    
    // Create threads for order creation and menu display
    pthread_t order_thread, display_thread;
    pthread_create(&order_thread, NULL, order_creation_thread, shm);
    pthread_create(&display_thread, NULL, menu_display_thread, shm);
    
    // Wait for order thread to complete
    pthread_join(order_thread, NULL);
    
    // Cancel display thread
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);
    
    detach_shared_memory(shm);
    return 0;
}
