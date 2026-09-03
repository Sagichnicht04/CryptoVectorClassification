#include "SHA-1.h"

// 循环移位
uint32_t rotate_left(uint32_t value, uint32_t bits) {
    return (value << bits) | (value >> (32 - bits));
}

// 消息填充
std::vector<uint8_t> sha1_pad(const std::string& message) {
    std::vector<uint8_t> padded(message.begin(), message.end());
    uint64_t bit_len = message.size() * 8;

    // 追加bit 1
    padded.push_back(0x80);

    // 追加bit 0 直至448bits
    while ((padded.size() % 64) != 56) {
        padded.push_back(0x00);
    }

    // 追加消息的长度 mod2^64
    for (int i = 7; i >= 0; --i) {
        padded.push_back(static_cast<uint8_t>(bit_len >> (i * 8)));
    }

    return padded;
}


// sha1压缩函数 
void sha1_process_chunk(const uint8_t* chunk, uint32_t* H) {
    uint32_t W[80] = { 0 };

    // 消息字子分组 0-15
    for (int i = 0; i < 16; ++i) {
        W[i] = (chunk[i * 4] << 24) | (chunk[i * 4 + 1] << 16) |
            (chunk[i * 4 + 2] << 8) | chunk[i * 4 + 3];
    }

    // 导出的其余的64个子分组 16-79
    for (int i = 16; i < 80; ++i) {
        W[i] = rotate_left(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
    }

    // 初始化5个32bits寄存器
    uint32_t a = H[0], b = H[1], c = H[2], d = H[3], e = H[4];

    // 循环 4轮 80步，每一轮都有一个常数Kr，[2,3,5,10]开方，乘2^30，取整数部分
    for (int i = 0; i < 80; ++i) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        }
        else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        }
        else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        }
        else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = rotate_left(a, 5) + f + e + k + W[i];
        e = d;
        d = c;
        c = rotate_left(b, 30);
        b = a;
        a = temp;
    }

    // 最终的ABCD与最初输入的链接变量进行模加
    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
}


// SHA-1 函数
std::string sha1(const std::string& message) {
    // 初始化链接变量ABCDE
    uint32_t H[5] = {
        0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0
    };

    // 填充
    std::vector<uint8_t> padded = sha1_pad(message);

    // 压缩
    for (size_t i = 0; i < padded.size(); i += 64) {
        sha1_process_chunk(&padded[i], H);
    }

    // 组合最终的hash值
    std::ostringstream result;
    for (int i = 0; i < 5; ++i) {
        result << std::hex << std::setw(8) << std::setfill('0') << H[i];
    }

    return result.str();
}

