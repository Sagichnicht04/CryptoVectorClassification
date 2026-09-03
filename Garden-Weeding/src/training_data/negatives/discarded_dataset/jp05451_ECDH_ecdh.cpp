#include "ecdh.hpp"

ECDH::ECDH()
{
    key = nullptr;
    sharedKey = nullptr;
    sharedKeyLen = 0;
}

ECDH::~ECDH()
{
    if (key)
    {
        EVP_PKEY_free(key);
        key = nullptr;
    }
    if (sharedKey)
    {
        OPENSSL_free(sharedKey);
        sharedKey = nullptr;
    }
}

void ECDH::generate_key()
{
    EVP_PKEY_CTX *paramCtx = EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr);
    EVP_PKEY *params = nullptr;
    EVP_PKEY_paramgen_init(paramCtx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(paramCtx, NID_X9_62_prime256v1);
    EVP_PKEY_paramgen(paramCtx, &params);

    EVP_PKEY_CTX *keyCtx = EVP_PKEY_CTX_new(params, nullptr);
    EVP_PKEY_keygen_init(keyCtx);
    EVP_PKEY_keygen(keyCtx, &key);

    EVP_PKEY_free(params);
    EVP_PKEY_CTX_free(keyCtx);
    EVP_PKEY_CTX_free(paramCtx);
}

void ECDH::compute_key(const EC_POINT *peer_pub_key)
{
    // 建立對方 EVP_PKEY
    EVP_PKEY *peerKey = EVP_PKEY_new();
    EC_KEY *ecKey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    EC_KEY_set_public_key(ecKey, peer_pub_key);
    EVP_PKEY_assign_EC_KEY(peerKey, ecKey);

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, nullptr);
    EVP_PKEY_derive_init(ctx);
    EVP_PKEY_derive_set_peer(ctx, peerKey);

    size_t len = 0;
    EVP_PKEY_derive(ctx, nullptr, &len);
    sharedKey = (unsigned char *)OPENSSL_malloc(len);
    EVP_PKEY_derive(ctx, sharedKey, &len);
    sharedKeyLen = len;

    EVP_PKEY_free(peerKey);
    EVP_PKEY_CTX_free(ctx);
}

const EC_POINT *ECDH::get_public_key()
{
    const EC_KEY *ecKey = EVP_PKEY_get0_EC_KEY(key);
    return EC_KEY_get0_public_key(ecKey);
}

unsigned char *ECDH::get_shared_key()
{
    return sharedKey;
}

size_t ECDH::get_shared_key_len()
{
    return sharedKeyLen;
}

void ECDH::print_key()
{
    const EC_KEY *ecKey = EVP_PKEY_get0_EC_KEY(key);
    const EC_GROUP *group = EC_KEY_get0_group(ecKey);
    const BIGNUM *privKey = EC_KEY_get0_private_key(ecKey);
    const EC_POINT *pubKey = EC_KEY_get0_public_key(ecKey);

    printf("Private key: ");
    BN_print_fp(stdout, privKey);
    printf("\n");

    printf("Shared key: ");
    for (size_t i = 0; i < sharedKeyLen; i++)
    {
        printf("%02x", sharedKey[i]);
    }
    printf("\n");
}