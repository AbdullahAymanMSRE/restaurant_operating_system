#include "shared.h"

SharedMemory* attach_shared_memory() {
    // Create or get shared memory segment
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment
    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    return shm;
}

void detach_shared_memory(SharedMemory* shm) {
    if (shmdt(shm) == -1) {
        perror("shmdt failed");
        exit(1);
    }
}

void initialize_shared_memory(SharedMemory* shm) {
    // Initialize mutex with shared memory attribute
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->orders_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    // Initialize orders
    shm->num_orders = 0;

    // Initialize sample menu items
    MenuItem sample_menu[] = {
        {1, "Burger", 9.99},
        {2, "Pizza", 12.99},
        {3, "Salad", 7.99},
        {4, "Fries", 3.99},
        {5, "Soda", 1.99}
    };

    shm->num_menu_items = sizeof(sample_menu) / sizeof(MenuItem);
    memcpy(shm->menu, sample_menu, sizeof(sample_menu));
}

void initialize_order_mutex(pthread_mutex_t* mutex) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}
