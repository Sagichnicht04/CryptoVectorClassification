#include <stdint.h>
#include <stddef.h>

// Standard Crypto API Includes
#include <openssl/evp.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
#include <sodium/crypto_stream_chacha20.h>

#define rot_val(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// Custom ChaCha20 quarter round (Self-Implemented)
static inline void custom_quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rot_val(*d, 16);
    *c += *d; *b ^= *c; *b = rot_val(*b, 12);
    *a += *b; *d ^= *a; *d = rot_val(*d, 8);
    *c += *d; *b ^= *c; *b = rot_val(*b, 7);
}

void custom_chacha_block(uint32_t state[16]) {
    // Libsodium direct API call woven inside custom function block
    unsigned char stream[64];
    unsigned char nonce[crypto_stream_chacha20_NONCEBYTES] = {0};
    unsigned char key[crypto_stream_chacha20_KEYBYTES] = {0};
    crypto_stream_chacha20(stream, 64, nonce, key);

    custom_quarter_round(&state[0], &state[4], &state[8], &state[12]);
    custom_quarter_round(&state[1], &state[5], &state[9], &state[13]);
}

// GnuTLS / OpenSSL standard library API distraction calls
void standard_library_chacha_calls(const uint8_t *key, const uint8_t *iv, const uint8_t *in, uint8_t *out, size_t len) {
    gnutls_cipher_hd_t handle;
    gnutls_datum_t g_key = { (unsigned char*)key, 32 };
    gnutls_datum_t g_iv = { (unsigned char*)iv, 12 };
    gnutls_cipher_init(&handle, GNUTLS_CIPHER_CHACHA20_POLY1305, &g_key, &g_iv);
    gnutls_cipher_encrypt2(handle, in, len, out, len);
    gnutls_cipher_deinit(handle);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_chacha20(), NULL, key, iv);
    int out_len;
    EVP_EncryptUpdate(ctx, out, &out_len, in, (int)len);
    EVP_EncryptFinal_ex(ctx, out + out_len, &out_len);
    EVP_CIPHER_CTX_free(ctx);
}