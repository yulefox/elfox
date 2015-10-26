#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "pike.h"

namespace elf {

#define	GENIUS_NUMBER  0x05027919

typedef  struct ff_addikey_s {
	uint32_t sd;
	int32_t dis1;
	int32_t dis2;
	int32_t index;
	int32_t carry;
	uint32_t buffer[64];
} ff_addikey_t;

struct pike_s {
	uint32_t sd;
	int32_t index;
	ff_addikey_t addikey[3];
	uint8_t buffer[4096];
};

static uint32_t linearity(uint32_t key) {
	return ((((key >> 31) ^ (key >> 6) ^ (key >> 4) ^ (key >> 2) ^ (key >> 1) ^ key) & 0x00000001) << 31) | (key >> 1);
}

pike_t *pike_ctx_init(uint32_t sd)  {
    pike_t *ctx;
    int i;

    ctx = (pike_t*)malloc(sizeof(pike_t));
	ctx->sd = sd ^ GENIUS_NUMBER;

	ctx->addikey[0].sd = ctx->sd;
	ctx->addikey[0].sd = linearity(ctx->addikey[0].sd);
	ctx->addikey[0].dis1 = 55;
	ctx->addikey[0].dis2 = 24;

	ctx->addikey[1].sd = ((ctx->sd & 0xAAAAAAAA) >> 1) | ((ctx->sd & 0x55555555) << 1);
	ctx->addikey[1].sd = linearity(ctx->addikey[1].sd);
	ctx->addikey[1].dis1 = 57;
	ctx->addikey[1].dis2 = 7;

	ctx->addikey[2].sd = ~(((ctx->sd & 0xF0F0F0F0) >> 4) | ((ctx->sd & 0x0F0F0F0F) << 4));
	ctx->addikey[2].sd = linearity(ctx->addikey[2].sd);
	ctx->addikey[2].dis1 = 58;
	ctx->addikey[2].dis2 = 19;

	for (i = 0;i < 3; i++) {
		uint32_t tmp = ctx->addikey[i].sd;
        int j;
		for (j = 0;j < 64; j++) {
            int k;
			for (k = 0;k < 32; k++) {
				tmp = linearity(tmp);
			}
			ctx->addikey[i].buffer[j] = tmp;
		}
		ctx->addikey[i].carry = 0;
		ctx->addikey[i].index = 63;
	}
	ctx->index = 4096;
    return ctx;
}

void pike_ctx_fini(pike_t *ctx) {
    if (ctx != NULL) {
        free(ctx);
    }
}

static void _addikey_next(ff_addikey_t *addikey) {
    int32_t tmp;
    int32_t i1;
    int32_t i2;
    
    tmp = addikey->index + 1;
	addikey->index = tmp & 0x03F;

	i1 = ((addikey->index | 0x40) - addikey->dis1) & 0x03F;
	i2 = ((addikey->index | 0x40) - addikey->dis2) & 0x03F;

	addikey->buffer[addikey->index] = addikey->buffer[i1] + addikey->buffer[i2];
	if ((addikey->buffer[addikey->index] < addikey->buffer[i1]) ||
        (addikey->buffer[addikey->index] < addikey->buffer[i2])) {
		addikey->carry = 1;
	} else {
		addikey->carry = 0;
	}
}

static void _generate(pike_t *ctx) {
    int base;
    int i;
	for (i = 0;i < 1024; i++) {
		int32_t carry = ctx->addikey[0].carry + ctx->addikey[1].carry + ctx->addikey[2].carry;
        uint32_t tmp;
		if (carry == 0 || carry == 3) {
			_addikey_next(&ctx->addikey[0]);
			_addikey_next(&ctx->addikey[1]);
			_addikey_next(&ctx->addikey[2]);
		} else {
            int32_t flag = 0;
            int j;
			if (carry == 2) {
				flag = 1;
			}
			for (j = 0;j < 3; j++) {
				if (ctx->addikey[j].carry == flag) {
					_addikey_next(&ctx->addikey[j]);
				}
			}
		}

		tmp = ctx->addikey[0].buffer[ctx->addikey[0].index] ^
            ctx->addikey[1].buffer[ctx->addikey[1].index] ^
            ctx->addikey[2].buffer[ctx->addikey[2].index];
		base = i << 2;
		ctx->buffer[base] = (uint8_t)tmp;
		ctx->buffer[base+1] = (uint8_t)(tmp >> 8);
		ctx->buffer[base+2] = (uint8_t)(tmp >> 16);
		ctx->buffer[base+3] = (uint8_t)(tmp >> 24);
	}
	ctx->index = 0;
}



uint8_t *pike_codec(pike_t *ctx, uint8_t *data, size_t size) {
    int32_t remnant;
    int32_t off;
    if (size == 0) {
        return data;
    }

    off = 0;
    for (;;) {
        int32_t i;
		remnant = 4096 - ctx->index;
		if (remnant <= 0) {
			_generate(ctx);
			continue;
		}

		if ((size_t)remnant > size) {
			remnant = size;
		}
        size -= remnant;
		for (i = 0; i < remnant; i++) {
			data[off] ^= ctx->buffer[ctx->index + i];
			off++;
		}
		ctx->index += remnant;
        if (size <= 0) break;
	}
    return data;
}

}

