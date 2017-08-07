#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include <ipnoise-common/log_common.h>

#include "base64.h"

static const char base64[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * Base64-encode data
 *
 * @v raw		Raw data
 * @v len		Length of raw data
 * @v encoded		Buffer for encoded string
 *
 * The buffer must be the correct length for the encoded string.  Use
 * something like
 *
 *     char buf[ base64_encoded_len ( len ) + 1 ];
 *
 * (the +1 is for the terminating NUL) to provide a buffer of the
 * correct size.
 */
void base64_encode(
    const uint8_t   *raw,
    int64_t          len,
    char            *encoded)
{
    const uint8_t   *raw_bytes      = ((const uint8_t *)raw);
    uint8_t         *encoded_bytes  = ((uint8_t *)encoded);
    int64_t          raw_bit_len     = (8 * len);
    unsigned int    bit;
    unsigned int    byte;
    unsigned int    shift;
    unsigned int    tmp;

    for (bit = 0 ; bit < raw_bit_len ; bit += 6){
        byte    = (bit / 8);
        shift   = (bit % 8);
        tmp     = (raw_bytes[byte] << shift);
        if ((byte + 1) < len){
            tmp |= (raw_bytes[byte + 1] >> (8 - shift));
        }
        tmp = ((tmp >> 2) & 0x3f);
        *(encoded_bytes++) = base64[tmp];
    }
    for (; (bit % 8) != 0; bit += 6){
        *(encoded_bytes++) = '=';
    }
    *(encoded_bytes++) = '\0';

    PDEBUG(100, "Base64-encoded to \"%s\":\n", encoded);
    if (strlen(encoded) != base64_encoded_len(len)){
#ifdef __x86_64__
        PFATAL("strlen(encoded) != base64_encoded_len(len)\n"
            "strlen(encoded):           '%ld'\n"
            "base64_encoded_len(len):   '%ld'\n",
            strlen(encoded),
            base64_encoded_len(len)
        );
#else
        PFATAL("strlen(encoded) != base64_encoded_len(len)\n"
            "strlen(encoded):           '%d'\n"
            "base64_encoded_len(len):   '%lld'\n",
            strlen(encoded),
            base64_encoded_len(len)
        );
#endif
    }
}

/**
 * Base64-decode string
 *
 * @v encoded		Encoded string
 * @v raw		Raw data
 * @ret len		Length of raw data, or negative error
 *
 * The buffer must be large enough to contain the decoded data.  Use
 * something like
 *
 *     char buf[ base64_decoded_max_len ( encoded ) ];
 *
 * to provide a buffer of the correct size.
 */
int base64_decode(
    const char  *encoded,
    uint8_t     *raw)
{
    const uint8_t  *encoded_bytes = ((const uint8_t *)encoded);
    uint8_t        *raw_bytes     = ((uint8_t *)raw);
    uint8_t        encoded_byte;
    char           *match;
    int            decoded;
    unsigned int   bit = 0;
    unsigned int   pad_count = 0;
    int64_t        len;

    // Zero the raw data
    memset(raw, 0x00, base64_decoded_max_len(encoded));

    // Decode string
    while ((encoded_byte = *(encoded_bytes++))){
        // Ignore whitespace characters
        if (isspace(encoded_byte)){
            continue;
        }

        // process pad characters
        if (encoded_byte == '='){
            if (pad_count >= 2){
                PERROR("Base64-encoded string \"%s\""
                    " has too many pad characters\n",
                    encoded
                );
                len = -EINVAL;
                goto fail;
            }
            pad_count++;
            bit -= 2; // unused_bits = ( 2 * pad_count )
            continue;
        }
        if (pad_count){
            PERROR("Base64-encoded string \"%s\""
                " has invalid pad "
                "sequence\n",
                encoded
            );
            len = -EINVAL;
            goto fail;
        }

        // process normal characters
        match = strchr(base64, encoded_byte);
        if (!match){
            PERROR("Base64-encoded string \"%s\""
                " contains invalid "
                "character '%c'\n",
                encoded,
                encoded_byte
            );
            len = -EINVAL;
            goto fail;
        }
        decoded = match - base64;

        // add to raw data
        decoded <<= 2;
        raw_bytes[bit / 8]     |= (decoded >> (bit % 8));
        raw_bytes[bit / 8 + 1] |= (decoded << (8 - (bit % 8)));
        bit += 6;
    }

    // check that we decoded a whole number of bytes
    if ((bit % 8) != 0){
        PERROR("Base64-encoded string \"%s\""
            " has invalid bit length "
            "%d\n",
            encoded,
            bit
        );
        len = -EINVAL;
        goto fail;
    }

    len = bit/8;

    PDEBUG(100, "Base64-decoded \"%s\" to:\n", encoded);
    if (len < base64_decoded_max_len(encoded)){
#ifdef __x86_64__
        PFATAL("len < base64_decoded_max_len(encoded)\n"
            "len:                             '%ld'\n"
            "base64_decoded_max_len(encoded): '%ld'\n",
            len,
            base64_decoded_max_len(encoded)
        );
#else
        PFATAL("len < base64_decoded_max_len(encoded)\n"
            "len:                             '%lld'\n"
            "base64_decoded_max_len(encoded): '%lld'\n",
            len,
            base64_decoded_max_len(encoded)
        );
#endif
    }

out:
    return len;
fail:
    goto out;
}

