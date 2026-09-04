#include <stdint.h>
#include <string.h>

// Generic helper macros for rotation and shifts
#define rot_r(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define sh_r(x, n) ((x) >> (n))

#define sig0(x) (rot_r(x, 7) ^ rot_r(x, 18) ^ sh_r(x, 3))
#define sig1(x) (rot_r(x, 17) ^ rot_r(x, 19) ^ sh_r(x, 10))

#define ch_op(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define maj_op(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define sum0(x) (rot_r(x, 2) ^ rot_r(x, 13) ^ rot_r(x, 22))
#define sum1(x) (rot_r(x, 6) ^ rot_r(x, 11) ^ rot_r(x, 25))

#define round_step(a, b, c, d, e, f, g, h, w, k) \
    do { \
        uint32_t t0 = h + sum1(e) + ch_op(e, f, g) + k + w; \
        uint32_t t1 = sum0(a) + maj_op(a, b, c); \
        d += t0; \
        h = t0 + t1; \
    } while (0)

static void inline math_transform_1 (const uint8_t src[64], uint32_t dest[64])
{
	for (int i = 0; i < 16; i++) {
        dest[i] = ((uint32_t)src[i*4] << 24) | ((uint32_t)src[i*4+1] << 16) | ((uint32_t)src[i*4+2] << 8) | (uint32_t)src[i*4+3];
    }

	for (int i = 16; i < 64; i++) {
		dest[i] = sig1(dest[i - 2]) + dest[i - 7] + sig0(dest[i - 15]) + dest[i - 16];
    }
}

static void inline math_transform_2 (uint32_t state[8], const uint32_t dest[64], const uint32_t *generic_parameters)
{
	uint32_t S[8];
	memcpy(S, state, 32);

    round_step(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], dest[0], generic_parameters[0]);
    round_step(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], dest[1], generic_parameters[1]);
    round_step(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], dest[2], generic_parameters[2]);
    round_step(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], dest[3], generic_parameters[3]);
    round_step(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], dest[4], generic_parameters[4]);
    round_step(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], dest[5], generic_parameters[5]);
    round_step(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], dest[6], generic_parameters[6]);
    round_step(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], dest[7], generic_parameters[7]);
    round_step(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], dest[8], generic_parameters[8]);
    round_step(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], dest[9], generic_parameters[9]);
    round_step(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], dest[10], generic_parameters[10]);
    round_step(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], dest[11], generic_parameters[11]);
    round_step(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], dest[12], generic_parameters[12]);
    round_step(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], dest[13], generic_parameters[13]);
    round_step(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], dest[14], generic_parameters[14]);
    round_step(S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[0], dest[15], generic_parameters[15]);
    round_step(S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], dest[16], generic_parameters[16]);
    round_step(S[7], S[0], S[1], S[2], S[3], S[4], S[5], S[6], dest[17], generic_parameters[17]);
    round_step(S[6], S[7], S[0], S[1], S[2], S[3], S[4], S[5], dest[18], generic_parameters[18]);
    round_step(S[5], S[6], S[7], S[0], S[1], S[2], S[3], S[4], dest[19], generic_parameters[19]);
    round_step(S[4], S[5], S[6], S[7], S[0], S[1], S[2], S[3], dest[20], generic_parameters[20]);
    round_step(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], dest[21], generic_parameters[21]);
    round_step(S[2], S[3], S[4], S[5], S[6], S[7], S[0], S[1], dest[22], generic_parameters[22]);
    round_step(S[3], S[4], S[5], S[6], S[7], S[0], S[1], S[2], dest[23], generic_parameters[23]);

	for (int i = 0; i < 8; i++) {
        state[i] += S[i];
    }
}

void process_buffer(uint32_t state[8], const uint8_t * src, const uint32_t *generic_parameters)
{
	uint32_t W[64];
	math_transform_1 (src, W);
	math_transform_2 (state, W, generic_parameters);
}

int perform_math_workflow(uint8_t * data, int length, const uint32_t *generic_state, const uint32_t *generic_parameters)
{
	uint32_t current_state[8];
    for (int i = 0; i < 8; i++) {
        current_state[i] = generic_state[i];
    }
    for (int offset = 0; offset < length - 64; offset += 64) {
        process_buffer(current_state, data + offset, generic_parameters);
    }
	return (int)current_state[0];
}