#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef uint8_t u8;
typedef int64_t i64;
typedef i64 field_elem[16];

static void scalarmult(u8 *out, const u8 *scalar, const u8 *point);
static void randombytes(u8 *buffer, i64 size);

/* Implementation of field addition */
static void fadd(field_elem out, const field_elem a, const field_elem b) {
    int i;
    for (i = 0; i < 16; ++i) 
        out[i] = a[i] + b[i];
}

/* Implementation of field subtraction */
static void fsub(field_elem out, const field_elem a, const field_elem b) {
    int i;
    for (i = 0; i < 16; ++i) 
        out[i] = a[i] - b[i];
}

/* Converts a 32-byte array into a 16 element array  */
static void unpack25519(field_elem out, const u8 *in) {
    int i;
    for (i = 0; i < 16; ++i) 
        out[i] = in[2 * i] + ((i64) in[2 * i + 1] << 8);
    out[15] &= 0x7fff; // &= is bitwise AND + assignment, performs a mask because highest bit of public key ignored 
}

/* Ensures field_elem elements are within the correct range, handles overflow */
static void carry25519(field_elem elem) {
    int i;
    i64 carry;
    for (i = 0; i < 16; ++i) {
        carry = elem[i] >> 16;
        elem[i] -= carry << 16;
        if (i < 15) 
            elem[i + 1] += carry; 
        else 
            elem[0] += 38 * carry;
    }
}

/* Implementation of field multiplication, handles overflow */
static void fmul(field_elem out, const field_elem a, const field_elem b) {
    i64 i, j, product[31];
    for (i = 0; i < 31; ++i) 
        product[i] = 0;
    for (i = 0; i < 16; ++i) {
        for (j = 0; j < 16; ++j) 
            product[i + j] += a[i] * b[j];
    }
    for (i = 0; i < 15; ++i) 
        product[i] += 38 * product[i + 16];
    for (i = 0; i < 16; ++i) 
        out[i] = product[i];
    carry25519(out);
    carry25519(out);
}

/* Implemenation of field multiplicative inverse, uses Fermat's Little Theorem */
static void finverse(field_elem out, const field_elem in) {
    field_elem c;
    int i;
    for (i = 0; i < 16; ++i) 
        c[i] = in[i];
    for (i = 253; i >= 0; i--) { 
        fmul(c, c, c);
        if (i != 2 && i != 4) 
            fmul(c, c, in); // Using Fermat's Little Theorem
    }
    for (i = 0; i < 16; ++i) 
        out[i] = c[i];
}

/* Conditional swap of two field elements  */
static void swap25519(field_elem p, field_elem q, int bit) {
    i64 t, i, c = ~(bit - 1);
    for (i = 0; i < 16; ++i) {
        t = c & (p[i] ^ q[i]);
        p[i] ^= t; // Bitwise XOR + assignment
        q[i] ^= t;
    }
}

/* Reverse of unpack25519 */
static void pack25519(u8 *out, const field_elem in) {
    int i, j, carry;
    field_elem m, t;
    for (i = 0; i < 16; ++i) 
        t[i] = in[i];
    carry25519(t); 
    carry25519(t); 
    carry25519(t);
    for (j = 0; j < 2; ++j) {
        m[0] = t[0] - 0xffed;
        for(i = 1; i < 15; i++) {
            m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xffff;
        }
        m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
        carry = (m[15] >> 16) & 1;
        m[14] &= 0xffff;
        swap25519(t, m, 1 - carry);
    }
    for (i = 0; i < 16; ++i) {
        out[2 * i] = t[i] & 0xff;
        out[2 * i + 1] = t[i] >> 8;
    }
}

static const u8 _9[32] = {9};

void scalarmult_base(u8 *out, const u8 *scalar);
void generate_keypair(u8 *pk, u8 *sk);
void x25519(u8 *out, const u8 *pk, const u8 *sk);

static const field_elem _121665 = {0xDB41, 1}; 

/* 0xDB41 = 121665, this is (A - 2)/4, where A is the Montgomery parameter 486662
This is due to some optimisation in the Montgomery ladder step as described in 
Bernstein's paper: https://cr.yp.to/ecdh/curve25519-20060209.pdf /*

/* Performs Montgomery Curve Arithmetic using Montgomery Ladder */
void scalarmult(u8 *out, const u8 *scalar, const u8 *point) {
    u8 clamped[32];
    i64 bit, i;
    field_elem a, b, c, d, e, f, x;
    for (i = 0; i < 32; ++i) 
        clamped[i] = scalar[i];
    clamped[0] &= 0xf8;
    clamped[31] = (clamped[31] & 0x7f) | 0x40;
    unpack25519(x, point);
    for (i = 0; i < 16; ++i) {
        b[i] = x[i];
        d[i] = a[i] = c[i] = 0;
    }
    a[0] = d[0] = 1;
    for (i = 254; i >= 0; --i) {
        bit = (clamped[i >> 3] >> (i & 7)) & 1;
        swap25519(a, b, bit);
        swap25519(c, d, bit);
        fadd(e, a, c);
        fsub(a, a, c);
        fadd(c, b, d);
        fsub(b, b, d);
        fmul(d, e, e);
        fmul(f, a, a);
        fmul(a, c, a);
        fmul(c, b, e);
        fadd(e, a, c);
        fsub(a, a, c);
        fmul(b, a, a);
        fsub(c, d, f);
        fmul(a, c, _121665);
        fadd(a, a, d);
        fmul(c, c, a);
        fmul(a, d, f);
        fmul(d, b, x);
        fmul(b, e, e);
        swap25519(a, b, bit);
        swap25519(c, d, bit);
    }
    finverse(c, c);
    fmul(a, a, c);
    pack25519(out, a);
}

/* Starting point for EC operations, scalar will be private key value */
void scalarmult_base(u8 *out, const u8 *scalar) {
    scalarmult(out, scalar, _9);
}

/* Generates public and private keys */
void generate_keypair(u8 *pk, u8 *sk) {
    randombytes(sk, 32);
    scalarmult_base(pk, sk);
}

/* Performs Key Exchange Protocol */
void x25519(u8 *out, const u8 *pk, const u8 *sk) {
    scalarmult(out, sk, pk);
}

/* Random byte generation, NB: NOT cryptographically secure, using /dev/urandom would be ideal but this is for demonstration */
void randombytes(u8 *buffer, i64 size) {
    static int seeded = 0;
    if (!seeded) { // Seeds random number generation based on current time
        srand((unsigned int)time(NULL));
        seeded = 1;  // Ensures srand is called only once
    }
    for (i64 i = 0; i < size; ++i) {
        buffer[i] = rand() % 256; 
    }
}

/* Test Function */
int main() { 
    u8 A_private[32], A_public[32];
    u8 B_private[32], B_public[32];
    u8 shared_secret_A[32], shared_secret_B[32];

    generate_keypair(A_public, A_private);
    generate_keypair(B_public, B_private);

    x25519(shared_secret_A, B_public, A_private);
    x25519(shared_secret_B, A_public, B_private);

    if (memcmp(shared_secret_A, shared_secret_B, 32) == 0) {
        printf("Key exchange successful.\n");
    } else {
        printf("Key exchange unsuccessful.\n");
    }

    printf("Alice's Public Key: ");
    for (int i = 0; i < 32; i++) printf("%02x", A_public[i]);
    printf("\nBob's Public Key: ");
    for (int i = 0; i < 32; i++) printf("%02x", B_public[i]);
    printf("\nShared Secret (Alice): ");
    for (int i = 0; i < 32; i++) printf("%02x", shared_secret_A[i]);
    printf("\nShared Secret (Bob): ");
    for (int i = 0; i < 32; i++) printf("%02x", shared_secret_B[i]);
    printf("\n");

    return 0;
}
