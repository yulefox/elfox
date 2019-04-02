#ifndef __PIKE_H
#define __PIKE_H

namespace elf {

typedef struct pike_s pike_t;

pike_t *pike_ctx_init(uint32_t sd);
void pike_ctx_fini(void *ctx);
uint8_t *pike_codec(void *ctx, uint8_t *data, size_t size);

}

#endif /* !__PIKE_H */

