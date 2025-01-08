#include<shared.h>

int main(){
    SharedMemory* shm = attach_shared_memory();
    initialize_shared_memory(shm);
}
