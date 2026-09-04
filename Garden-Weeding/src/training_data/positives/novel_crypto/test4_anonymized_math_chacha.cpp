#include <stdint.h>
#include <stddef.h>

#define rot_val(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static inline void process_quad(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, const int *rot) {
    *a += *b; *d ^= *a; *d = rot_val(*d, rot[0]);
    *c += *d; *b ^= *c; *b = rot_val(*b, rot[1]);
    *a += *b; *d ^= *a; *d = rot_val(*d, rot[2]);
    *c += *d; *b ^= *c; *b = rot_val(*b, rot[3]);
}

void transform_data_block(uint32_t arr[16], int loop_limit, const int *rot) {
    for (int i = 0; i < loop_limit; i++) {
        // Vertical rounds
        process_quad(&arr[0], &arr[4], &arr[8], &arr[12], rot);
        process_quad(&arr[1], &arr[5], &arr[9], &arr[13], rot);
        process_quad(&arr[2], &arr[6], &arr[10], &arr[14], rot);
        process_quad(&arr[3], &arr[7], &arr[11], &arr[15], rot);

        // Diagonal rounds
        process_quad(&arr[0], &arr[5], &arr[10], &arr[15], rot);
        process_quad(&arr[1], &arr[6], &arr[11], &arr[12], rot);
        process_quad(&arr[2], &arr[7], &arr[8], &arr[13], rot);
        process_quad(&arr[3], &arr[4], &arr[9], &arr[14], rot);
    }
}