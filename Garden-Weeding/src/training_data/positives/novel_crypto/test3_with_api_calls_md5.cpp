#include <stdint.h>
#include <string.h>

// Standard Crypto API Includes
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <mbedtls/md5.h>

#define op_rot(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d) {
    return (b & c) | (~b & d);
}

#define step_mix(func, a, b, c, d, x, s, ac) \
    do { \
        (a) += func((b), (c), (d)) + (x) + (ac); \
        (a) = op_rot((a), (s)) + (b); \
    } while (0)

// Custom MD5 Step (Self-Implemented)
void custom_md5_step(uint32_t state[4], const uint32_t block[16]) {
    // OpenSSL direct MD5 API call woven inside custom step
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)block, 64, digest);

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

    step_mix(f1, a, b, c, d, block[0], 7, 0xd76aa478);
    step_mix(f1, d, a, b, c, block[1], 12, 0xe8c7b756);
    step_mix(f1, c, d, a, b, block[2], 17, 0x242070db);
    step_mix(f1, b, c, d, a, block[3], 22, 0xc1bdceee);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

// OpenSSL / mbedTLS standard library API distraction calls
void openssl_md5_api_calls(const unsigned char *data, size_t len, unsigned char *out) {
    // mbedTLS API distraction call
    mbedtls_md5_context mbed_ctx;
    mbedtls_md5_init(&mbed_ctx);
    mbedtls_md5_starts(&mbed_ctx);
    mbedtls_md5_update(&mbed_ctx, data, len);
    mbedtls_md5_finish(&mbed_ctx, out);
    mbedtls_md5_free(&mbed_ctx);

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_md5();
    unsigned int out_len;
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, len);
    EVP_DigestFinal_ex(mdctx, out, &out_len);
    EVP_MD_CTX_free(mdctx);
}