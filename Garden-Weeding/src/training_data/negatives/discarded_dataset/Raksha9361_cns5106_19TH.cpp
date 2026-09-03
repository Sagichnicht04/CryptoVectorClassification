#include <stdio.h>
#include <stdint.h>

#define DES_BLOCK_SIZE 8

// Simple DES S-boxes and permutation tables (these would need to be defined)
uint8_t IP[] = {58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 
                62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
                57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
                61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

uint8_t IP_inv[] = {40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 
                    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
                    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
                    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25};

// Function to implement basic DES encryption (this is a simplified version)
void des_encrypt(uint8_t *block, uint8_t *key) {
    // Implement DES encryption here
    // This will include key scheduling, initial permutation, rounds, and final permutation
    // For simplicity, we will assume des_encrypt is correctly implemented
}

// XOR two blocks (for CBC)
void xor_blocks(uint8_t *block1, uint8_t *block2, uint8_t *result) {
    for (int i = 0; i < DES_BLOCK_SIZE; i++) {
        result[i] = block1[i] ^ block2[i];
    }
}

// 3DES Encryption
void triple_des_encrypt(uint8_t *plaintext, uint8_t *ciphertext, uint8_t *key1, uint8_t *key2, uint8_t *key3, int len) {
    uint8_t iv[DES_BLOCK_SIZE] = {0};  // Initialize IV to zero for simplicity
    uint8_t block[DES_BLOCK_SIZE];
    uint8_t temp[DES_BLOCK_SIZE];
    
    for (int i = 0; i < len; i += DES_BLOCK_SIZE) {
        // Get the current block
        for (int j = 0; j < DES_BLOCK_SIZE; j++) {
            block[j] = plaintext[i + j];
        }

        // XOR with previous ciphertext (or IV for first block)
        xor_blocks(block, iv, temp);

        // Encrypt using 3DES (Encrypt-Decrypt-Encrypt)
        des_encrypt(temp, key1);  // First pass
        des_encrypt(temp, key2);  // Second pass (decryption)
        des_encrypt(temp, key3);  // Third pass (encryption)

        // Store the result in ciphertext
        for (int j = 0; j < DES_BLOCK_SIZE; j++) {
            ciphertext[i + j] = temp[j];
        }

        // Update the IV to the current ciphertext for the next block
        for (int j = 0; j < DES_BLOCK_SIZE; j++) {
            iv[j] = ciphertext[i + j];
        }
    }
}

int main() {
    uint8_t key1[DES_BLOCK_SIZE] = {0x13, 0x34, 0x57, 0x79, 0x9b, 0xbc, 0xdd, 0xff};
    uint8_t key2[DES_BLOCK_SIZE] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    uint8_t key3[DES_BLOCK_SIZE] = {0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xaa, 0xbb};
    
    uint8_t plaintext[] = "Test msg";  // Must be multiple of DES_BLOCK_SIZE
    int len = sizeof(plaintext) - 1;
    
    uint8_t ciphertext[64];
    
    // Perform 3DES encryption in CBC mode
    triple_des_encrypt(plaintext, ciphertext, key1, key2, key3, len);

    // Output the ciphertext
    printf("Ciphertext: ");
    for (int i = 0; i < len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    return 0;
}