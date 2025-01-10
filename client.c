#include "shared.h"
#include "order_process.h"

void *order_creation_thread(void *arg)
{
    SharedMemory *shm = (SharedMemory *)arg;

    while (1)
    {
        printf("\033[H\033[J");

        Order new_order = {0};
        new_order.timestamp = time(NULL);
        new_order.status = STATUS_NEW;

        printf("\n===== MENU =====\n");
        for (int i = 0; i < shm->num_menu_items; i++)
        {
            printf("%d. %-20s $%.2f\n",
                   shm->menu[i].id,
                   shm->menu[i].name,
                   shm->menu[i].price);
        }
        printf("===============\n");
        // Add items to order
        while (new_order.num_items < MAX_ITEMS)
        {
            printf("\nEnter item ID (0 to finish): ");
            int item_id;
            if (scanf("%d", &item_id) != 1)
            {
                printf("invalid input, please enter a number");
                while (getchar() != '\n')
                    ; // to neglect all the chars from the buffer
                continue;
            }
            if (item_id == 0)
                break;
            if (item_id < 1 || item_id > shm->num_menu_items)
            {
                printf("invalid input, please enter a valid choice");
                continue;
            }
            printf("Enter quantity: ");
            int quantity;
            if (scanf("%d", &quantity) != 1)
            {
                printf("invalid input, please enter a number");
                while (getchar() != '\n')
                    ; // to neglect all the chars from the buffer
                continue;
            }
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

int main()
{
    SharedMemory *shm = attach_shared_memory();

    // Create threads for order creation and menu display
    pthread_t order_thread, display_thread;
    pthread_create(&order_thread, NULL, order_creation_thread, shm);
    // pthread_create(&display_thread, NULL, menu_display_thread, shm);

    // Wait for order thread to complete
    pthread_join(order_thread, NULL);

    // Cancel display thread
    pthread_cancel(display_thread);
    pthread_join(display_thread, NULL);

    detach_shared_memory(shm);
    return 0;
}
