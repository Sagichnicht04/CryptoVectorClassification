#include "ecdh.h"
#include <random>

ECDHKeys ECDHKeys::generate_keys() {
    ECDHKeys keys;
    keys.private_key = rand() % 1000 + 1;
    keys.public_key = keys.private_key * 2 + 5; // 示例算法
    return keys;
}

int ECDHKeys::shared_secret(int other_public) const {
    // 示例算法
    return other_public * private_key + 7;
}