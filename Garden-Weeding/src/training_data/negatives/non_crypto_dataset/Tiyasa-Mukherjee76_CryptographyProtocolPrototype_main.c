#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diffie_hellman.h"
#include "sha256.h"

int main() {
    // Initialize Diffie-Hellman parameters (small for demo, use larger in practice)
    uint64_t prime = 23; // Example small prime
    uint64_t generator = 5; // Example generator
    uint64_t alice_private = 6; // Alice's private key
    uint64_t bob_private = 15; // Bob's private key

    // Alice generates public key
    uint64_t alice_public = dh_generate_public_key(prime, generator, alice_private);
    printf("Alice's public key: %llu\n", alice_public);

    // Bob generates public key
    uint64_t bob_public = dh_generate_public_key(prime, generator, bob_private);
    printf("Bob's public key: %llu\n", bob_public);

    // Both compute shared secret
    uint64_t alice_shared = dh_compute_shared_secret(prime, bob_public, alice_private);
    uint64_t bob_shared = dh_compute_shared_secret(prime, alice_public, bob_private);
    printf("Alice's shared secret: %llu\n", alice_shared);
    printf("Bob's shared secret: %llu\n", bob_shared);

    // Verify shared secrets match
    if (alice_shared != bob_shared) {
        printf("Error: Shared secrets do not match!\n");
        return 1;
    }

    // Hash the shared secret using SHA-256
    uint8_t hash[SHA256_HASH_SIZE];
    sha256_hash((uint8_t*)&alice_shared, sizeof(alice_shared), hash);
    printf("SHA-256 hash of shared secret: ");
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    return 0;
}
