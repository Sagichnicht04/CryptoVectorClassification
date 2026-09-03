#include "aes/aes-gcm.hpp"
#include "aes/internal/iv-formatting.hpp"
#include "logger.hpp"

#include <openssl/evp.h>

namespace dh {

int aesErr(std::string const& msg, Logger& log) {
  log.err(msg);
  return -1;
}

int aesEncrypt(unsigned char  const* const  plaintext,
               unsigned char*               ciphertext,
               unsigned char*               tag,
               size_t                const  data_len,
               AESParams             const& aes_v) {
  Logger& log = Logger::get();
  EVP_CIPHER_CTX* ctx;
  int len;
  int ciphertext_len = 0;

  //init contex
  if (!(ctx = EVP_CIPHER_CTX_new())) 
    aesErr("Couldn't init context.", log); 

  //init ecryption operation
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
    aesErr("Couldn't init AES-GCM.", log);

  //set iv length (96-bits)
  if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL))
    aesErr("Couldn't set IV length for AES.", log);

  //init key & iv
  unsigned char iv[12] = {0};
  if (EXIT_SUCCESS != formIV(iv, aes_v.aes_iv))
    aesErr("Couldn't format IV for encryption.", log);

  if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, aes_v.aes_key, iv))
    aesErr("Couldn't set key and IV for AES.", log);

  //encrypt plaintext to ciphertext
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, data_len))
    aesErr("Couldn't encrypt plaintext.", log);
  ciphertext_len += len;

  //finalize
  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext+len, &len))
    aesErr("Couldn't finalize encryption", log);

  //get tag 
  if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
    aesErr("Could not retrieve encryption tag.", log);

  //clean up
  EVP_CIPHER_CTX_free(ctx);
  return ciphertext_len;
}

int aesDecrypt(unsigned char*        plaintext,
               unsigned char* const  ciphertext,
               unsigned char* const  tag,
               size_t         const  data_len,
               AESParams      const& aes_v) {
  Logger& log = Logger::get();
  EVP_CIPHER_CTX* ctx;
  int len;
  int plaintext_len;
  int ret;

  if (!(ctx = EVP_CIPHER_CTX_new()))
    aesErr("Couldn't init context.", log);

  //init decryption operation
  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
    aesErr("Couldn't init AES-GCM decryption.", log);

  //set iv length (96-bits)
  if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL) != 1)
    aesErr("Couldn't set IV length for AES.", log);

  //init key & iv
  unsigned char iv[12] = {0};
  if (EXIT_SUCCESS != formIV(iv, aes_v.aes_iv))
    aesErr("Couldn't format IV for encryption.", log);

  if (EVP_DecryptInit_ex(ctx, NULL, NULL, aes_v.aes_key, iv) != 1)
    aesErr("Couldn't set key and IV for AES.", log);

  //decrypt ciphertext to plaintext
  if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, data_len) != 1)
    aesErr("Couldn't decrypt ciphertext.", log);

  plaintext_len = len;

  if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
    aesErr("Couldn't set tag.", log);

  //finalize
  ret = EVP_DecryptFinal_ex(ctx, plaintext+len, &len);

  //clean up
  EVP_CIPHER_CTX_free(ctx);

  if (ret > 0) {
    plaintext_len += len;
    return plaintext_len;
  } else {
    return -1;
  } 
}


} //dh
