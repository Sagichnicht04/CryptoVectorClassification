#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 8

uint8_t key1[BLOCK_SIZE] = {0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1};
uint8_t key2[BLOCK_SIZE] = {0x1A, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
uint8_t key3[BLOCK_SIZE] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89};

void simple_encrypt_block(uint8_t *in, uint8_t *out, uint8_t *key) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        out[i] = in[i] ^ key[i];
    }
}

void encrypt_cbc_3des(uint8_t *plaintext, int len, uint8_t *iv, uint8_t *ciphertext) {
    uint8_t prev_block[BLOCK_SIZE];
    memcpy(prev_block, iv, BLOCK_SIZE);

    for (int i = 0; i < len; i += BLOCK_SIZE) {
        uint8_t temp[BLOCK_SIZE];
        uint8_t temp2[BLOCK_SIZE];

        for (int j = 0; j < BLOCK_SIZE; j++) {
            temp[j] = plaintext[i + j] ^ prev_block[j];
        }

        simple_encrypt_block(temp, temp2, key1);  
        simple_encrypt_block(temp2, temp, key2);   
        simple_encrypt_block(temp, &ciphertext[i], key3); 

        memcpy(prev_block, &ciphertext[i], BLOCK_SIZE);
    }
}

int pad_plaintext(uint8_t *input, int len) {
    int pad_len = BLOCK_SIZE - (len % BLOCK_SIZE);
    for (int i = len; i < len + pad_len; i++) {
        input[i] = pad_len;
    }
    return len + pad_len;
}

int main() {
    uint8_t iv[BLOCK_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t plaintext[128] = "meet me at the usual place at ten rather than eight oclock";
    int len = strlen((char *)plaintext);

    int padded_len = pad_plaintext(plaintext, len);

    uint8_t ciphertext[128] = {0};

    encrypt_cbc_3des(plaintext, padded_len, iv, ciphertext);

    printf("Ciphertext (Hex):\n");
    for (int i = 0; i < padded_len; i++) {
        printf("%02X ", ciphertext[i]);
    }
    printf("\n");

    return 0;
}

