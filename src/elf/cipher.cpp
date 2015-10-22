#include <stdlib.h>

#include "cipher.h"

namespace elf {

cipher_t *cipher_init(void *ctx, cipher_codec codec, cipher_free free) {
    cipher_t *self = NULL;

    self = (cipher_t*)malloc(sizeof(cipher_t));
    if (self == NULL) {
        return NULL;
    }

    self->ctx = ctx;
    self->codec = codec;
    self->free = free;
    return self;
}

void cipher_fini(cipher_t *self)
{
    if (self != NULL) {
        if (self->free) {
            self->free(self->ctx);
        }
        free(self);
    }
}

}

