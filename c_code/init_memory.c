#include "../header_files/shared.h"

int main(){
    /* int shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666); */

    // If shared memory already exists, don't initialize it
    /* if (shmid >= 0) { */
      SharedMemory* shm = attach_shared_memory();
      initialize_shared_memory(shm);
    /* } */

    return 0;
}
