#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>
#include <string.h>

/**
 * Calculate length of base64-encoded data
 *
 * @v raw_len		Raw data length
 * @ret encoded_len	Encoded string length (excluding NUL)
 */
static inline int64_t base64_encoded_len(
    int64_t raw_len)
{
	return (((raw_len + 3 - 1)/3) * 4);
}

/**
 * Calculate maximum length of base64-decoded string
 *
 * @v encoded		Encoded string
 * @v max_raw_len	Maximum length of raw data
 *
 * Note that the exact length of the raw data cannot be known until
 * the string is decoded.
 */
static inline int64_t base64_decoded_max_len(
    const char *encoded)
{
	return (((strlen(encoded) + 4 - 1)/4) * 3);
}

void base64_encode(
    const uint8_t   *raw,
    int64_t          len,
    char            *encoded
);

int base64_decode(
    const char  *encoded,
    uint8_t     *raw
);

#endif

