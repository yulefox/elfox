#include <math.h>
#include <stdlib.h>
#include <elf/skiplist.h>

#include <sys/ipc.h>
#include <sys/shm.h>

namespace elf {

void *shm_alloc(int key, size_t size)
{
    void *ptr = NULL;
    int id = shmget((key_t)key, size, (SHM_R | SHM_W | IPC_CREAT));
    if (id == -1) {
        return NULL;
    }
    ptr = shmat(id, NULL, 0);
    if (ptr == NULL) {
        return NULL;
    }
    return ptr;
}

void shm_free(void *ptr)
{
    shmdt(ptr);
}

}

