#include <sodium.h>

// algoritmo usado: XSalsa20-Poly1305 (criptografia simétrica autenticada)

// Codifica Base64
char* to_base64(const unsigned char* data, size_t len)
{
    size_t out_len = sodium_base64_ENCODED_LEN(len, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
    char* out = malloc(out_len);
    sodium_bin2base64(out, out_len, data, len, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
    return out;
}

// Decodifica Base64
unsigned char* from_base64(const char* b64, size_t* out_len)
{
    size_t len = strlen(b64);
    unsigned char* out = malloc(len);
    sodium_base642bin(out, len, b64, len, NULL, out_len, NULL, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
    return out;
}

// --------------------------------
//  FUNÇÃO DE CRIPTOGRAFIA
// --------------------------------
char* encrypt_string(const char* plaintext)
{
    size_t msg_len = strlen(plaintext);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof nonce);

    size_t cipher_len = msg_len + crypto_secretbox_MACBYTES;
    unsigned char* cipher = malloc(cipher_len);

    crypto_secretbox_easy(
        cipher,
        (const unsigned char*)plaintext,
        msg_len,
        nonce,
        SECRET_KEY
    );

    size_t total_len = crypto_secretbox_NONCEBYTES + cipher_len;
    unsigned char* output = malloc(total_len);

    // output = nonce | ciphertext
    memcpy(output, nonce, crypto_secretbox_NONCEBYTES);
    memcpy(output + crypto_secretbox_NONCEBYTES, cipher, cipher_len);

    free(cipher);

    // retorna como Base64
    char* b64 = to_base64(output, total_len);
    free(output);

    return b64;
}

// --------------------------------
//  FUNÇÃO DE DESCRIPTOGRAFIA
// --------------------------------
char* decrypt_string(const char* b64)
{
    size_t bin_len;
    unsigned char* bin = from_base64(b64, &bin_len);

    if (bin_len < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
        free(bin);
        return NULL;
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    memcpy(nonce, bin, crypto_secretbox_NONCEBYTES);

    unsigned char* cipher = bin + crypto_secretbox_NONCEBYTES;
    size_t cipher_len = bin_len - crypto_secretbox_NONCEBYTES;

    size_t msg_len = cipher_len - crypto_secretbox_MACBYTES;
    unsigned char* msg = malloc(msg_len + 1);
    msg[msg_len] = '\0';

    if (crypto_secretbox_open_easy(msg, cipher, cipher_len, nonce, SECRET_KEY) != 0)
    {
        free(bin);
        free(msg);
        return NULL;
    }

    free(bin);
    return (char*)msg;
}
