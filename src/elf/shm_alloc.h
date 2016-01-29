#ifndef __SHM_ALLOC_H
#define __SHM_ALLOC_H

namespace elf {

void *shm_alloc(int key, size_t size);
void shm_free(void *ptr);

}

#endif /* !__SHM_ARRAY_H */
