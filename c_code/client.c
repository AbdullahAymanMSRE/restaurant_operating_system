#include "../header_files/shared.h"
#include "../header_files/order_process.h"

MenuItem *get_menu_item(SharedMemory *shm, int item_id)
{
    for (int i = 0; i < shm->num_menu_items; i++)
    {
        if (shm->menu[i].id == item_id)
        {
            return &shm->menu[i];
        }
    }
    return NULL;
}

int create_order(SharedMemory *shm, int item_ids[], int quantities[], int item_count)
{
    Order new_order = {0};
    new_order.timestamp = time(NULL);
    new_order.status = STATUS_NEW;
    new_order.total_bill = 0;

    // Add items to order
    while (new_order.num_items < MAX_ITEMS && new_order.num_items < item_count)
    {
        int item_id = item_ids[new_order.num_items];
        int quantity = quantities[new_order.num_items];
        MenuItem *item = get_menu_item(shm, item_id);

        new_order.items[new_order.num_items].menu_item_id = item_id;
        new_order.items[new_order.num_items].quantity = quantity;
        new_order.total_bill += quantity * item->price;
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

    return order_index;
}

void print_menu(SharedMemory *shm)
{
    for (int i = 0; i < shm->num_menu_items; i++)
    {
        printf("%d. %-20s $%.2f\n",
               shm->menu[i].id,
               shm->menu[i].name,
               shm->menu[i].price);
    }
}

void print_menu_size(SharedMemory *shm)
{
    printf("%d\n", shm->num_menu_items);
}

void print_order_details(SharedMemory *shm, int order_index)
{
    Order order = shm->orders[order_index];
    printf("\nOrder #%d Details:\n", order.order_id);
    printf("Items:\n");

    for (int i = 0; i < order.num_items; i++)
    {

        MenuItem *item = get_menu_item(shm, order.items[i].menu_item_id);

        printf("- %dx %s ($%.2f each)\n",
               order.items[i].quantity,
               item->name,
               item->price);
    }
    printf("Total Bill: $%.2f\n", order.total_bill);
}

int main(int argc, char *argv[])
{
    SharedMemory *shm = attach_shared_memory();

    if (argc > 1)
    {
        if (strcmp(argv[1], "--menu") == 0)
        {
            print_menu(shm);
        }
        else if (strcmp(argv[1], "--size") == 0)
        {
            print_menu_size(shm);
        }
        else if (strcmp(argv[1], "--order") == 0)
        {
            if (argc < 4 || (argc - 2) % 2 != 0)
            {
                printf("Invalid number of arguments for create_order\n");
                exit(1);
            }

            int item_count = (argc - 2) / 2;
            int item_ids[MAX_ITEMS];
            int quantities[MAX_ITEMS];

            for (int i = 0; i < item_count; i++)
            {
                item_ids[i] = atoi(argv[2 + i]);
                quantities[i] = atoi(argv[2 + item_count + i]);
            }

            int order_index = create_order(shm, item_ids, quantities, item_count);
            print_order_details(shm, order_index);
        }
    }

    detach_shared_memory(shm);
    return 0;
}
