#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>

using namespace std;

string base64_encode(unsigned char const *, unsigned int len);
string base64_decode(string const &);
string base64_encode(const string &);

#endif

