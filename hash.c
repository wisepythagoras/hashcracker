#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include "hash.h"

/**
 * Convert the hash to hex and put it in the buffer.
 *
 * @param hash The hash to convert to hex.
 * @param output_buffer The output buffer.
 * @param length The length of the output buffer.
 */
void hash_to_hex(unsigned char *hash, char *output_buffer, int length)
{
    int i = 0;

    for(i = 0; i < length; i++) {
        sprintf(output_buffer + (i * 2), "%02x", hash[i]);
    }

    output_buffer[length * 2] = 0;
}

/**
 * Create an SHA512 hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void sha512(char *string, char output_buffer[128])
{
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, string, strlen(string));
    SHA512_Final(hash, &sha512);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, SHA512_DIGEST_LENGTH);
}

/**
 * Create an SHA256 hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void sha256(char *string, char output_buffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, SHA256_DIGEST_LENGTH);
}

/**
 * Create an SHA hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void sha1(char *string, char output_buffer[40])
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha;
    SHA1_Init(&sha);
    SHA1_Update(&sha, string, strlen(string));
    SHA1_Final(hash, &sha);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, SHA_DIGEST_LENGTH);
}

/**
 * Create an MD5 hash.
 *
 * @param string The string to hash.
 * @param buffer The output buffer.
 */
void md5(char *string, char output_buffer[33])
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, string, strlen(string));
    MD5_Final(hash, &md5);

    // Produce the hex 
    hash_to_hex(hash, output_buffer, MD5_DIGEST_LENGTH);
}
