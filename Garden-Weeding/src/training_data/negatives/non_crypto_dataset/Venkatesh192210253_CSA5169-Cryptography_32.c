#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Simulated DSA signature with random k
void DSA_sign(const char* message, char* signature) {
    int k = rand(); // Random value k changes every time
    snprintf(signature, 100, "DSA_signature_%d_%s", k, message); // DSA signature varies due to k
}

// Simulated RSA signature (same result for the same message)
void RSA_sign(const char* message, char* signature) {
    snprintf(signature, 100, "RSA_signature_%s", message); // RSA signature is deterministic
}

int main() {
    srand(time(0)); // Seed for randomness

    char message[100];
    char dsa_signature1[100], dsa_signature2[100];
    char rsa_signature1[100], rsa_signature2[100];

    // Get user input for the message
    printf("Enter the message to be signed: ");
    fgets(message, 100, stdin);
    message[strcspn(message, "\n")] = 0; // Remove trailing newline

    // Signing with DSA
    DSA_sign(message, dsa_signature1);
    DSA_sign(message, dsa_signature2);

    // Signing with RSA
    RSA_sign(message, rsa_signature1);
    RSA_sign(message, rsa_signature2);

    // Print the DSA signatures
    printf("DSA Signature 1: %s\n", dsa_signature1);
    printf("DSA Signature 2: %s\n", dsa_signature2); // Will be different due to random k

    // Print the RSA signatures
    printf("RSA Signature 1: %s\n", rsa_signature1);
    printf("RSA Signature 2: %s\n", rsa_signature2); // Will be the same

    return 0;
}
