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
#define MAX_THREADS 50
#define MAX_OBSERVERS 20
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

typedef void (*OrderCallback)(int order_id, const char* status);

typedef struct {
    int order_id;
    OrderCallback callback;
} OrderObserver;

typedef struct {
    Order orders[MAX_ORDERS];
    int order_count;
    MenuItem menu[MAX_ITEMS];
    int menu_size;
    pthread_mutex_t mutex;
    pthread_cond_t order_cond;
    OrderObserver observers[MAX_OBSERVERS];
    int observer_count;
    pthread_mutex_t observer_mutex;
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

void notify_observers(int order_id, const char* status) {
    pthread_mutex_lock(&shm_ptr->observer_mutex);
    for(int i = 0; i < shm_ptr->observer_count; i++) {
        if(shm_ptr->observers[i].order_id == order_id || 
           shm_ptr->observers[i].order_id == -1) {  // -1 means observe all orders
            shm_ptr->observers[i].callback(order_id, status);
        }
    }
    pthread_mutex_unlock(&shm_ptr->observer_mutex);
}
void init_shared_memory() {
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
        
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        
        pthread_mutex_init(&shm_ptr->mutex, &attr);
        pthread_mutex_init(&shm_ptr->observer_mutex, &attr);
        pthread_cond_init(&shm_ptr->order_cond, NULL);
        
        shm_ptr->order_count = 0;
        shm_ptr->observer_count = 0;
        init_menu();
        
        pthread_mutexattr_destroy(&attr);
    }
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
    pthread_mutex_lock(&shm_ptr->mutex);
    strncpy(shm_ptr->orders[order_id].notification, notification, 99);
    pthread_mutex_unlock(&shm_ptr->mutex);
    
    notify_observers(order_id, notification);
    pthread_cond_broadcast(&shm_ptr->order_cond);
}

void register_observer(int order_id, OrderCallback callback) {
    pthread_mutex_lock(&shm_ptr->observer_mutex);
    if(shm_ptr->observer_count < MAX_OBSERVERS) {
        shm_ptr->observers[shm_ptr->observer_count].order_id = order_id;
        shm_ptr->observers[shm_ptr->observer_count].callback = callback;
        shm_ptr->observer_count++;
    }
    pthread_mutex_unlock(&shm_ptr->observer_mutex);
}

void unregister_observer(OrderCallback callback) {
    pthread_mutex_lock(&shm_ptr->observer_mutex);
    for(int i = 0; i < shm_ptr->observer_count; i++) {
        if(shm_ptr->observers[i].callback == callback) {
            // Remove this observer by shifting the rest down
            for(int j = i; j < shm_ptr->observer_count - 1; j++) {
                shm_ptr->observers[j] = shm_ptr->observers[j + 1];
            }
            shm_ptr->observer_count--;
            break;
        }
    }
    pthread_mutex_unlock(&shm_ptr->observer_mutex);
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
    
    pthread_mutex_lock(&shm_ptr->mutex);
    shm_ptr->orders[order_id].status = 1;
    pthread_mutex_unlock(&shm_ptr->mutex);
    
    update_order_status(order_id, "Order completed!");
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
        pthread_mutex_lock(&shm_ptr->mutex);
        while(1) {
            int order_to_process = -1;
            for(int i = 0; i < shm_ptr->order_count; i++) {
                if(shm_ptr->orders[i].status == 0) {
                    order_to_process = i;
                    shm_ptr->orders[i].status = 2;
                    break;
                }
            }
            
            if(order_to_process == -1) {
                pthread_cond_wait(&shm_ptr->order_cond, &shm_ptr->mutex);
            } else {
                pthread_mutex_unlock(&shm_ptr->mutex);
                process_order(order_to_process);
                break;
            }
        }
    }
    return NULL;
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

void show_system_status() {
    sem_wait(&shm_ptr->mutex);
    printf("\nSystem Status\n");
    printf("-------------\n");
    printf("Total Orders: %d\n", shm_ptr->order_count);
    
    int new_orders = 0, in_progress = 0, completed = 0;
    for(int i = 0; i < shm_ptr->order_count; i++) {
        switch(shm_ptr->orders[i].status) {
            case 0: new_orders++; break;
            case 1: completed++; break;
            case 2: in_progress++; break;
        }
    }
    
    printf("New Orders: %d\n", new_orders);
    printf("In Progress: %d\n", in_progress);
    printf("Completed: %d\n", completed);
    
    // Show active kitchen threads
    printf("\nKitchen Status:\n");
    char cmd[100];
    sprintf(cmd, "ps -T -p %d | grep kitchen", getpid());
    system(cmd);
    sem_post(&shm_ptr->mutex);
}

void show_memory_usage() {
    printf("\nMemory Usage Stats\n");
    printf("------------------\n");
    
    // Show shared memory segment info
    struct shmid_ds shm_info;
    if (shmctl(shm_id, IPC_STAT, &shm_info) == -1) {
        perror("shmctl");
        return;
    }
    
    printf("Shared Memory:\n");
    printf("- Segment Size: %lu bytes\n", shm_info.shm_segsz);
    printf("- Number of attached processes: %lu\n", shm_info.shm_nattch);
    
    // Calculate memory used by orders
    size_t orders_memory = sizeof(Order) * shm_ptr->order_count;
    printf("- Memory used by orders: %lu bytes\n", orders_memory);
    
    // Show process memory info
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp) {
        char line[128];
        while (fgets(line, 128, fp)) {
            if (strncmp(line, "VmSize:", 7) == 0 ||
                strncmp(line, "VmRSS:", 6) == 0) {
                printf("%s", line);
            }
        }
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s [create_order|process_orders]\n", argv[0]);
        exit(1);
    }
    
    init_shared_memory();
    
    if(strcmp(argv[1], "create_order") == 0) {
        if (argc < 4 || (argc - 2) % 2 != 0) {
            printf("Invalid number of arguments for create_order\n");
            exit(1);
        }
        
        int item_count = (argc - 2) / 2;
        int item_ids[MAX_ITEMS];
        int quantities[MAX_ITEMS];
        
        for(int i = 0; i < item_count; i++) {
            item_ids[i] = atoi(argv[2 + i*2]);
            quantities[i] = atoi(argv[3 + i*2]);
        }
        
        int order_id = create_order(item_ids, quantities, item_count);
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
    else if(strcmp(argv[1], "show_status") == 0) {
        show_system_status();
    }
    else if(strcmp(argv[1], "show_memory") == 0) {
        show_memory_usage();
    }   
    return 0;
}
