#include <stdio.h>

// Function prototypes
int gcdExtended(int a, int b, int *x, int *y);
int gcd(int a, int b);

int main() {
    // Input: Original RSA parameters
    int e = 31, d = 1087, n = 3599; // Public key, private key, and modulus
    int phi, p, q;

    // Step 1: Compute f(n) using leaked private key
    int x, y;
    gcdExtended(e, d * e - 1, &x, &y); // Solve for f(n) using the relationship (d * e) % f(n) = 1
    phi = (d * e - 1) / gcd(e, d * e - 1);

    // Step 2: Factorize n using f(n)
    for (p = 2; p < n; p++) {
        if (n % p == 0) {
            q = n / p;
            break;
        }
    }

    // Step 3: Analyze security of generating new keys
    printf("Modulus n: %d\n", n);
    printf("Leaked private key d: %d\n", d);
    printf("Computed f(n): %d\n", phi);
    printf("Factors of n: p = %d, q = %d\n", p, q);
    printf("Security Analysis:\n");
    printf("Once d is leaked, f(n) can be calculated, allowing factorization of n.\n");
    printf("Using the same modulus n for new keys is insecure, as f(n) compromises all future keys.\n");

    return 0;
}

// Extended Euclidean Algorithm to compute gcd and find modular inverse
int gcdExtended(int a, int b, int *x, int *y) {
    if (a == 0) {
        *x = 0, *y = 1;
        return b;
    }
    int x1, y1;
    int gcd = gcdExtended(b % a, a, &x1, &y1);
    *x = y1 - (b / a) * x1;
    *y = x1;
    return gcd;
}

// Compute GCD of two numbers
int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

