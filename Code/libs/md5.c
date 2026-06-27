#include "md5.h"
#include <string.h>
#include <stdio.h>

//Kaydırma sabitleri
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

//Temel MD5 işlevleri
#define F(x,y,z)(((x)&(y))|((~x)&(z)))
#define G(x,y,z)(((x)&(z))|((y)&(~z)))
#define H(x,y,z)((x)^(y)^(z))
#define I(x,y,z)((y)^((x)|(~z)))
#define ROL(x,n)(((x)<<(n))|((x)>>(32-(n))))
#define FF(a,b,c,d,x,s,t){(a)+=F(b,c,d)+(x)+(t);(a)=ROL(a,s);(a)+=(b);}
#define GG(a,b,c,d,x,s,t){(a)+=G(b,c,d)+(x)+(t);(a)=ROL(a,s);(a)+=(b);}
#define HH(a,b,c,d,x,s,t){(a)+=H(b,c,d)+(x)+(t);(a)=ROL(a,s);(a)+=(b);}
#define II(a,b,c,d,x,s,t){(a)+=I(b,c,d)+(x)+(t);(a)=ROL(a,s);(a)+=(b);}

//Dolgu baytları: ilk byte 0x80, geri kalanı sıfır
static const uint8_t PAD[64] = { 0x80 };

//İleriye bildirim
static void md5_transform(uint32_t state[4], const uint8_t block[64]);
static void encode(uint8_t *out, const uint32_t *in, unsigned int len);
static void decode(uint32_t *out, const uint8_t *in, unsigned int len);

void MD5Init(MD5_CTX *ctx){
    ctx->count[0] = ctx->count[1] = 0;
    //RFC 1321'de tanımlı başlangıç değerleri
    ctx->state[0] = 0x67452301u;
    ctx->state[1] = 0xefcdab89u;
    ctx->state[2] = 0x98badcfeu;
    ctx->state[3] = 0x10325476u;
}

void MD5Update(MD5_CTX *ctx, const uint8_t *in, size_t len){
    unsigned int idx = (unsigned int)((ctx->count[0] >> 3) & 0x3f);
    //bit sayacını güncelle
    if ((ctx->count[0] += (uint32_t)(len << 3)) < (uint32_t)(len << 3))
        ctx->count[1]++;
    ctx->count[1] += (uint32_t)(len >> 29);

    unsigned int part = 64 - idx;
    size_t i;

    if(len >= part){
        memcpy(&ctx->buf[idx], in, part);
        md5_transform(ctx->state, ctx->buf);
        for (i = part; i + 63 < len; i += 64)
            md5_transform(ctx->state, &in[i]);
        idx = 0;} 
    else{
        i = 0;}
    memcpy(&ctx->buf[idx], &in[i], len - i);
}

void MD5Final(uint8_t digest[16], MD5_CTX *ctx){
    uint8_t bits[8];
    encode(bits, ctx->count, 8);

    //56 mod 64 konumuna kadar doldur
    unsigned int idx  = (unsigned int)((ctx->count[0] >> 3) & 0x3f);
    unsigned int plen = (idx < 56) ? (56 - idx) : (120 - idx);
    MD5Update(ctx, PAD, plen);
    MD5Update(ctx, bits, 8);

    encode(digest, ctx->state, 16);
    memset(ctx, 0, sizeof(*ctx));
}

void md5_string(const char *input, size_t len, char output[33]){
    MD5_CTX ctx;
    uint8_t  digest[16];
    MD5Init(&ctx);
    MD5Update(&ctx, (const uint8_t *)input, len);
    MD5Final(digest, &ctx);
    for (int i = 0; i < 16; i++)
        snprintf(output + i * 2, 3, "%02x", (unsigned int)digest[i]);
    output[32] = '\0';
}

//İç fonksiyonlar
static void md5_transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    decode(x, block, 64);

    //Round 1
    FF(a,b,c,d,x[ 0],S11,0xd76aa478u); FF(d,a,b,c,x[ 1],S12,0xe8c7b756u);
    FF(c,d,a,b,x[ 2],S13,0x242070dbu); FF(b,c,d,a,x[ 3],S14,0xc1bdceeeu);
    FF(a,b,c,d,x[ 4],S11,0xf57c0fafu); FF(d,a,b,c,x[ 5],S12,0x4787c62au);
    FF(c,d,a,b,x[ 6],S13,0xa8304613u); FF(b,c,d,a,x[ 7],S14,0xfd469501u);
    FF(a,b,c,d,x[ 8],S11,0x698098d8u); FF(d,a,b,c,x[ 9],S12,0x8b44f7afu);
    FF(c,d,a,b,x[10],S13,0xffff5bb1u); FF(b,c,d,a,x[11],S14,0x895cd7beu);
    FF(a,b,c,d,x[12],S11,0x6b901122u); FF(d,a,b,c,x[13],S12,0xfd987193u);
    FF(c,d,a,b,x[14],S13,0xa679438eu); FF(b,c,d,a,x[15],S14,0x49b40821u);

    //Round 2
    GG(a,b,c,d,x[ 1],S21,0xf61e2562u); GG(d,a,b,c,x[ 6],S22,0xc040b340u);
    GG(c,d,a,b,x[11],S23,0x265e5a51u); GG(b,c,d,a,x[ 0],S24,0xe9b6c7aau);
    GG(a,b,c,d,x[ 5],S21,0xd62f105du); GG(d,a,b,c,x[10],S22,0x02441453u);
    GG(c,d,a,b,x[15],S23,0xd8a1e681u); GG(b,c,d,a,x[ 4],S24,0xe7d3fbc8u);
    GG(a,b,c,d,x[ 9],S21,0x21e1cde6u); GG(d,a,b,c,x[14],S22,0xc33707d6u);
    GG(c,d,a,b,x[ 3],S23,0xf4d50d87u); GG(b,c,d,a,x[ 8],S24,0x455a14edu);
    GG(a,b,c,d,x[13],S21,0xa9e3e905u); GG(d,a,b,c,x[ 2],S22,0xfcefa3f8u);
    GG(c,d,a,b,x[ 7],S23,0x676f02d9u); GG(b,c,d,a,x[12],S24,0x8d2a4c8au);

    //Round 3
    HH(a,b,c,d,x[ 5],S31,0xfffa3942u); HH(d,a,b,c,x[ 8],S32,0x8771f681u);
    HH(c,d,a,b,x[11],S33,0x6d9d6122u); HH(b,c,d,a,x[14],S34,0xfde5380cu);
    HH(a,b,c,d,x[ 1],S31,0xa4beea44u); HH(d,a,b,c,x[ 4],S32,0x4bdecfa9u);
    HH(c,d,a,b,x[ 7],S33,0xf6bb4b60u); HH(b,c,d,a,x[10],S34,0xbebfbc70u);
    HH(a,b,c,d,x[13],S31,0x289b7ec6u); HH(d,a,b,c,x[ 0],S32,0xeaa127fau);
    HH(c,d,a,b,x[ 3],S33,0xd4ef3085u); HH(b,c,d,a,x[ 6],S34,0x04881d05u);
    HH(a,b,c,d,x[ 9],S31,0xd9d4d039u); HH(d,a,b,c,x[12],S32,0xe6db99e5u);
    HH(c,d,a,b,x[15],S33,0x1fa27cf8u); HH(b,c,d,a,x[ 2],S34,0xc4ac5665u);

    //Round 4
    II(a,b,c,d,x[ 0],S41,0xf4292244u); II(d,a,b,c,x[ 7],S42,0x432aff97u);
    II(c,d,a,b,x[14],S43,0xab9423a7u); II(b,c,d,a,x[ 5],S44,0xfc93a039u);
    II(a,b,c,d,x[12],S41,0x655b59c3u); II(d,a,b,c,x[ 3],S42,0x8f0ccc92u);
    II(c,d,a,b,x[10],S43,0xffeff47du); II(b,c,d,a,x[ 1],S44,0x85845dd1u);
    II(a,b,c,d,x[ 8],S41,0x6fa87e4fu); II(d,a,b,c,x[15],S42,0xfe2ce6e0u);
    II(c,d,a,b,x[ 6],S43,0xa3014314u); II(b,c,d,a,x[13],S44,0x4e0811a1u);
    II(a,b,c,d,x[ 4],S41,0xf7537e82u); II(d,a,b,c,x[11],S42,0xbd3af235u);
    II(c,d,a,b,x[ 2],S43,0x2ad7d2bbu); II(b,c,d,a,x[ 9],S44,0xeb86d391u);

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    memset(x, 0, sizeof(x));
}

static void encode(uint8_t *out, const uint32_t *in, unsigned int len){
    for(unsigned int i = 0, j = 0; j < len; i++, j += 4){
        out[j]   = (uint8_t)( in[i]        & 0xff);
        out[j+1] = (uint8_t)((in[i] >>  8) & 0xff);
        out[j+2] = (uint8_t)((in[i] >> 16) & 0xff);
        out[j+3] = (uint8_t)((in[i] >> 24) & 0xff);}
}

static void decode(uint32_t *out, const uint8_t *in, unsigned int len){
    for(unsigned int i = 0, j = 0; j < len; i++, j += 4)
        out[i] = (uint32_t)in[j]          |
                 ((uint32_t)in[j+1] <<  8) |
                 ((uint32_t)in[j+2] << 16) |
                 ((uint32_t)in[j+3] << 24);
}