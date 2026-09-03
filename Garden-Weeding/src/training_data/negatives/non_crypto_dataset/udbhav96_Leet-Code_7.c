/*https://leetcode.com/problems/reverse-integer/description/*/
//Try 1 Final ( 10 min)
#include <limits.h>

int reverse(int x) {
    long sum = 0;

    while (x != 0) {
        int rem = x % 10;
        sum = sum * 10 + rem;
        x = x / 10;
    }
    if (sum > INT_MAX || sum < INT_MIN) {
        return 0;
    }

    return (int)sum;
}