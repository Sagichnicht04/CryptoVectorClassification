#include <stdint.h>
#include <string.h>

#define op_rot(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d) {
    return (b & c) | (~b & d);
}

static inline uint32_t g1(uint32_t b, uint32_t c, uint32_t d) {
    return (b & d) | (c & ~d);
}

static inline uint32_t h1(uint32_t b, uint32_t c, uint32_t d) {
    return b ^ c ^ d;
}

static inline uint32_t i1(uint32_t b, uint32_t c, uint32_t d) {
    return c ^ (b | ~d);
}

#define step_mix(func, a, b, c, d, x, s, ac) \
    do { \
        (a) += func((b), (c), (d)) + (x) + (ac); \
        (a) = op_rot((a), (s)) + (b); \
    } while (0)

void compute_digest_step(uint32_t state[4], const uint32_t block[16], const uint32_t *generic_params, const int *shift_params) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];

    // Round 1
    step_mix(f1, a, b, c, d, block[0], shift_params[0], generic_params[0]);
    step_mix(f1, d, a, b, c, block[1], shift_params[1], generic_params[1]);
    step_mix(f1, c, d, a, b, block[2], shift_params[2], generic_params[2]);
    step_mix(f1, b, c, d, a, block[3], shift_params[3], generic_params[3]);
    step_mix(f1, a, b, c, d, block[4], shift_params[0], generic_params[4]);
    step_mix(f1, d, a, b, c, block[5], shift_params[1], generic_params[5]);
    step_mix(f1, c, d, a, b, block[6], shift_params[2], generic_params[6]);
    step_mix(f1, b, c, d, a, block[7], shift_params[3], generic_params[7]);
    step_mix(f1, a, b, c, d, block[8], shift_params[0], generic_params[8]);
    step_mix(f1, d, a, b, c, block[9], shift_params[1], generic_params[9]);
    step_mix(f1, c, d, a, b, block[10], shift_params[2], generic_params[10]);
    step_mix(f1, b, c, d, a, block[11], shift_params[3], generic_params[11]);
    step_mix(f1, a, b, c, d, block[12], shift_params[0], generic_params[12]);
    step_mix(f1, d, a, b, c, block[13], shift_params[1], generic_params[13]);
    step_mix(f1, c, d, a, b, block[14], shift_params[2], generic_params[14]);
    step_mix(f1, b, c, d, a, block[15], shift_params[3], generic_params[15]);

    // Round 2
    step_mix(g1, a, b, c, d, block[1], shift_params[4], generic_params[16]);
    step_mix(g1, d, a, b, c, block[6], shift_params[5], generic_params[17]);
    step_mix(g1, c, d, a, b, block[11], shift_params[6], generic_params[18]);
    step_mix(g1, b, c, d, a, block[0], shift_params[7], generic_params[19]);
    step_mix(g1, a, b, c, d, block[5], shift_params[4], generic_params[20]);
    step_mix(g1, d, a, b, c, block[10], shift_params[5], generic_params[21]);
    step_mix(g1, c, d, a, b, block[15], shift_params[6], generic_params[22]);
    step_mix(g1, b, c, d, a, block[4], shift_params[7], generic_params[23]);
    step_mix(g1, a, b, c, d, block[9], shift_params[4], generic_params[24]);
    step_mix(g1, d, a, b, c, block[14], shift_params[5], generic_params[25]);
    step_mix(g1, c, d, a, b, block[3], shift_params[6], generic_params[26]);
    step_mix(g1, b, c, d, a, block[8], shift_params[7], generic_params[27]);
    step_mix(g1, a, b, c, d, block[13], shift_params[4], generic_params[28]);
    step_mix(g1, d, a, b, c, block[2], shift_params[5], generic_params[29]);
    step_mix(g1, c, d, a, b, block[7], shift_params[6], generic_params[30]);
    step_mix(g1, b, c, d, a, block[12], shift_params[7], generic_params[31]);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}