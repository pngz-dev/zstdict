#ifndef ZSTDICT_H
#define ZSTDICT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZSTDICT_OK           0
#define ZSTDICT_ERR_INIT    -1
#define ZSTDICT_ERR_DICT    -2
#define ZSTDICT_ERR_COMP    -3
#define ZSTDICT_ERR_DECOMP  -4
#define ZSTDICT_ERR_BOUND   -5

#define ZSTDICT_LEVEL 19
#define ZSTDICT_ENCODING "zstd-dict"

int    zstdict_start(const void *dict, size_t dict_len);
void   zstdict_stop(void);
size_t zstdict_max_out(size_t in_len);
int    zstdict_pack(const void *in, size_t in_len, void *out, size_t *out_len);
int    zstdict_unpack(const void *in, size_t in_len, void *out, size_t *out_len);

typedef struct zstdict_packer   zstdict_packer;
typedef struct zstdict_unpacker zstdict_unpacker;

zstdict_packer   *zstdict_packer_new(void);
void              zstdict_packer_drop(zstdict_packer *p);
int               zstdict_pack_with(zstdict_packer *p, const void *in, size_t in_len,
                                    void *out, size_t *out_len);

zstdict_unpacker *zstdict_unpacker_new(void);
void              zstdict_unpacker_drop(zstdict_unpacker *u);
int               zstdict_unpack_with(zstdict_unpacker *u, const void *in, size_t in_len,
                                      void *out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif

#ifdef ZSTDICT_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <zstd.h>

struct zstdict_packer   { ZSTD_CCtx *ctx; };
struct zstdict_unpacker { ZSTD_DCtx *ctx; };

static ZSTD_CDict *zstdict_cdict = NULL;
static ZSTD_DDict *zstdict_ddict = NULL;

int zstdict_start(const void *dict, size_t dict_len)
{
    if (!dict || !dict_len) return ZSTDICT_ERR_INIT;
    zstdict_stop();
    zstdict_cdict = ZSTD_createCDict(dict, dict_len, ZSTDICT_LEVEL);
    if (!zstdict_cdict) return ZSTDICT_ERR_DICT;
    zstdict_ddict = ZSTD_createDDict(dict, dict_len);
    if (!zstdict_ddict) {
        ZSTD_freeCDict(zstdict_cdict);
        zstdict_cdict = NULL;
        return ZSTDICT_ERR_DICT;
    }
    return ZSTDICT_OK;
}

void zstdict_stop(void)
{
    if (zstdict_cdict) { ZSTD_freeCDict(zstdict_cdict); zstdict_cdict = NULL; }
    if (zstdict_ddict) { ZSTD_freeDDict(zstdict_ddict); zstdict_ddict = NULL; }
}

size_t zstdict_max_out(size_t in_len)
{
    return ZSTD_compressBound(in_len);
}

int zstdict_pack(const void *in, size_t in_len, void *out, size_t *out_len)
{
    if (!zstdict_cdict || !in || !out || !out_len) return ZSTDICT_ERR_COMP;
    ZSTD_CCtx *ctx = ZSTD_createCCtx();
    if (!ctx) return ZSTDICT_ERR_COMP;
    size_t n = ZSTD_compress_usingCDict(ctx, out, *out_len, in, in_len, zstdict_cdict);
    ZSTD_freeCCtx(ctx);
    if (ZSTD_isError(n)) return ZSTDICT_ERR_COMP;
    *out_len = n;
    return ZSTDICT_OK;
}

int zstdict_unpack(const void *in, size_t in_len, void *out, size_t *out_len)
{
    if (!zstdict_ddict || !in || !out || !out_len) return ZSTDICT_ERR_DECOMP;
    ZSTD_DCtx *ctx = ZSTD_createDCtx();
    if (!ctx) return ZSTDICT_ERR_DECOMP;
    size_t n = ZSTD_decompress_usingDDict(ctx, out, *out_len, in, in_len, zstdict_ddict);
    ZSTD_freeDCtx(ctx);
    if (ZSTD_isError(n)) return ZSTDICT_ERR_DECOMP;
    *out_len = n;
    return ZSTDICT_OK;
}

zstdict_packer *zstdict_packer_new(void)
{
    zstdict_packer *p = calloc(1, sizeof(*p));
    if (!p) return NULL;
    p->ctx = ZSTD_createCCtx();
    if (!p->ctx) { free(p); return NULL; }
    return p;
}

void zstdict_packer_drop(zstdict_packer *p)
{
    if (!p) return;
    ZSTD_freeCCtx(p->ctx);
    free(p);
}

int zstdict_pack_with(zstdict_packer *p, const void *in, size_t in_len,
                      void *out, size_t *out_len)
{
    if (!p || !zstdict_cdict) return ZSTDICT_ERR_COMP;
    size_t n = ZSTD_compress_usingCDict(p->ctx, out, *out_len, in, in_len, zstdict_cdict);
    if (ZSTD_isError(n)) return ZSTDICT_ERR_COMP;
    *out_len = n;
    return ZSTDICT_OK;
}

zstdict_unpacker *zstdict_unpacker_new(void)
{
    zstdict_unpacker *u = calloc(1, sizeof(*u));
    if (!u) return NULL;
    u->ctx = ZSTD_createDCtx();
    if (!u->ctx) { free(u); return NULL; }
    return u;
}

void zstdict_unpacker_drop(zstdict_unpacker *u)
{
    if (!u) return;
    ZSTD_freeDCtx(u->ctx);
    free(u);
}

int zstdict_unpack_with(zstdict_unpacker *u, const void *in, size_t in_len,
                        void *out, size_t *out_len)
{
    if (!u || !zstdict_ddict) return ZSTDICT_ERR_DECOMP;
    size_t n = ZSTD_decompress_usingDDict(u->ctx, out, *out_len, in, in_len, zstdict_ddict);
    if (ZSTD_isError(n)) return ZSTDICT_ERR_DECOMP;
    *out_len = n;
    return ZSTDICT_OK;
}

#endif
