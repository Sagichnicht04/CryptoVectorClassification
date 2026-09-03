#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>
#include <stdio.h>

// Function to handle errors
void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

// Generate RSA key pair and save to files
void generate_rsa_key() {
    int ret;
    RSA *rsa = NULL;
    BIGNUM *bne = NULL;
    BIO *bp_public = NULL, *bp_private = NULL;

    int bits = 2048;
    unsigned long e = RSA_F4;

    // Generate the big number object
    bne = BN_new();
    ret = BN_set_word(bne, e);
    if (ret != 1) {
        handleErrors();
    }

    // Generate the RSA key pair
    rsa = RSA_new();
    ret = RSA_generate_key_ex(rsa, bits, bne, NULL);
    if (ret != 1) {
        handleErrors();
    }

    // Save the public key
    bp_public = BIO_new_file("public.pem", "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, rsa);
    if (ret != 1) {
        handleErrors();
    }

    // Save the private key
    bp_private = BIO_new_file("private.pem", "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL);
    if (ret != 1) {
        handleErrors();
    }

    // Free allocated memory
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(rsa);
    BN_free(bne);
}

// Encrypt the message using RSA public key
int rsa_encrypt(unsigned char *msg, int msg_len, unsigned char *encrypted) {
    RSA *rsa = NULL;
    FILE *fp = fopen("public.pem", "r");

    if (fp == NULL) {
        handleErrors();
    }

    rsa = PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL);
    if (rsa == NULL) {
        handleErrors();
    }

    int result = RSA_public_encrypt(msg_len, msg, encrypted, rsa, RSA_PKCS1_OAEP_PADDING);
    if (result == -1) {
        handleErrors();
    }

    RSA_free(rsa);
    fclose(fp);

    return result;
}

// Decrypt the message using RSA private key
int rsa_decrypt(unsigned char *enc_msg, int enc_msg_len, unsigned char *decrypted) {
    RSA *rsa = NULL;
    FILE *fp = fopen("private.pem", "r");

    if (fp == NULL) {
        handleErrors();
    }

    rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
    if (rsa == NULL) {
        handleErrors();
    }

    int result = RSA_private_decrypt(enc_msg_len, enc_msg, decrypted, rsa, RSA_PKCS1_OAEP_PADDING);
    if (result == -1) {
        handleErrors();
    }

    RSA_free(rsa);
    fclose(fp);

    return result;
}

int main() {
    // Generate RSA key pair
    generate_rsa_key();

    // Message to be encrypted
    unsigned char *message = (unsigned char *)"Hello User B, this is User A.";
    unsigned char encrypted[256];
    unsigned char decrypted[256];

    // Encrypt the message
    int encrypted_length = rsa_encrypt(message, strlen((char *)message), encrypted);
    if (encrypted_length == -1) {
        handleErrors();
    }

    printf("Encrypted message:\n");
    for (int i = 0; i < encrypted_length; i++) {
        printf("%02x", encrypted[i]);
    }
    printf("\n");

    // Decrypt the message
    int decrypted_length = rsa_decrypt(encrypted, encrypted_length, decrypted);
    if (decrypted_length == -1) {
        handleErrors();
    }

    decrypted[decrypted_length] = '\0'; // Null-terminate the decrypted message
    printf("Decrypted message: %s\n", decrypted);

    return 0;
}
