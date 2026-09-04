#include "blowfish.h"
#include "subkey_generator.h"
#include "blowfish_defines.h"
#include "encryptor.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#define PLAIN_FILE_PATH "C:\\Users\\79112\\OneDrive\\Рабочий стол\\blowfish_ofb\\plain_text.txt"
#define ENCRYPTED_FILE_PATH "C:\\Users\\79112\\OneDrive\\Рабочий стол\\blowfish_ofb\\encrypted_text.txt"
#define DECRYPTED_FILE_PATH "C:\\Users\\79112\\OneDrive\\Рабочий стол\\blowfish_ofb\\decrypted_text.txt"
#define SKIP_BMP_HEADER_FLAG 0
#define BMP_HEADER_SIZE_BYTES 54
#define INIT_VECTOR_VALUE 0x6a09e667f3bcc908

void blowfish(struct args _args){
    size_t input_key_size = 64;
    uint32_t input_key[2] = {
            0xfee1dead,
            0xbeefcdcd
    };
    uint64_t init_ofb_vector = INIT_VECTOR_VALUE;
    uint32_t *subkeys = generate_subkeys(input_key, input_key_size);
    printf("given key:");
    for (size_t i = 0; i < input_key_size / (sizeof (uint32_t) * 8); i++){
        printf("%" PRIx32, input_key[i]);
    }
    printf("\ngenerated keys:\n");
    for (size_t i = 0; i < SUBKEY_NUMBER; i++){
        printf("P[%zu]=%" PRIx32 "\n", i, subkeys[i]);
    }
    char *plain_text_path = PLAIN_FILE_PATH;
    FILE *plain_textf = fopen(plain_text_path, "rb");
    char *encrypted_text_path = ENCRYPTED_FILE_PATH;
    FILE *encrypted_textf = fopen(encrypted_text_path, "wb");
    char *decrypted_text_path = DECRYPTED_FILE_PATH;
    FILE *decrypted_textf = fopen(decrypted_text_path, "wb");
    size_t counter = 0;
    uint64_t buff = 0;
    _args.decrypt_mode_flag = 1;
    if (SKIP_BMP_HEADER_FLAG){
        for(size_t c = 0; c < BMP_HEADER_SIZE_BYTES; c++)
            fread(&buff, sizeof(uint8_t), 1, plain_textf);
    }
    printf("***ENCRYPTION***\n");
    for(; fread(&buff, sizeof(buff), 1, plain_textf); counter += sizeof(buff)){
        printf("%"PRIx64"\n", init_ofb_vector);
        uint32_t lvector = init_ofb_vector>>32;
        uint32_t rvector = (uint32_t) init_ofb_vector;
        blowfish_encrypt(&lvector, &rvector, subkeys);
        uint64_t encrypted_vector = (uint64_t)lvector << 32 | rvector;
        uint64_t encrypted_block = buff ^ encrypted_vector;
        fwrite(&encrypted_block, sizeof(buff), 1, encrypted_textf);
        init_ofb_vector = encrypted_vector;
    }
    printf("read & encrypted total: %"PRIu32" bytes\n", counter);
    counter = 0;
    fclose(encrypted_textf);
    printf("***DECRYPTION***\n");
    encrypted_textf = fopen(encrypted_text_path, "rb");
    init_ofb_vector = INIT_VECTOR_VALUE;
    for(; fread(&buff, sizeof(buff), 1, encrypted_textf); counter += sizeof(buff)){
        printf("%"PRIx64"\n", init_ofb_vector);
        uint32_t lvector = init_ofb_vector>>32;
        uint32_t rvector = (uint32_t) init_ofb_vector;
        blowfish_encrypt(&lvector, &rvector, subkeys);
        uint64_t encrypted_vector = (uint64_t)lvector << 32 | rvector;
        uint64_t encrypted_block = buff ^ encrypted_vector;
        fwrite(&encrypted_block, sizeof(buff), 1, decrypted_textf);
        init_ofb_vector = encrypted_vector;
    }
    printf("read & decrypted total: %"PRIu32" bytes\n", counter);
    fclose(plain_textf);
    fclose(encrypted_textf);
    fclose(decrypted_textf);
}
