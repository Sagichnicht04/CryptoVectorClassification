/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * security/openssl.cpp
 * Copyright (C) 2020-2024 Emilien Kia <emilien.kia+dev@gmail.com>
 *
 * libcexxy is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcexxy is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */

#include "openssl.hpp"

#include "exceptions.hpp"

#include <iostream>
#include <functional>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>



// https://crypto.stackexchange.com/questions/32692/what-is-the-typical-block-size-in-rsa/32694#32694
// https://crypto.stackexchange.com/questions/2074/rsa-oaep-input-parameters


namespace cxy::security::openssl
{

cxy::math::big_integer bn2bi(const BIGNUM *bn)
{
    cxy::math::big_integer res;
    if(bn!=nullptr)
    {
        int sz = BN_num_bytes(bn);
        if(sz>0)
        {
            uint8_t*  buffer = new uint8_t[sz];
            int s = BN_bn2bin(bn, buffer);
            if(s>0)
            {
                res.assign<uint8_t>(buffer, sz, cxy::math::big_integer::WORD_MOST_SIGNIFICANT_FIRST, cxy::math::big_integer::MOST_SIGNIFICANT_FIRST);
            }
            delete [] buffer;
        }
    }
    return res;
}

BIGNUM* bi2bn(const cxy::math::big_integer& bi, BIGNUM* bn)
{
    std::vector<uint8_t> buffer = bi.get_vector<uint8_t>(cxy::math::big_integer::WORD_MOST_SIGNIFICANT_FIRST, cxy::math::big_integer::MOST_SIGNIFICANT_FIRST);
    if(!buffer.empty())
    {
        return BN_bin2bn(buffer.data(), buffer.size(), bn);
    }
    else
    {
        if(bn != nullptr)
            BN_zero(bn);
        return bn;
    }
}


//
// EVP Message Digest
//



std::map<std::string, std::function<const EVP_MD*()>> evp_md::_evp {
    {"NULL", EVP_md_null}
# ifndef OPENSSL_NO_MD2
    ,{"MD2", EVP_md2}
# endif
# ifndef OPENSSL_NO_MD4
    ,{"MD4", EVP_md4}
# endif
# ifndef OPENSSL_NO_MD5
    ,{"MD5", EVP_md5}
    ,{"MD5-SHA1", EVP_md5_sha1}
# endif
# ifndef OPENSSL_NO_BLAKE2
    ,{"BLAKE2S", EVP_blake2s256}
    ,{"BLAKE2B", EVP_blake2b512}
    ,{"BLAKE2-256", EVP_blake2s256}
    ,{"BLAKE2-512", EVP_blake2b512}
# endif
    ,{"SHA1", EVP_sha1}
    ,{"SHA2-224", EVP_sha224}, {"SHA-224", EVP_sha224}
    ,{"SHA2-256", EVP_sha256}, {"SHA-256", EVP_sha256}
    ,{"SHA2-384", EVP_sha384}, {"SHA-384", EVP_sha384}
    ,{"SHA2-512", EVP_sha512}, {"SHA-512", EVP_sha512}
    ,{"SHA2-512-224", EVP_sha512_224}, {"SHA-512-224", EVP_sha512_224}
    ,{"SHA2-512-256", EVP_sha512_256}, {"SHA-512-256", EVP_sha512_256}
    ,{"SHA3-224", EVP_sha3_224}
    ,{"SHA3-256", EVP_sha3_256}
    ,{"SHA3-384", EVP_sha3_384}
    ,{"SHA3-512", EVP_sha3_512}
    ,{"SHAKE-128", EVP_shake128}
    ,{"SHAKE-256", EVP_shake256}
# ifndef OPENSSL_NO_MDC2
    ,{"SMDC2", EVP_mdc2}
# endif
# ifndef OPENSSL_NO_RMD160
    ,{"RIPEMD-160", EVP_ripemd160}
# endif
# ifndef OPENSSL_NO_WHIRLPOOL
    ,{"WHIRLPOOL", EVP_whirlpool}
# endif
# ifndef OPENSSL_NO_SM3
    ,{"SM3", EVP_sm3}
#endif
};

const EVP_MD * evp_md::get_EVP_MD(const std::string& algorithm)
{
    auto it = _evp.find(algorithm);
    if (it!=_evp.end()) {
        return it->second();
    } else {
        return nullptr;
    }
}


std::shared_ptr<evp_md> evp_md::get(const std::string& algorithm)
{
    const EVP_MD * md = get_EVP_MD(algorithm);
    if (md!=nullptr) {
        return std::make_shared<evp_md>(md, algorithm);
    } else {
        throw no_such_algorithm_exception(algorithm + " is not supported");
    }
}

evp_md::evp_md(const EVP_MD *type, const std::string& algo):
_algo(algo)
{
    _mdctx = EVP_MD_CTX_create();
    if(EVP_DigestInit_ex(_mdctx, type, nullptr)==0)
    {
        throw digest_exception("Cannot initialize digest");
    }
}

evp_md::evp_md(const evp_md& other)
{
    _mdctx = EVP_MD_CTX_create();
    if(EVP_MD_CTX_copy_ex(_mdctx, other._mdctx)==0)
    {
        throw digest_exception("Cannot copy digest");
    }
}

evp_md::~evp_md()
{
    if(_mdctx!=nullptr) {
        EVP_MD_CTX_free(_mdctx);
        _mdctx = nullptr;
    }
}

std::string evp_md::algorithm() const
{
    return _algo;
}

uint16_t evp_md::digest_length() const
{
    return EVP_MD_CTX_size(_mdctx);
}

message_digest& evp_md::update(const void* data, size_t size)
{
    if(EVP_DigestUpdate(_mdctx, data, size)==0)
    {
        throw digest_exception("Cannot update digest");
    }
    return *this;
}

std::vector<uint8_t /*std::byte*/> evp_md::digest()
{
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    if(EVP_DigestFinal_ex(_mdctx, md_value, &md_len)==0)
    {
        throw digest_exception("Cannot update digest");
    }
    return std::vector<uint8_t /*std::byte*/>(md_value, md_value+md_len);
}

message_digest& evp_md::reset()
{
    EVP_MD_CTX_reset(_mdctx);
    return *this;
}


//
// EVP Signature
//

std::shared_ptr<evp_sign> evp_sign::get(const cipher_builder& bldr)
{
    const private_key* key = dynamic_cast<const private_key*>(bldr.key());
    if(bldr.key()==nullptr || key==nullptr) {
        throw invalid_key_exception("A private key must be specified.");
    }

    // Could be empty if key alogirthm support it,
    // Typically RSA with no padding or CMAC, Poly1305 and SipHash
    // See https://www.openssl.org/docs/man1.1.1/man3/EVP_DigestSignInit.html
    const EVP_MD *md = nullptr;
    if(!bldr.md().empty()) {
        md = evp_md::get_EVP_MD(bldr.md());
        if(md==nullptr) {
            throw no_such_algorithm_exception(bldr.md() + " is not supported");
        }
    }

    EVP_PKEY* pkey = evp_pkey_cipher::get(bldr.algorithm(), key);

    // TODO set padding data

    return std::make_shared<evp_sign>(md, bldr.algorithm(), key, pkey);
}

evp_sign::evp_sign(const EVP_MD *md, const std::string& algo, const private_key* key, EVP_PKEY* pkey):
_algo(algo),
_pkey(key)
{
    _mdctx = EVP_MD_CTX_create();
    int res = EVP_DigestSignInit(_mdctx, &_pkctx, md, nullptr, pkey);
    if (res == -2) {
        throw invalid_key_exception("Signing operation is not supported by the key algorithm.");
    } else if (res <= 0) {
        throw invalid_key_exception("Error in signature initialization.");
    }

// TODO : conditionnalize that:
//    if(EVP_PKEY_CTX_set_rsa_padding(_pkctx, RSA_PKCS1_PADDING) <= 0) {
//        throw invalid_key_exception(/*...*/);
//    }
}

evp_sign::~evp_sign()
{
    if(_mdctx!=nullptr) {
        EVP_MD_CTX_free(_mdctx);
        _mdctx = nullptr;
    }
}

signature& evp_sign::update(const void* data, size_t size)
{
    if(EVP_DigestSignUpdate(_mdctx, data, size)==0) {
        throw digest_exception("Cannot update signature");
    }
    return *this;
}

std::vector<uint8_t /*std::byte*/> evp_sign::sign()
{
    size_t len;
    if(EVP_DigestSignFinal(_mdctx, nullptr, &len)<=0) {
        throw digest_exception("Problem while getting signature size.");
    }
    std::vector<uint8_t> buff(len);
    if(EVP_DigestSignFinal(_mdctx, (unsigned char *)buff.data(), &len)<=0) {
        throw digest_exception("Problem while computing signature.");
    }
    buff.resize(len);
    return buff;
}

//
// EVP Verifier
//


std::shared_ptr<evp_verify> evp_verify::get(const cipher_builder& bldr)
{
    const public_key* key = dynamic_cast<const public_key*>(bldr.key());
    if(bldr.key()==nullptr || key==nullptr) {
        throw invalid_key_exception("A public key must be specified.");
    }

    // Could be empty if key alogirthm support it,
    // Typically RSA with no padding or CMAC, Poly1305 and SipHash
    // See https://www.openssl.org/docs/man1.1.1/man3/EVP_DigestSignInit.html
    const EVP_MD *md = nullptr;
    if(!bldr.md().empty()) {
        md = evp_md::get_EVP_MD(bldr.md());
        if(md==nullptr) {
            throw no_such_algorithm_exception(bldr.md() + " is not supported");
        }
    }

    EVP_PKEY* pkey = evp_pkey_cipher::get(bldr.algorithm(), key);

    // TODO set padding data

    return std::make_shared<evp_verify>(md, bldr.algorithm(), key, pkey);
}

evp_verify::evp_verify(const EVP_MD *md, const std::string& algo, const public_key* key, EVP_PKEY* pkey):
_algo(algo),
_pkey(key)
{
    _mdctx = EVP_MD_CTX_create();
    int res = EVP_DigestVerifyInit(_mdctx, &_pkctx, md, nullptr, pkey);
    if (res == -2) {
        throw invalid_key_exception("Verifying operation is not supported by the key algorithm.");
    } else if (res <= 0) {
        throw invalid_key_exception("Error in verifier initialization.");
    }

// TODO : conditionnalize that:
//    if(EVP_PKEY_CTX_set_rsa_padding(_pkctx, RSA_PKCS1_PADDING) <= 0) {
//        throw invalid_key_exception(/*...*/);
//    }
}

evp_verify::~evp_verify()
{
    if(_mdctx!=nullptr) {
        EVP_MD_CTX_free(_mdctx);
        _mdctx = nullptr;
    }
}

verifier& evp_verify::update(const void* data, size_t size)
{
    if(EVP_DigestVerifyUpdate(_mdctx, data, size)==0) {
        throw digest_exception("Cannot update verifier");
    }
    return *this;
}

bool evp_verify::verify(const void* data, size_t size)
{
    int res = EVP_DigestVerifyFinal(_mdctx, (const unsigned char *)data, size);
    if(res==1) {
        return true;
    } else if(res==0) {
        return false;
    } else {
        throw digest_exception("Problem while ferifying signature.");
    }
}

//
// EVP symmetric cipher
//

struct symetric_cipher_desc {
    std::string name;
    std::string mode;
    int key_size; // in bytes, -1 means no fixed size
    int iv_size; // iv size in bytes (= block size), 0 means no iv, -1 means no fixed size
    std::function<const EVP_CIPHER*()> fct;
};

symetric_cipher_desc _sym_ciphers [] = {
    {"AES", "CBC",    16, 16, EVP_aes_128_cbc },
    {"AES", "CBC",    24, 16, EVP_aes_192_cbc },
    {"AES", "CBC",    32, 16, EVP_aes_256_cbc },
    {"AES", "CFB",    16, 16, EVP_aes_128_cfb },
    {"AES", "CFB",    24, 16, EVP_aes_192_cfb },
    {"AES", "CFB",    32, 16, EVP_aes_256_cfb },
    {"AES", "CFB1",   16, 16, EVP_aes_128_cfb1 },
    {"AES", "CFB1",   24, 16, EVP_aes_192_cfb1 },
    {"AES", "CFB1",   32, 16, EVP_aes_256_cfb1 },
    {"AES", "CFB8",   16, 16, EVP_aes_128_cfb8 },
    {"AES", "CFB8",   24, 16, EVP_aes_192_cfb8 },
    {"AES", "CFB8",   32, 16, EVP_aes_256_cfb8 },
    {"AES", "CFB128", 16, 16, EVP_aes_128_cfb128 },
    {"AES", "CFB128", 24, 16, EVP_aes_192_cfb128 },
    {"AES", "CFB128", 32, 16, EVP_aes_256_cfb128 },
    {"AES", "CTR",    16, 16, EVP_aes_128_ctr },
    {"AES", "CTR",    24, 16, EVP_aes_192_ctr },
    {"AES", "CTR",    32, 16, EVP_aes_256_ctr },
    {"AES", "ECB",    16,  0, EVP_aes_128_ecb },
    {"AES", "ECB",    24,  0, EVP_aes_192_ecb },
    {"AES", "ECB",    32,  0, EVP_aes_256_ecb },
    {"AES", "OFB",    16, 16, EVP_aes_128_ofb },
    {"AES", "OFB",    24, 16, EVP_aes_192_ofb },
    {"AES", "OFB",    32, 16, EVP_aes_256_ofb },

#ifndef OPENSSL_NO_ARIA
    {"ARIA", "CBC",    16, 16, EVP_aria_128_cbc},
    {"ARIA", "CBC",    24, 16, EVP_aria_192_cbc},
    {"ARIA", "CBC",    32, 16, EVP_aria_256_cbc},
    {"ARIA", "CFB",    16, 16, EVP_aria_128_cfb},
    {"ARIA", "CFB",    24, 16, EVP_aria_192_cfb},
    {"ARIA", "CFB",    32, 16, EVP_aria_256_cfb},
    {"ARIA", "CFB1",   16, 16, EVP_aria_128_cfb1},
    {"ARIA", "CFB1",   24, 16, EVP_aria_192_cfb1},
    {"ARIA", "CFB1",   32, 16, EVP_aria_256_cfb1},
    {"ARIA", "CFB8",   16, 16, EVP_aria_128_cfb8},
    {"ARIA", "CFB8",   24, 16, EVP_aria_192_cfb8},
    {"ARIA", "CFB8",   32, 16, EVP_aria_256_cfb8},
    {"ARIA", "CFB128", 16, 16, EVP_aria_128_cfb128},
    {"ARIA", "CFB128", 24, 16, EVP_aria_192_cfb128},
    {"ARIA", "CFB128", 32, 16, EVP_aria_256_cfb128},
    {"ARIA", "CTR",    16, 16, EVP_aria_128_ctr},
    {"ARIA", "CTR",    24, 16, EVP_aria_192_ctr},
    {"ARIA", "CTR",    32, 16, EVP_aria_256_ctr},
    {"ARIA", "ECB",    16,  0, EVP_aria_128_ecb},
    {"ARIA", "ECB",    24,  0, EVP_aria_192_ecb},
    {"ARIA", "ECB",    32,  0, EVP_aria_256_ecb},
    {"ARIA", "OFB",    16, 16, EVP_aria_128_ofb},
    {"ARIA", "OFB",    24, 16, EVP_aria_192_ofb},
    {"ARIA", "OFB",    32, 16, EVP_aria_256_ofb},
#endif // OPENSSL_NO_ARIA

#ifndef OPENSSL_NO_BF
    {"Blowfish", "CBC",   -1, 8, EVP_bf_cbc},
    {"Blowfish", "CFB",   -1, 8, EVP_bf_cfb},
    {"Blowfish", "CFB64", -1, 8, EVP_bf_cfb64},
    {"Blowfish", "ECB",   -1, 0, EVP_bf_ecb},
    {"Blowfish", "OFB",   -1, 8, EVP_bf_ofb},
#endif // OPENSSL_NO_ARIA

#ifndef OPENSSL_NO_CAMELLIA
    {"Camellia", "CBC",    16, 16, EVP_camellia_128_cbc},
    {"Camellia", "CBC",    24, 16, EVP_camellia_192_cbc},
    {"Camellia", "CBC",    32, 16, EVP_camellia_256_cbc},
    {"Camellia", "CFB",    16, 16, EVP_camellia_128_cfb},
    {"Camellia", "CFB",    24, 16, EVP_camellia_192_cfb},
    {"Camellia", "CFB",    32, 16, EVP_camellia_256_cfb},
    {"Camellia", "CFB1",   16, 16, EVP_camellia_128_cfb1},
    {"Camellia", "CFB1",   24, 16, EVP_camellia_192_cfb1},
    {"Camellia", "CFB1",   32, 16, EVP_camellia_256_cfb1},
    {"Camellia", "CFB8",   16, 16, EVP_camellia_128_cfb8},
    {"Camellia", "CFB8",   24, 16, EVP_camellia_192_cfb8},
    {"Camellia", "CFB8",   32, 16, EVP_camellia_256_cfb8},
    {"Camellia", "CFB128", 16, 16, EVP_camellia_128_cfb128},
    {"Camellia", "CFB128", 24, 16, EVP_camellia_192_cfb128},
    {"Camellia", "CFB128", 32, 16, EVP_camellia_256_cfb128},
    {"Camellia", "CTR",    16, 16, EVP_camellia_128_ctr},
    {"Camellia", "CTR",    24, 16, EVP_camellia_192_ctr},
    {"Camellia", "CTR",    32, 16, EVP_camellia_256_ctr},
    {"Camellia", "ECB",    16,  0, EVP_camellia_128_ecb},
    {"Camellia", "ECB",    24,  0, EVP_camellia_192_ecb},
    {"Camellia", "ECB",    32,  0, EVP_camellia_256_ecb},
    {"Camellia", "OFB",    16, 16, EVP_camellia_128_ofb},
    {"Camellia", "OFB",    24, 16, EVP_camellia_192_ofb},
    {"Camellia", "OFB",    32, 16, EVP_camellia_256_ofb},
#endif // OPENSSL_NO_CAMELLIA

#ifndef OPENSSL_NO_CAST
    {"Cast5", "CBC",   -1, 8, EVP_bf_cbc},
    {"Cast5", "CFB",   -1, 8, EVP_bf_cfb},
    {"Cast5", "CFB64", -1, 8, EVP_bf_cfb64},
    {"Cast5", "ECB",   -1, 0, EVP_bf_ecb},
    {"Cast5", "OFB",   -1, 8, EVP_bf_ofb},
#endif // OPENSSL_NO_CAST

#ifndef OPENSSL_NO_IDEA
    {"IDEA", "CBC",   16, 8, EVP_idea_cbc},
    {"IDEA", "CFB",   16, 8, EVP_idea_cfb},
    {"IDEA", "CFB64", 16, 8, EVP_idea_cfb64},
    {"IDEA", "ECB",   16, 0, EVP_idea_ecb},
    {"IDEA", "OFB",   16, 8, EVP_idea_ofb},
#endif // OPENSSL_NO_IDEA

#ifndef OPENSSL_NO_SM4
    {"SM4", "CBC",    16, 16, EVP_sm4_cbc},
    {"SM4", "CFB",    16, 16, EVP_sm4_cfb},
    {"SM4", "CFB128", 16, 16, EVP_sm4_cfb128},
    {"SM4", "CTR",    16, 16, EVP_sm4_ctr},
    {"SM4", "ECB",    16,  0, EVP_sm4_ecb},
    {"SM4", "OFB",    16, 16, EVP_sm4_ofb},
#endif // OPENSSL_NO_SM4

#ifndef OPENSSL_NO_CHACHA
    {"ChaCha20", "", 32, 16, EVP_chacha20},
#ifndef OPENSSL_NO_POLY1305
    {"ChaCha20-Poly1305", "", 32, 12, EVP_chacha20_poly1305}, // AEAD
#endif
#endif

};


const EVP_CIPHER* evp_cipher::find_EVP_CIPHER(const cipher_builder& bldr)
{
    // TODO normalize params

    bool find_algo = false;
    bool find_mode = false;
    for(auto& cdesc : _sym_ciphers) {
        if(bldr.algorithm()==cdesc.name) {
            find_algo = true;
            if(bldr.mode()==cdesc.mode) {
                find_mode = true;
                return cdesc.fct();
            }
        }
    }

    if (!find_algo) {
        throw no_such_algorithm_exception("No corresponding cipher.");
    } else if (!find_mode) {
        throw no_such_algorithm_exception("No corresponding mode.");
    } else {
        throw invalid_key_exception("Key size not supported.");
    }
}


std::shared_ptr<cipher> evp_cipher::get(const std::string& algorithm, const std::string& mode, const std::string& padding, const cxy::security::key* key, const std::vector<uint8_t/*std::byte*/>& iv, bool encrypt)
{
    const cxy::security::secret_key* seckey = dynamic_cast<const cxy::security::secret_key*>(key);
    // Assume a key is present
    if(seckey == nullptr) {
        return nullptr; // A key is mandatory
    }

    bool pad;
    if(padding.empty()) {
        pad = true;
    } else if(padding==CXY_CIPHER_NO_PADDING) {
        pad = false;
    } else  if(padding==CXY_CIPHER_PKCS5_PADDING || padding==CXY_CIPHER_PKCS7_PADDING ) {
        //pad = padding;
        // PKCS#5 is a substract of PKCS#7 padding., use them as an alias for legacy compatibility
        // See https://crypto.stackexchange.com/questions/9043/what-is-the-difference-between-pkcs5-padding-and-pkcs7-padding
        pad = true;
    } else {
        std::ostringstream stm;
        stm << "Unsupported padding : " << padding;
        throw no_such_padding_exception(stm.str());
    }

    // TODO normalize params

    bool find_algo = false;
    bool find_mode = false;
    for(auto& cdesc : _sym_ciphers) {
        if(algorithm==cdesc.name) {
            find_algo = true;
            if(mode==cdesc.mode) {
                find_mode = true;
                if(cdesc.key_size==-1 || seckey->size()==cdesc.key_size) {
                    const EVP_CIPHER* c = cdesc.fct();
                    int ivsz = EVP_CIPHER_iv_length(c);
                    if(iv.size()==ivsz) {
                        return std::make_shared<evp_cipher>(c, pad, seckey->value().data(), iv.data(), encrypt);
                    } else {
                        std::ostringstream stm;
                        stm << "Bad IV size (" << iv.size() << " / " << ivsz << " expected)";
                        throw invalid_key_exception(stm.str());
                    }
                }
            }
        }
    }

    if (!find_algo) {
        throw no_such_algorithm_exception("No corresponding cipher.");
    } else if (!find_mode) {
        throw no_such_algorithm_exception("No corresponding mode.");
    } else {
        throw invalid_key_exception("Key size not supported.");
    }

}

evp_cipher::evp_cipher(const EVP_CIPHER *type, bool padding, const unsigned char *key, const unsigned char *iv, bool enc)
{
    _ctx = EVP_CIPHER_CTX_new();

    if(!EVP_CipherInit(_ctx, type, key, iv, enc?1:0)) {
        throw invalid_key_exception("Error when instatiating cipher.");
    }

    if(!EVP_CIPHER_CTX_set_padding(_ctx, padding ? 1 : 0)) {
        throw invalid_key_exception("Error when setting padding to cipher.");
    }
}

evp_cipher::~evp_cipher()
{
    if(_ctx!=nullptr) {
        EVP_CIPHER_CTX_free(_ctx);
        _ctx = nullptr;
    }
}

cipher& evp_cipher::update_aad(const void* data, size_t sz)
{
    if(!EVP_CipherUpdate(_ctx, nullptr, nullptr, (const unsigned char*)data, sz)) {
        throw invalid_key_exception("Error while streaming aad data to cipher.");
    }
    return *this;
}

std::vector<uint8_t/*std::byte*/> evp_cipher::update(const void* data, size_t sz)
{
    std::vector<uint8_t/*std::byte*/> res;
    unsigned char buffer[1024 + EVP_MAX_BLOCK_LENGTH];
    int len;
    const unsigned char* ptr = (const unsigned char*)data;

    while(sz>0) {
        int s = std::min((int)sz, 1024);
        if(!EVP_CipherUpdate(_ctx, buffer, &len, ptr, s)) {
            throw invalid_key_exception("Error while streaming data to cipher.");
        }
        res.insert(res.end(), buffer, buffer+len);
        sz -= s;
        ptr += s;
    }

    return res;
}

std::vector<uint8_t/*std::byte*/> evp_cipher::finalize()
{
    std::vector<uint8_t/*std::byte*/> res;
    unsigned char buffer[EVP_MAX_BLOCK_LENGTH];
    int len;

    if(!EVP_CipherFinal_ex(_ctx, buffer, &len)) {
        throw invalid_key_exception("Error while streaming data to cipher.");
    }
    res.insert(res.end(), buffer, buffer+len);

    return res;
}

std::vector<uint8_t/*std::byte*/> evp_cipher::finalize(const void* data, size_t sz)
{
    std::vector<uint8_t/*std::byte*/> res;
    unsigned char buffer[1024 + EVP_MAX_BLOCK_LENGTH];
    int len;
    const unsigned char* ptr = (const unsigned char*)data;

    while(sz>0) {
        int s = std::min((int)sz, 1024);
        if(!EVP_CipherUpdate(_ctx, buffer, &len, ptr, s)) {
            throw invalid_key_exception("Error while streaming data to cipher.");
        }
        res.insert(res.end(), buffer, buffer+len);
        sz -= s;
        ptr += s;
    }

    if(!EVP_CipherFinal_ex(_ctx, buffer, &len)) {
        throw invalid_key_exception("Error while finalizing ciphering.");
    }
    res.insert(res.end(), buffer, buffer+len);

    return res;
}


//
// EVP PKEY
//

EVP_PKEY* evp_pkey_cipher::get(const std::string& algorithm, const cxy::security::key* key) {
   // Assume a key is present
    if(key == nullptr) {
        return nullptr; // A key is mandatory
    }

    //
    // RSA
    //
    const rsa_key* rsakey = dynamic_cast<const rsa_key*>(key);
    if(rsakey!=nullptr) {
        if(!algorithm.empty() && algorithm!=CXY_KEY_RSA) {
            throw invalid_key_exception("Key type mismatch.");
        }

        const ossl_rsa_key* orsakey = dynamic_cast<const ossl_rsa_key*>(rsakey);
        if(orsakey==nullptr) {
            // TODO convert key to openssl
            throw security_exception("Not implemented yet");
        }

        EVP_PKEY *pkey = EVP_PKEY_new();
        if(!EVP_PKEY_set1_RSA(pkey, const_cast<RSA*>(orsakey->get()))) {
            throw invalid_key_exception("Cannot assign RSA key");
        }

        return pkey;
    }

    // Not supported key type
    return nullptr;
}


std::shared_ptr<cipher> evp_pkey_cipher::get(const std::string& algorithm, const std::string& padding, const std::string& md, const cxy::security::key* key, bool encrypt) {
    // Assume a key is present
    if(key == nullptr) {
        return nullptr; // A key is mandatory
    }

    //
    // RSA
    //
    EVP_PKEY* pkey = get(algorithm, key);

    const rsa_key* rsakey = dynamic_cast<const rsa_key*>(key);
    if(rsakey!=nullptr) {

        EVP_PKEY_PADDING_MODE padmode = RSA_PAD_NONE;
        if(padding==CXY_CIPHER_PKCS1_PADDING) {
            padmode = RSA_PAD_PKCS1;
        } else if(padding==CXY_CIPHER_PKCS1_OAEP_PADDING) {
            padmode = RSA_PAD_OAEP;
        } else if(!padding.empty() && padding!=CXY_CIPHER_NO_PADDING) {
            std::ostringstream stm;
            stm << "Unsupported padding mode '" << padding << "' for RSA.";
            throw no_such_padding_exception(stm.str());
        }

        return std::dynamic_pointer_cast<cipher>(std::make_shared<evp_pkey_cipher>(pkey, padmode, md, encrypt));
    }

    throw no_such_algorithm_exception();
}

evp_pkey_cipher::evp_pkey_cipher(EVP_PKEY *pkey, EVP_PKEY_PADDING_MODE padding, const std::string& md, bool enc) : _encode(enc) {
    _ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if(!_ctx) {
        throw invalid_key_exception(/*...*/);
    }

    if((enc ? EVP_PKEY_encrypt_init : EVP_PKEY_decrypt_init)(_ctx) <= 0) {
        throw invalid_key_exception(/*...*/);
    }

    switch(padding) {
        case RSA_PAD_NONE:
            if(EVP_PKEY_CTX_set_rsa_padding(_ctx, RSA_NO_PADDING) <= 0) {
                throw invalid_key_exception(/*...*/);
            }
            break;
        case RSA_PAD_PKCS1:
            if(EVP_PKEY_CTX_set_rsa_padding(_ctx, RSA_PKCS1_PADDING) <= 0) {
                throw invalid_key_exception(/*...*/);
            }
            break;
        case RSA_PAD_OAEP:
            if(EVP_PKEY_CTX_set_rsa_padding(_ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
                throw invalid_key_exception(/*...*/);
            }
            if(!md.empty()) {
                const EVP_MD * evpmd = evp_md::get_EVP_MD(md);
                if(evpmd!=nullptr) {
                    if(EVP_PKEY_CTX_set_rsa_oaep_md(_ctx, evpmd) <= 0) {
                        throw invalid_key_exception(/*...*/);
                    }
                } else {
                    throw no_such_padding_exception(std::string("Unsupported OAEP MD algorithm ") + md);
                }
            }
            break;
    }
}

evp_pkey_cipher::~evp_pkey_cipher() {
    if(_ctx!=nullptr) {
        EVP_PKEY_CTX_free(_ctx);
        _ctx = nullptr;
    }
}

cipher& evp_pkey_cipher::update_aad(const void* data, size_t sz) {
    // TODO / Do nothing
    return *this;
}

std::vector<uint8_t/*std::byte*/> evp_pkey_cipher::update(const void* data, size_t sz) {
    throw invalid_key_exception("Update not supported for RSA ciphering");
}

std::vector<uint8_t/*std::byte*/> evp_pkey_cipher::finalize() {
    throw invalid_key_exception("Empty finalize not supported for RSA ciphering");
}

std::vector<uint8_t/*std::byte*/> evp_pkey_cipher::finalize(const void* data, size_t sz) {
    size_t outlen;
    if((_encode ? EVP_PKEY_encrypt : EVP_PKEY_decrypt)(_ctx, nullptr, &outlen, (const uint8_t*)data, sz) <= 0) {
        std::ostringstream stm;
        stm << "Problem in " << (_encode ? "encrypt" : "decrypt") << " size calculation.";
        throw invalid_key_exception(stm.str());
    }

    char errbuff[1024];

    std::vector<uint8_t> res(outlen);
    int r = (_encode ? EVP_PKEY_encrypt : EVP_PKEY_decrypt)(_ctx, res.data(), &outlen, (const uint8_t*)data, sz);
    if(r <= 0) {
        ERR_error_string_n(ERR_get_error() , errbuff, 1024);
        std::ostringstream stm;
        stm << "Problem in " << (_encode ? "encrypting" : "decrypting") << " : " << r << " : " << errbuff;
        throw invalid_key_exception(stm.str());
    }
    res.resize(outlen);
    return res;
}




//
// RSA
//

inline RSA_sptr make_rsa(RSA* rsa) {
    return RSA_sptr(rsa, RSA_free);
}



//
// ossl_rsa_key
//

ossl_rsa_key::ossl_rsa_key(RSA* rsa) :
_rsa(make_rsa(rsa))
{
}

cxy::math::big_integer ossl_rsa_key::modulus() const {
    return bn2bi(RSA_get0_n(_rsa.get()));
}

size_t ossl_rsa_key::modulus_size() const {
    return RSA_bits(_rsa.get());
}

//
// ossl_rsa_public_key
//

ossl_rsa_public_key::ossl_rsa_public_key(RSA* rsa) :
ossl_rsa_key(rsa)
{
}

cxy::math::big_integer ossl_rsa_public_key::public_exponent() const {
    return bn2bi(RSA_get0_e(_rsa.get()));
}

//
// ossl_rsa_private_key
//

ossl_rsa_private_key::ossl_rsa_private_key(RSA* rsa) :
ossl_rsa_key(rsa)
{
}

cxy::math::big_integer ossl_rsa_private_key::private_exponent() const {
    return bn2bi(RSA_get0_d(_rsa.get()));
}

//
// ossl_rsa_private_crt_key
//

ossl_rsa_private_crt_key::ossl_rsa_private_crt_key(RSA* rsa) :
ossl_rsa_key(rsa)
{
}

cxy::math::big_integer ossl_rsa_private_crt_key::private_exponent() const {
    return ossl_rsa_private_key::private_exponent();
}

cxy::math::big_integer ossl_rsa_private_crt_key::crt_coefficient() const {
    return bn2bi(RSA_get0_iqmp(_rsa.get()));
}

cxy::math::big_integer ossl_rsa_private_crt_key::prime_exponent_p() const {
    return bn2bi(RSA_get0_dmp1(_rsa.get()));
}

cxy::math::big_integer ossl_rsa_private_crt_key::prime_exponent_q() const {
    return bn2bi(RSA_get0_dmq1(_rsa.get()));
}

cxy::math::big_integer ossl_rsa_private_crt_key::prime_p() const {
    return bn2bi(RSA_get0_p(_rsa.get()));
}

cxy::math::big_integer ossl_rsa_private_crt_key::prime_q() const {
    return bn2bi(RSA_get0_q(_rsa.get()));
}

cxy::math::big_integer ossl_rsa_private_crt_key::public_exponent() const {
    return bn2bi(RSA_get0_e(_rsa.get()));
}

//
// ossl_rsa_multiprime_private_crt_key
//

ossl_rsa_multiprime_private_crt_key::ossl_rsa_multiprime_private_crt_key(RSA* rsa) :
ossl_rsa_key(rsa)
{
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::private_exponent() const {
    return ossl_rsa_private_crt_key::private_exponent();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::crt_coefficient() const {
    return ossl_rsa_private_crt_key::crt_coefficient();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::prime_exponent_p() const {
    return ossl_rsa_private_crt_key::prime_exponent_p();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::prime_exponent_q() const {
    return ossl_rsa_private_crt_key::prime_exponent_q();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::prime_p() const {
    return ossl_rsa_private_crt_key::prime_p();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::prime_q() const {
    return ossl_rsa_private_crt_key::prime_q();
}

cxy::math::big_integer ossl_rsa_multiprime_private_crt_key::public_exponent() const {
    return ossl_rsa_private_crt_key::public_exponent();
}

std::vector<cxy::math::big_integer> ossl_rsa_multiprime_private_crt_key::other_prime_info() const {
    // TODO
    throw key_exception("Not implemnted yet");
}



std::shared_ptr<ossl_rsa_private_key> make_rsa_private_key(RSA* rsa) {
    if(rsa!=nullptr) {
        if (RSA_get_version(rsa)==RSA_ASN1_VERSION_MULTI) {
            return std::dynamic_pointer_cast<ossl_rsa_private_key>(std::make_shared<ossl_rsa_multiprime_private_crt_key>(rsa));
        } else if(RSA_get0_p(rsa)  != nullptr && RSA_get0_dmp1(rsa) != nullptr
                && RSA_get0_q(rsa) != nullptr && RSA_get0_dmq1(rsa) != nullptr
                && RSA_get0_iqmp(rsa) != nullptr ) {
            return std::dynamic_pointer_cast<ossl_rsa_private_key>(std::make_shared<ossl_rsa_private_crt_key>(rsa));
        } else {
            return std::dynamic_pointer_cast<ossl_rsa_private_key>(std::make_shared<ossl_rsa_private_key>(rsa));
        }
    } else {
        return nullptr;
    }
}

//
// ossl_rsa_key_pair
//

ossl_rsa_key_pair::ossl_rsa_key_pair(RSA* rsa):
_rsa(make_rsa(rsa))
{}

std::shared_ptr<cxy::security::rsa_public_key> ossl_rsa_key_pair::rsa_public_key() const {
    if(!_pub) {
        _pub = std::make_shared<ossl_rsa_public_key>(RSAPublicKey_dup(_rsa.get()));
    }
    return _pub;
}

std::shared_ptr<cxy::security::rsa_private_key> ossl_rsa_key_pair::rsa_private_key() const {
    if(!_priv) {
        _priv = make_rsa_private_key(RSAPrivateKey_dup(_rsa.get()));
    }
    return _priv;
}

//
// ossl_rsa_key_pair_generator
//

rsa_key_pair_generator& ossl_rsa_key_pair_generator::key_size(size_t key_size) {
    _key_size = key_size;
    return *this;
}

rsa_key_pair_generator& ossl_rsa_key_pair_generator::public_exponent(const cxy::math::big_integer& pub) {
    _pub = pub;
    return *this;
}

size_t ossl_rsa_key_pair_generator::key_size() const {
    return _key_size;
}

cxy::math::big_integer ossl_rsa_key_pair_generator::public_exponent() const {
    return _pub;
}

std::shared_ptr<key_pair> ossl_rsa_key_pair_generator::generate() {
    RSA* rsa = RSA_new();

    // TODO generate multiprime keys

    if(RSA_generate_key_ex(rsa, _key_size, bi2bn(_pub), nullptr)!=0) {
        return std::dynamic_pointer_cast<key_pair>(std::make_shared<ossl_rsa_key_pair>(rsa));
    } else {
        throw std::runtime_error("Error while genrating RSA key");
    }
}


//
// PEM readers
//

//
// OpenSSL system's FILE PEM reader
//
ossl_FILE_pem_reader::ossl_FILE_pem_reader(FILE* f):
_fp(f, std::fclose)
{
}

// TODO fopencookie

ossl_FILE_pem_reader::ossl_FILE_pem_reader(const std::string& path):
ossl_FILE_pem_reader(fopen(path.c_str(), "r"))
{
    if(!_fp) {
        // TODO process errno
        throw std::runtime_error("Cannot fopen filestream");
    }
}

ossl_FILE_pem_reader::ossl_FILE_pem_reader(const void *buf, size_t size):
ossl_FILE_pem_reader(fmemopen(const_cast<void*>(buf), size, "r"))
{
    if(!_fp) {
        // TODO process errno
        throw std::runtime_error("Cannot open memstream");
    }
}

std::shared_ptr<security::public_key> ossl_FILE_pem_reader::public_key()
{
    // TODO support also PEM_read_RSAPublicKey in parallel.
    EVP_PKEY* pkey = PEM_read_PUBKEY(_fp.get(), nullptr, nullptr, nullptr);
    if(pkey!=nullptr) {
        if(EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA) {
            RSA *rsa = EVP_PKEY_get1_RSA(pkey);
            EVP_PKEY_free(pkey);
            return std::shared_ptr<security::public_key>(new ossl_rsa_public_key(rsa));
        }
        // TODO Add other public key types
        EVP_PKEY_free(pkey);
    }
    return nullptr;
}

std::shared_ptr<security::rsa_public_key> ossl_FILE_pem_reader::rsa_public_key()
{
    RSA* rsa = PEM_read_RSAPublicKey(_fp.get(), nullptr, nullptr, nullptr);
    if(rsa!=nullptr) {
        return std::shared_ptr<security::rsa_public_key>(new ossl_rsa_public_key(rsa));
    }
    return nullptr;
}

std::shared_ptr<security::private_key> ossl_FILE_pem_reader::private_key()
{
    EVP_PKEY* pkey = PEM_read_PrivateKey(_fp.get(), nullptr, nullptr, nullptr);
    if(pkey!=nullptr) {
        if(EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA) {
            RSA *rsa = EVP_PKEY_get1_RSA(pkey);
            EVP_PKEY_free(pkey);
            return std::dynamic_pointer_cast<security::private_key>(
                make_rsa_private_key(rsa)
            );
        }
        // TODO Add other private key types
        EVP_PKEY_free(pkey);
    }
    return nullptr;
}

std::shared_ptr<security::private_key> ossl_FILE_pem_reader::private_key(const std::string& passwd)
{
    EVP_PKEY* pkey = PEM_read_PrivateKey(_fp.get(), nullptr, nullptr, const_cast<char*>(passwd.c_str()));
    if(pkey!=nullptr) {
        if(EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA) {
            RSA *rsa = EVP_PKEY_get1_RSA(pkey);
            EVP_PKEY_free(pkey);
            return std::dynamic_pointer_cast<security::private_key>(
                make_rsa_private_key(rsa)
            );
        }
        // TODO Add other private key types
        EVP_PKEY_free(pkey);
    }
    return nullptr;
}


std::shared_ptr<security::rsa_private_key> ossl_FILE_pem_reader::rsa_private_key()
{
    RSA* rsa = PEM_read_RSAPrivateKey(_fp.get(), nullptr, nullptr, nullptr);
    if(rsa!=nullptr) {
        return make_rsa_private_key(rsa);
    }
    return nullptr;
}

std::shared_ptr<security::rsa_private_key> ossl_FILE_pem_reader::rsa_private_key(const std::string& passwd)
{
    RSA* rsa = PEM_read_RSAPrivateKey(_fp.get(), nullptr, nullptr, const_cast<char*>(passwd.c_str()));
    if(rsa!=nullptr) {
        return make_rsa_private_key(rsa);
    }
    return nullptr;
}

//
// ossl_string_pem_reader
//

ossl_string_pem_reader::ossl_string_pem_reader(const std::string& str):
_str(new std::string(str))
{
    _fp = std::shared_ptr<std::FILE>(fmemopen(const_cast<char*>(_str->data()), _str->size(), "r"), std::fclose);
}


//
// PEM writers
//

//
// ossl_FILE_pem_writer
//

ossl_FILE_pem_writer::ossl_FILE_pem_writer(FILE* f):
_fp(f, std::fclose)
{
}

ossl_FILE_pem_writer::ossl_FILE_pem_writer(const std::string& path, bool append):
ossl_FILE_pem_writer(fopen(path.c_str(), append ? "a" : "w"))
{
    if(!_fp) {
        // TODO process errno
        throw std::runtime_error("Cannot fopen filestream");
    }
}

void ossl_FILE_pem_writer::public_key(const security::public_key& key)
{
    std::shared_ptr<EVP_PKEY> pkey { EVP_PKEY_new(), EVP_PKEY_free };

    const security::rsa_public_key* rsakey = dynamic_cast<const security::rsa_public_key*>(&key);
    if(rsakey != nullptr) {
        const ossl_rsa_public_key* osslrsakey = dynamic_cast<const ossl_rsa_public_key*>(rsakey);
        if(osslrsakey==nullptr) {
            // TODO convert rsa privkey to ossl
            throw std::runtime_error("Not implemented yet");
        }
        if(!EVP_PKEY_set1_RSA(pkey.get(), const_cast<RSA*>(osslrsakey->get()))) {
            throw invalid_key_exception("Invalid RSA public key");
        }
    } else {
        // TODO add other public key types
        throw key_exception("unsupported key type");
    }
    
    if(!PEM_write_PUBKEY(_fp.get(), pkey.get())) {
        throw security_exception("Failling writing PKCS8 public key");
    }
}

void ossl_FILE_pem_writer::rsa_public_key(const security::rsa_public_key& key)
{
    const ossl_rsa_public_key* osslrsakey = dynamic_cast<const ossl_rsa_public_key*>(&key);
    if(osslrsakey==nullptr) {
        // TODO convert rsa pubkey to ossl
        throw std::runtime_error("Not implemented yet");
    }
    if(PEM_write_RSAPublicKey(_fp.get(), const_cast<RSA*>(osslrsakey->get())) ==0) {
        throw security_exception("Failling writing PKCS1 RSA public key");
    }
}


void ossl_FILE_pem_writer::private_key(const security::private_key& key)
{
    std::shared_ptr<EVP_PKEY> pkey { EVP_PKEY_new(), EVP_PKEY_free };

    const security::rsa_private_key* rsakey = dynamic_cast<const security::rsa_private_key*>(&key);
    if(rsakey != nullptr) {
        const ossl_rsa_private_key* osslrsakey = dynamic_cast<const ossl_rsa_private_key*>(rsakey);
        if(osslrsakey==nullptr) {
            // TODO convert rsa privkey to ossl
            throw std::runtime_error("Not implemented yet");
        }
        if(!EVP_PKEY_set1_RSA(pkey.get(), const_cast<RSA*>(osslrsakey->get()))) {
            throw invalid_key_exception("Invalid RSA private key");
        }
    } else {
        // TODO add other public key types
        throw key_exception("unsupported key type");
    }
    
    if(!PEM_write_PKCS8PrivateKey(_fp.get(), pkey.get(), nullptr, nullptr, 0, nullptr, 0)) {
        throw security_exception("Failling writing PKCS8 private key");
    }
}

void ossl_FILE_pem_writer::private_key(const security::private_key& key, const security::cipher_builder& cipher, const std::string& passwd)
{
    const EVP_CIPHER* evpcipher = evp_cipher::find_EVP_CIPHER(cipher);
    if(evpcipher==nullptr) {
        throw no_such_algorithm_exception("Invalid algorithm");
        // Shall not occur, an exception must have been thrown.
    }

    std::shared_ptr<EVP_PKEY> pkey { EVP_PKEY_new(), EVP_PKEY_free };
    const security::rsa_private_key* rsakey = dynamic_cast<const security::rsa_private_key*>(&key);
    if(rsakey != nullptr) {
        const ossl_rsa_private_key* osslrsakey = dynamic_cast<const ossl_rsa_private_key*>(rsakey);
        if(osslrsakey==nullptr) {
            // TODO convert rsa privkey to ossl
            throw std::runtime_error("Not implemented yet");
        }
        if(!EVP_PKEY_set1_RSA(pkey.get(), const_cast<RSA*>(osslrsakey->get()))) {
            throw invalid_key_exception("Invalid RSA private key");
        }
    } else {
        // TODO add other public key types
        throw key_exception("Unsupported key type");
    }
    
    if(!PEM_write_PKCS8PrivateKey(_fp.get(), pkey.get(), evpcipher, const_cast<char*>(passwd.data()), passwd.size(), nullptr, 0)) {
        throw security_exception("Failling writing encrypted PKCS8 private key");
    }
}

void ossl_FILE_pem_writer::rsa_private_key(const security::rsa_private_key& rsakey)
{
    std::shared_ptr<EVP_PKEY> pkey { EVP_PKEY_new(), EVP_PKEY_free };

    const ossl_rsa_private_key* osslrsakey = dynamic_cast<const ossl_rsa_private_key*>(&rsakey);
    if(osslrsakey==nullptr) {
        // TODO convert rsa privkey to ossl
        throw std::runtime_error("Not implemented yet");
    }
    
    if(!PEM_write_RSAPrivateKey(_fp.get(), const_cast<RSA*>(osslrsakey->get()), nullptr, nullptr, 0, nullptr, 0)) {
        throw security_exception("Failling writing PKCS1 RSA private key");
    }
}

void ossl_FILE_pem_writer::rsa_private_key(const security::rsa_private_key& rsakey, const security::cipher_builder& cipher, const std::string& passwd)
{
    const EVP_CIPHER* evpcipher = evp_cipher::find_EVP_CIPHER(cipher);
    if(evpcipher==nullptr) {
        throw no_such_algorithm_exception("Invalid algorithm");
        // Shall not occur, an exception must have been thrown.
    }

    const ossl_rsa_private_key* osslrsakey = dynamic_cast<const ossl_rsa_private_key*>(&rsakey);
    if(osslrsakey==nullptr) {
        // TODO convert rsa privkey to ossl
        throw std::runtime_error("Not implemented yet");
    }
    
    if(!PEM_write_RSAPrivateKey(_fp.get(), const_cast<RSA*>(osslrsakey->get()), evpcipher, (unsigned char*) const_cast<char*>(passwd.data()), passwd.size(), nullptr, 0)) {
        throw security_exception("Failling writing encrypted PKCS1 RSA private key");
    }
}


//
// ossl_string_pem_writer
//

ossl_string_pem_writer::ossl_string_pem_writer()
{
    std::FILE* f = open_memstream(&_str, &_sz);
    if(f==nullptr) {
        // TODO process errno
        throw std::runtime_error("Cannot open memstream");
    }
    _fp = std::shared_ptr<std::FILE>(f, std::fclose);
}

ossl_string_pem_writer::~ossl_string_pem_writer()
{
    _fp.reset();
    if(_str!=nullptr) {
        free(_str);
        _str = nullptr;
    }
    _sz = 0;
}

std::string ossl_string_pem_writer::str()const
{
    if(fflush(_fp.get()) != 0) {
        // TODO process errno
        throw std::runtime_error("Cannot fflush memstream");
    }
    return std::string(_str, _str+_sz);
}

} // namespace cxy::security::openssl
