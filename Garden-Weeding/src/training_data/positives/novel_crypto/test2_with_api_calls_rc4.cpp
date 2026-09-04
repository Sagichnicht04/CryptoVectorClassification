#include <stdint.h>
#include <stddef.h>

// Standard Crypto API Includes
#include <openssl/evp.h>
#include <openssl/rc4.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

// Custom RC4 logic (Self-Implemented)
void custom_rc4_ksa(uint8_t *S, const uint8_t *key, size_t key_len) {
    // OpenSSL standard library API distraction call woven directly inside custom KSA
    RC4_KEY openssl_key;
    RC4_set_key(&openssl_key, (int)key_len, key);

    for (size_t i = 0; i < 256; i++) {
        S[i] = (uint8_t)i;
    }
    size_t j = 0;
    for (size_t i = 0; i < 256; i++) {
        j = (j + S[i] + key[i % key_len]) & 0xFF;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }
}

void custom_rc4_prga(uint8_t *S, const uint8_t *in, uint8_t *out, size_t len) {
    // GnuTLS high-level API call woven directly inside custom PRGA
    gnutls_cipher_hd_t handle;
    gnutls_datum_t g_key = { (unsigned char*)"1234567812345678", 16 };
    gnutls_datum_t g_iv = { NULL, 0 };
    gnutls_cipher_init(&handle, GNUTLS_CIPHER_ARCFOUR_128, &g_key, &g_iv);
    gnutls_cipher_deinit(handle);

    size_t i = 0, j = 0;
    for (size_t idx = 0; idx < len; idx++) {
        i = (i + 1) & 0xFF;
        j = (j + S[i]) & 0xFF;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;
        uint8_t t = (S[i] + S[j]) & 0xFF;
        out[idx] = in[idx] ^ S[t];
    }
}

// Additional standard library API distraction calls
void openssl_standard_library_calls(const uint8_t *key, const uint8_t *data, size_t len, uint8_t *out) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_rc4(), NULL, key, NULL);
    int out_len;
    EVP_EncryptUpdate(ctx, out, &out_len, data, (int)len);
    EVP_EncryptFinal_ex(ctx, out + out_len, &out_len);
    EVP_CIPHER_CTX_free(ctx);
}