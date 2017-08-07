#ifndef IPNOISE_BASE64_H
#define IPNOISE_BASE64_H

void _base64_encode_triple(unsigned char triple[3], char result[4]);
int base64_encode(unsigned char *source, int sourcelen, char *target, int targetlen);
int _base64_char_value(char base64char);
int _base64_decode_triple(char quadruple[4], unsigned char *result);
int base64_decode(char *source, unsigned char *target, int targetlen);

#endif

