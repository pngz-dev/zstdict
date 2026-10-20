#define ZSTDICT_IMPLEMENTATION
#include "zstdict.h"
#include "dict_blob.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    if (zstdict_start(dict_32k_zst, dict_32k_zst_len) != ZSTDICT_OK) return 1;
    
    // Read all of stdin
    size_t cap = 1024 * 1024; // 1MB buffer
    unsigned char *in = malloc(cap);
    size_t in_len = fread(in, 1, cap, stdin);
    
    // Decompress
    unsigned char *out = malloc(cap * 10);
    size_t out_len = cap * 10;
    
    int res = zstdict_unpack(in, in_len, out, &out_len);
    if (res == ZSTDICT_OK) {
        fwrite(out, 1, out_len, stdout);
    } else {
        fprintf(stderr, "Error: zstdict_unpack failed with code %d (in_len: %zu)\n", res, in_len);
    }
    return 0;
}
