#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ORDERS 100
#define MAX_ITEMS 10
#define MAX_ITEM_NAME 50
#define SHM_KEY 12345
#define ORDER_SEM_KEY 54321

// Order status enumeration
typedef enum {
    STATUS_NEW,
    STATUS_IN_PROGRESS,
    STATUS_READY,
    STATUS_COMPLETED
} OrderStatus;

// Structure for a single menu item
typedef struct {
    int id;
    char name[MAX_ITEM_NAME];
    double price;
} MenuItem;

// Structure for an order item
typedef struct {
    int menu_item_id;
    int quantity;
} OrderItem;

// Structure for a complete order
typedef struct {
    int order_id;
    pid_t process_id;          // Process ID handling this order
    OrderItem items[MAX_ITEMS];
    int num_items;
    OrderStatus status;
    time_t timestamp;
    int table_number;
    pthread_mutex_t order_mutex;  // Mutex specific to this order
} Order;

// Structure for shared memory
typedef struct {
    Order orders[MAX_ORDERS];
    int num_orders;
    pthread_mutex_t orders_mutex;  // Mutex for orders array modification
    MenuItem menu[MAX_ITEMS];
    int num_menu_items;
} SharedMemory;

// Function declarations
SharedMemory* attach_shared_memory();
void detach_shared_memory(SharedMemory* shm);
void initialize_shared_memory(SharedMemory* shm);
void initialize_order_mutex(pthread_mutex_t* mutex);

#endif
