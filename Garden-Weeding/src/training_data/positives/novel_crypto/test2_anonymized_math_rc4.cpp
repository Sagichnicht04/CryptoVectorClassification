#include <stdint.h>
#include <stddef.h>

void step_a(uint8_t *arr, const uint8_t *k, size_t k_len, size_t list_limit, uint8_t mask) {
    for (size_t i = 0; i < list_limit; i++) {
        arr[i] = (uint8_t)i;
    }
    size_t j = 0;
    for (size_t i = 0; i < list_limit; i++) {
        j = (j + arr[i] + k[i % k_len]) & mask;
        uint8_t temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void step_b(uint8_t *arr, const uint8_t *in, uint8_t *out, size_t len, uint8_t mask) {
    size_t i = 0, j = 0;
    for (size_t idx = 0; idx < len; idx++) {
        i = (i + 1) & mask;
        j = (j + arr[i]) & mask;
        uint8_t temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        uint8_t t = (arr[i] + arr[j]) & mask;
        out[idx] = in[idx] ^ arr[t];
    }
}