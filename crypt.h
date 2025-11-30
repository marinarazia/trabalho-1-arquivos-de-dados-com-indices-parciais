#ifndef CRYPT_H
#define CRYPT_H

#include <stddef.h>

char* to_base64(const unsigned char* data, size_t len);

unsigned char* from_base64(const char* b64, size_t* out_len);

char* encrypt_string(const char* plaintext);

char* decrypt_string(const char* b64);

#endif
