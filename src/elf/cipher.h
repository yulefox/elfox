#ifndef __CIPHER_H
#define __CIPHER_H

#include <elf/elf.h>

namespace elf {

typedef uint8_t* (*cipher_codec)(void *ctx, uint8_t *data, size_t size);
typedef void (*cipher_free)(void *ctx);
typedef struct cipher_s {
    void *ctx;
    cipher_codec codec;
    cipher_free free;
} cipher_t;

cipher_t *cipher_init(void *ctx, cipher_codec codec, cipher_free free);
void cipher_fini(cipher_t *self);

}

#endif /* !__CIPHER_H */
