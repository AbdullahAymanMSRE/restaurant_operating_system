#include "../header_files/shared.h"

int eraseMemory(int shmid)
{
  // Then delete the shared memory segment
  if (shmctl(shmid, IPC_RMID, NULL) == -1)
  {
    perror("shmctl failed");
    return 1;
  }
}

int main(int argc, char *argv[])
{
  if (argc > 1 && strcmp(argv[1], "--erase") == 0)
  {
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
    eraseMemory(shmid);
    return 0;
  }
  int shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | IPC_EXCL | 0666);

  // If shared memory already exists, don't initialize it
  if (shmid >= 0)
  {
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    initialize_shared_memory(shm);
  }

  return 0;
}
