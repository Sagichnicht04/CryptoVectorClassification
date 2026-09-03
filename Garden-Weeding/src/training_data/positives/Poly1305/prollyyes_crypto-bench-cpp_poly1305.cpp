// Minimal portable Poly1305 implementation by me due to missing dependency (mac.h) (RFC7539)
// I implemented this from scratch since multiple brew reinstallations did not provide the missing header.
#include "poly1305.hpp"

#include <cstdint>
#include <cstring>

// Reference-style implementation using 128-bit intermediates. Public-domain style.
// Not optimized for speed; focuses on clarity and correctness.

static inline uint64_t load32_le(const unsigned char *p) {
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24);
}

static inline uint64_t load64_le(const unsigned char *p) {
    uint64_t lo = load32_le(p);
    uint64_t hi = load32_le(p + 4);
    return lo | (hi << 32);
}

void poly1305_auth(unsigned char tag[16], const unsigned char *m, size_t mlen, const unsigned char key[32]) {
    // r = key[0..15] (with clamp), s = key[16..31]
    uint64_t r0 = load32_le(key + 0) & 0x3ffffff;
    uint64_t r1 = (load32_le(key + 3) >> 2) & 0x3ffff03;
    // Simpler approach: use 128-bit limbs per reference but here we use generic algorithm

    // I'll implement the algorithm using 130-bit accumulator represented by 3 64-bit limbs.
    // Use the widely used arithmetic method but keep code compact.

    // Decode r as 16-byte little-endian and clamp bits per RFC
    unsigned char rbytes[16];
    memcpy(rbytes, key, 16);
    rbytes[3] &= 15;
    rbytes[7] &= 15;
    rbytes[11] &= 15;
    rbytes[15] &= 15;
    rbytes[4] &= 252;
    rbytes[8] &= 252;
    rbytes[12] &= 252;

    // convert r to 5 26-bit limbs
    uint64_t r[5];
    r[0] = (uint64_t)(rbytes[0] | ((uint64_t)rbytes[1] << 8) | ((uint64_t)rbytes[2] << 16) | (((uint64_t)rbytes[3] & 0x3) << 24)) & 0x3ffffff;
    r[1] = (uint64_t)(((rbytes[3] >> 2) | ((uint64_t)rbytes[4] << 6) | ((uint64_t)rbytes[5] << 14) | ((uint64_t)(rbytes[6] & 0xF) << 22))) & 0x3ffffff;
    r[2] = (uint64_t)(((rbytes[6] >> 4) | ((uint64_t)rbytes[7] << 4) | ((uint64_t)rbytes[8] << 12) | ((uint64_t)(rbytes[9] & 0x3) << 20))) & 0x3ffffff;
    r[3] = (uint64_t)(((rbytes[9] >> 2) | ((uint64_t)rbytes[10] << 6) | ((uint64_t)rbytes[11] << 14) | ((uint64_t)(rbytes[12] & 0xF) << 22))) & 0x3ffffff;
    r[4] = (uint64_t)(((rbytes[12] >> 4) | ((uint64_t)rbytes[13] << 4) | ((uint64_t)rbytes[14] << 12) | ((uint64_t)rbytes[15] << 20))) & 0x3ffffff;

    // accumulator
    uint64_t h[5] = {0,0,0,0,0};

    size_t offset = 0;
    while (mlen > 0) {
        // allocate 17 bytes so we can safely set block[take]=1 when take==16
        unsigned char block[17] = {0};
        size_t take = mlen >= 16 ? 16 : mlen;
        if (take > 16) take = 16; // defensive
        memcpy(block, m + offset, take);
        block[take] = 1; // append 1 (safe even when take==16)

        // parse block into 5 limbs
        uint64_t t0 = (uint64_t)(block[0] | ((uint64_t)block[1] << 8) | ((uint64_t)block[2] << 16) | ((uint64_t)block[3] << 24)) & 0x3ffffff;
        uint64_t t1 = (uint64_t)(((block[3] >> 2) | ((uint64_t)block[4] << 6) | ((uint64_t)block[5] << 14) | ((uint64_t)block[6] << 22))) & 0x3ffffff;
        uint64_t t2 = (uint64_t)(((block[6] >> 4) | ((uint64_t)block[7] << 4) | ((uint64_t)block[8] << 12) | ((uint64_t)block[9] << 20))) & 0x3ffffff;
        uint64_t t3 = (uint64_t)(((block[9] >> 2) | ((uint64_t)block[10] << 6) | ((uint64_t)block[11] << 14) | ((uint64_t)block[12] << 22))) & 0x3ffffff;
        uint64_t t4 = (uint64_t)(((block[12] >> 4) | ((uint64_t)block[13] << 4) | ((uint64_t)block[14] << 12) | ((uint64_t)block[15] << 20))) & 0x3ffffff;

        // h += t
        h[0] += t0;
        h[1] += t1;
        h[2] += t2;
        h[3] += t3;
        h[4] += t4;

        // h *= r mod (2^130-5)
        __uint128_t d0 = (__uint128_t)h[0]*r[0] + (__uint128_t)h[1]*5*r[4] + (__uint128_t)h[2]*5*r[3] + (__uint128_t)h[3]*5*r[2] + (__uint128_t)h[4]*5*r[1];
        __uint128_t d1 = (__uint128_t)h[0]*r[1] + (__uint128_t)h[1]*r[0] + (__uint128_t)h[2]*5*r[4] + (__uint128_t)h[3]*5*r[3] + (__uint128_t)h[4]*5*r[2];
        __uint128_t d2 = (__uint128_t)h[0]*r[2] + (__uint128_t)h[1]*r[1] + (__uint128_t)h[2]*r[0] + (__uint128_t)h[3]*5*r[4] + (__uint128_t)h[4]*5*r[3];
        __uint128_t d3 = (__uint128_t)h[0]*r[3] + (__uint128_t)h[1]*r[2] + (__uint128_t)h[2]*r[1] + (__uint128_t)h[3]*r[0] + (__uint128_t)h[4]*5*r[4];
        __uint128_t d4 = (__uint128_t)h[0]*r[4] + (__uint128_t)h[1]*r[3] + (__uint128_t)h[2]*r[2] + (__uint128_t)h[3]*r[1] + (__uint128_t)h[4]*r[0];

        // partial carry
        uint64_t c0 = (uint64_t)(d0 & 0x3ffffff);
        uint64_t carry = (uint64_t)(d0 >> 26);
        d1 += carry;

        uint64_t c1 = (uint64_t)(d1 & 0x3ffffff);
        carry = (uint64_t)(d1 >> 26);
        d2 += carry;

        uint64_t c2 = (uint64_t)(d2 & 0x3ffffff);
        carry = (uint64_t)(d2 >> 26);
        d3 += carry;

        uint64_t c3 = (uint64_t)(d3 & 0x3ffffff);
        carry = (uint64_t)(d3 >> 26);
        d4 += carry;

        uint64_t c4 = (uint64_t)(d4 & 0x3ffffff);
        carry = (uint64_t)(d4 >> 26);

        c0 += carry * 5;
        uint64_t c0c = c0 & 0x3ffffff;
        carry = c0 >> 26;
        c1 += carry;

        h[0] = c0c;
        h[1] = c1 & 0x3ffffff;
        h[2] = c2;
        h[3] = c3;
        h[4] = c4;

        offset += take;
        mlen -= take;
    }

    // fully carry
    uint64_t carry = h[1] >> 26; h[1] &= 0x3ffffff; h[2] += carry;
    carry = h[2] >> 26; h[2] &= 0x3ffffff; h[3] += carry;
    carry = h[3] >> 26; h[3] &= 0x3ffffff; h[4] += carry;
    carry = h[4] >> 26; h[4] &= 0x3ffffff; h[0] += carry * 5;
    carry = h[0] >> 26; h[0] &= 0x3ffffff; h[1] += carry;

    // compute h + -p
    uint64_t g0 = h[0] + 5;
    uint64_t g1 = h[1] + (g0 >> 26);
    uint64_t g2 = h[2] + (g1 >> 26);
    uint64_t g3 = h[3] + (g2 >> 26);
    uint64_t g4 = h[4] + (g3 >> 26);

    uint64_t mask = ((g4 >> 26) - 1) & 0xffffffffffffffffULL;
    // select h if h < p else h - p
    uint64_t final_h[5];
    final_h[0] = (h[0] & ~mask) | (g0 & mask);
    final_h[1] = (h[1] & ~mask) | (g1 & mask);
    final_h[2] = (h[2] & ~mask) | (g2 & mask);
    final_h[3] = (h[3] & ~mask) | (g3 & mask);
    final_h[4] = (h[4] & ~mask) | (g4 & mask);

    // convert the 130-bit number (final_h limbs) into a 128-bit little-endian value,
    // add the 128-bit 's' (key[16..31]) and produce the 16-byte tag.
    unsigned char s[16];
    memcpy(s, key + 16, 16);

    // build __uint128_t accumulator from limbs: h = h0 + h1*2^26 + h2*2^52 + h3*2^78 + h4*2^104
    __uint128_t acc = (__uint128_t)final_h[0]
        | ((__uint128_t)final_h[1] << 26)
        | ((__uint128_t)final_h[2] << 52)
        | ((__uint128_t)final_h[3] << 78)
        | ((__uint128_t)final_h[4] << 104);

    // load s as little-endian 128-bit
    uint64_t s_lo = load64_le(s);
    uint64_t s_hi = load64_le(s + 8);
    __uint128_t s_acc = (__uint128_t)s_lo | ((__uint128_t)s_hi << 64);

    acc += s_acc;

    // write out low 128 bits to tag (little-endian)
    for (int i = 0; i < 16; ++i) {
        tag[i] = (unsigned char)((acc >> (8 * i)) & 0xff);
    }
}

int poly1305_verify_tag(const unsigned char tag1[16], const unsigned char tag2[16]) {
    unsigned char diff = 0;
    for (int i = 0; i < 16; ++i) diff |= tag1[i] ^ tag2[i];
    return diff == 0;
}
