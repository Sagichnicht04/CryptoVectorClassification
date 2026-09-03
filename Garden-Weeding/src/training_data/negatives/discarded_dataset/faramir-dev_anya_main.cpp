#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include <fcntl.h>

extern "C" {
#include <sodium.h>
}

#include "fd.hpp"
#include "mmap.hpp"

// https://libsodium.gitbook.io/doc/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction

namespace {

constexpr std::string_view usage = R"(
Usage: xchacha20-poly1305 [options] <key-file> <input-file> <output-file>
options:
    -d, --decrypt   Decrypt input file
    -e, --encrypt   Encrypt input file (default)
)";

enum class Mode : bool {
    Encrypt,
    Decrypt
};

// Reads whole file into a string.
// Removes \n and \r at the end of the string.
[[nodiscard]] std::string read_whole_file(const std::filesystem::path& filepath) {
    auto mem = MMap::open(filepath);
    std::string result(reinterpret_cast<const char*>(*mem), mem.size());

    // Remove trailing whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }

    return result;
}

void encrypt_file(const std::filesystem::path& srcpath,
                 const std::filesystem::path& dstpath,
                 std::string_view password) {
    unsigned long long dstlen = 0;

    {
        auto src = MMap::open(srcpath);
        const size_t dst_max_size = +src
            + crypto_pwhash_SALTBYTES
            + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
            + crypto_aead_xchacha20poly1305_ietf_ABYTES;

        auto dst = MMap::create(dstpath, dst_max_size);
        unsigned char* p = *dst;

        // Generate salt
        std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
        randombytes_buf(salt.data(), salt.size());
        std::copy(salt.begin(), salt.end(), p);
        p += salt.size();

        // Generate nonce
        unsigned char* nonce = p;
        randombytes_buf(nonce, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
        p += crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

        // Derive key from password
        std::array<unsigned char, crypto_aead_xchacha20poly1305_ietf_KEYBYTES> key;
        if (crypto_pwhash(key.data(), key.size(),
                         password.data(), password.size(),
                         salt.data(),
                         crypto_pwhash_OPSLIMIT_INTERACTIVE,
                         crypto_pwhash_MEMLIMIT_INTERACTIVE,
                         crypto_pwhash_ALG_DEFAULT) != 0) {
            throw std::runtime_error("crypto_pwhash failed: out of memory");
        }

        // Encrypt
        if (crypto_aead_xchacha20poly1305_ietf_encrypt(
                p, &dstlen,
                *src, +src,
                salt.data(), salt.size(),
                nullptr,
                nonce,
                key.data()) != 0) {
            throw std::runtime_error("Encryption failed");
        }

        dstlen += salt.size();
        dstlen += crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

        if (dstlen > dst_max_size) {
            throw std::runtime_error(
                std::format("Encryption failed: output size {} exceeds maximum {}",
                            dstlen, dst_max_size));
        }
    }

    // Resize the dst file to the actual size
    if (ftruncate(*FD::open(dstpath, O_RDWR), static_cast<off_t>(dstlen)) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                std::format("Failed to truncate: {}", dstpath.string()));
    }
}

void decrypt_file(const std::filesystem::path& srcpath,
                 const std::filesystem::path& dstpath,
                 std::string_view password) {
    unsigned long long dstlen = 0;

    {
        auto src = MMap::open(srcpath);

        constexpr auto minimum_file_size = crypto_pwhash_SALTBYTES
            + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES
            + crypto_aead_xchacha20poly1305_ietf_ABYTES;

        if (minimum_file_size >= +src) {
            throw std::runtime_error(
                std::format("File too small: {} bytes (minimum: {} bytes)",
                          +src, minimum_file_size));
        }

        auto dst = MMap::create(dstpath, +src);

        const unsigned char* p = *src;
        const unsigned char* salt = p;
        p += crypto_pwhash_SALTBYTES;

        const unsigned char* nonce = p;
        p += crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

        const unsigned long long cryptotext_len =
            +src - crypto_pwhash_SALTBYTES - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

        // Derive key from password
        std::array<unsigned char, crypto_aead_xchacha20poly1305_ietf_KEYBYTES> key;
        if (crypto_pwhash(key.data(), key.size(),
                         password.data(), password.size(),
                         salt,
                         crypto_pwhash_OPSLIMIT_INTERACTIVE,
                         crypto_pwhash_MEMLIMIT_INTERACTIVE,
                         crypto_pwhash_ALG_DEFAULT) != 0) {
            throw std::runtime_error("crypto_pwhash failed: out of memory");
        }

        // Decrypt
        if (crypto_aead_xchacha20poly1305_ietf_decrypt(
                *dst, &dstlen,
                nullptr,
                p, cryptotext_len,
                salt, crypto_pwhash_SALTBYTES,
                nonce,
                key.data()) != 0) {
            throw std::runtime_error("Decryption failed: wrong password or corrupted file");
        }
    }

    // Resize the dst file to the actual size
    if (ftruncate(*FD::open(dstpath, O_RDWR), static_cast<off_t>(dstlen)) == -1) {
        throw std::system_error(errno, std::generic_category(),
                                std::format("Failed to truncate: {}", dstpath.string()));
    }
}

} // anonymous namespace

int main(int argc, char** argv)
try {
    // Initialize libsodium
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    // Convert to span for modern C++ access
    std::span<char*> args(argv, static_cast<size_t>(argc));
    Mode mode = Mode::Encrypt;

    // Parse arguments
    if (argc < 4 || argc > 5) {
        std::print("{}", usage);
        return 1;
    }

    // Check for help flag
    if (argc >= 2) {
        std::string_view arg1 = args[1];
        if (arg1 == "-h" || arg1 == "--help") {
            std::print("{}", usage);
            return 0;
        }
    }

    // Parse mode flag
    size_t arg_offset = 1;
    if (argc == 5) {
        std::string_view mode_flag = args[1];
        if (mode_flag == "-d" || mode_flag == "--decrypt") {
            mode = Mode::Decrypt;
        } else if (mode_flag == "-e" || mode_flag == "--encrypt") {
            mode = Mode::Encrypt;
        } else {
            std::println(stderr, "Unknown option: {}", mode_flag);
            std::print("{}", usage);
            return 1;
        }
        arg_offset = 2;
    }

    // Get file paths
    const std::filesystem::path passwordpath = args[arg_offset];
    const std::filesystem::path srcpath = args[arg_offset + 1];
    const std::filesystem::path dstpath = args[arg_offset + 2];

    // Read password
    const std::string password = read_whole_file(passwordpath);

    if (password.empty()) {
        throw std::runtime_error("Password file is empty");
    }

    // Perform encryption/decryption
    if (mode == Mode::Encrypt) {
        std::println("Encrypting {} -> {}", srcpath.string(), dstpath.string());
        encrypt_file(srcpath, dstpath, password);
        std::println("Encryption successful");
    } else {
        std::println("Decrypting {} -> {}", srcpath.string(), dstpath.string());
        decrypt_file(srcpath, dstpath, password);
        std::println("Decryption successful");
    }

    return 0;

} catch (const std::exception& e) {
    std::println(stderr, "Error: {}", e.what());
    return 2;
} catch (...) {
    std::println(stderr, "Unknown error :-(");
    return 3;
}
