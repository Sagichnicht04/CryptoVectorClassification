#include <stdio.h>

int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

int rsa_encrypt(int plaintext, int e, int n) {
    return (int)((long long)plaintext * plaintext) % n; 
}

int main() {
    int n = 323; 
    int e = 7;   
    int plaintext = 46; 
    printf("Original plaintext: %d\n", plaintext);

    if (gcd(plaintext, n) > 1) {
        printf("Plaintext has a common factor with n.\n");
        printf("This information might be useful for factoring n.\n");
    } else {
        printf("Plaintext does not have a common factor with n.\n");
    }

    int ciphertext = rsa_encrypt(plaintext, e, n);
    printf("Encrypted ciphertext: %d\n", ciphertext);

    return 0;
}