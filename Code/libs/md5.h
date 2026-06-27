#ifndef MD5_H
#define MD5_H

//MD5 şifreleme algoritması
#include <stdint.h>
#include <stddef.h>

typedef struct{
    uint32_t state[4];
    uint32_t count[2];
    uint8_t  buf[64];
} MD5_CTX;

void MD5Init(MD5_CTX *ctx);
void MD5Update(MD5_CTX *ctx, const uint8_t *data, size_t len);
void MD5Final(uint8_t digest[16], MD5_CTX *ctx);
//Bir yazının MD5'ini alıp string olarak verir (32 karakter + \0)
void md5_string(const char *input, size_t len, char output[33]);
#endif /* MD5_H */