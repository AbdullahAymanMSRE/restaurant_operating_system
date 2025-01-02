#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>

#define MAX_ORDERS 100
#define MAX_ITEMS 10
#define SHM_SIZE 1024
#define MAX_NOTIFICATIONS 50

typedef struct {
    int item_id;
    char name[50];
    double price;
    int quantity;
} MenuItem;

typedef struct {
    int order_id;
    MenuItem items[MAX_ITEMS];
    int item_count;
    double total_bill;
    int status;  // 0: new, 1: completed, 2: in progress
    time_t creation_time;
    char notification[100];
} Order;

typedef struct {
    Order orders[MAX_ORDERS];
    int order_count;
    MenuItem menu[MAX_ITEMS];
    int menu_size;
    sem_t mutex;
} SharedMemory;

SharedMemory *shm_ptr;
int shm_id;

// Initialize menu items
void init_menu() {
    MenuItem default_menu[] = {
        {1, "Burger", 9.99, 0},
        {2, "Pizza", 12.99, 0},
        {3, "Salad", 7.99, 0},
        {4, "Drink", 2.99, 0}
    };
    
    shm_ptr->menu_size = sizeof(default_menu) / sizeof(MenuItem);
    memcpy(shm_ptr->menu, default_menu, sizeof(default_menu));
}

void init_shared_memory() {
    key_t key = ftok("restaurant_system", 65);
    shm_id = shmget(key, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }
    
    shm_ptr = (SharedMemory *)shmat(shm_id, NULL, 0);
    if ((void *)shm_ptr == (void *)-1) {
        perror("shmat");
        exit(1);
    }
    
    sem_init(&shm_ptr->mutex, 1, 1);
    shm_ptr->order_count = 0;
    init_menu();
}

void update_order_status(int order_id, const char* notification) {
    sem_wait(&shm_ptr->mutex);
    strncpy(shm_ptr->orders[order_id].notification, notification, 99);
    sem_post(&shm_ptr->mutex);
    printf("Order %d: %s\n", order_id, notification);
}

int create_order(int item_ids[], int quantities[], int item_count) {
    sem_wait(&shm_ptr->mutex);
    int order_id = shm_ptr->order_count++;
    Order* order = &shm_ptr->orders[order_id];
    
    order->order_id = order_id;
    order->status = 0;
    order->creation_time = time(NULL);
    order->item_count = item_count;
    order->total_bill = 0;
    
    for (int i = 0; i < item_count; i++) {
        for (int j = 0; j < shm_ptr->menu_size; j++) {
            if (shm_ptr->menu[j].item_id == item_ids[i]) {
                order->items[i] = shm_ptr->menu[j];
                order->items[i].quantity = quantities[i];
                order->total_bill += order->items[i].price * quantities[i];
                break;
            }
        }
    }
    
    sprintf(order->notification, "Order created. Total bill: $%.2f", order->total_bill);
    sem_post(&shm_ptr->mutex);
    
    printf("Order %d created. Total bill: $%.2f\n", order_id, order->total_bill);
    return order_id;
}

void process_order(int order_id) {
    update_order_status(order_id, "Processing order...");
    sleep(5); // Simulate processing time
    
    sem_wait(&shm_ptr->mutex);
    shm_ptr->orders[order_id].status = 1;
    update_order_status(order_id, "Order completed!");
    sem_post(&shm_ptr->mutex);
}

void* kitchen_thread(void* arg) {
    while(1) {
        sem_wait(&shm_ptr->mutex);
        for(int i = 0; i < shm_ptr->order_count; i++) {
            if(shm_ptr->orders[i].status == 0) {
                shm_ptr->orders[i].status = 2;
                sem_post(&shm_ptr->mutex);
                process_order(i);
                break;
            }
        }
        sem_post(&shm_ptr->mutex);
        sleep(1);
    }
    return NULL;
}

void print_order_details(int order_id) {
    Order* order = &shm_ptr->orders[order_id];
    printf("\nOrder #%d Details:\n", order_id);
    printf("Status: %s\n", order->notification);
    printf("Items:\n");
    
    for (int i = 0; i < order->item_count; i++) {
        printf("- %dx %s ($%.2f each)\n", 
               order->items[i].quantity,
               order->items[i].name,
               order->items[i].price);
    }
    printf("Total Bill: $%.2f\n", order->total_bill);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s [create_order|process_orders]\n", argv[0]);
        exit(1);
    }
    
    init_shared_memory();
    
    if(strcmp(argv[1], "create_order") == 0) {
        // Example order creation
        int item_ids[] = {1, 2};
        int quantities[] = {2, 1};
        int order_id = create_order(item_ids, quantities, 2);
        print_order_details(order_id);
    }
    else if(strcmp(argv[1], "process_orders") == 0) {
        pthread_t kitchen_threads[3];
        for(int i = 0; i < 3; i++) {
            pthread_create(&kitchen_threads[i], NULL, kitchen_thread, NULL);
        }
        for(int i = 0; i < 3; i++) {
            pthread_join(kitchen_threads[i], NULL);
        }
    }
    
    return 0;
}
