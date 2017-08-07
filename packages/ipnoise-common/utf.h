#ifndef UTF_HPP
#define UTF_HPP

#include <string>
#include <ipnoise-common/log_common.h>

using namespace std;

static wstring Utf8ToUtf16(const string &a_utf8)
{
    vector<unsigned long> unicode;
    size_t  i = 0;
    wstring utf16;

    while (i < a_utf8.size()){
        unsigned long   uni     = 0;
        size_t          todo    = 0;
        unsigned char   ch      = a_utf8[i++];

        if (ch <= 0x7F){
            uni     = ch;
            todo    = 0;
        } else if (ch <= 0xBF){
            goto notutf8;
        } else if (ch <= 0xDF){
            uni     = ch&0x1F;
            todo    = 1;
        } else if (ch <= 0xEF){
            uni     = ch&0x0F;
            todo    = 2;
        } else if (ch <= 0xF7){
            uni     = ch&0x07;
            todo    = 3;
        } else {
            goto notutf8;
        }

        for (size_t j = 0; j < todo; ++j){
            if (i == a_utf8.size()){
                goto notutf8;
            }
            unsigned char ch = a_utf8[i++];
            if (ch < 0x80 || ch > 0xBF){
                goto notutf8;
            }
            uni <<= 6;
            uni += ch & 0x3F;
        }
        if (    uni >= 0xD800
            &&  uni <= 0xDFFF)
        {
            goto notutf8;
        }
        if (uni > 0x10FFFF){
            goto notutf8;
        }
        unicode.push_back(uni);
    }
    for (size_t i = 0; i < unicode.size(); ++i){
        unsigned long uni = unicode[i];
        if (uni <= 0xFFFF){
            utf16 += (wchar_t)uni;
        } else {
            uni -= 0x10000;
            utf16 += (wchar_t)((uni >> 10) + 0xD800);
            utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
        }
    }
out:
    return utf16;

notutf8:
    PDEBUG(5, "not utf8 string\n");
    goto out;
}

static wstring NativeToUtf16(const string &sNative)
{
    locale locNative;

    // The UTF-16 will never be longer
    // than the input string
    vector<wchar_t> vUtf16(1+sNative.length());

    // convert
    use_facet< ctype<wchar_t> >(locNative).widen(
        sNative.c_str(),
        sNative.c_str()+sNative.length(),
        &vUtf16[0]
    );

    return wstring(vUtf16.begin(), vUtf16.end());
}

static string Utf16ToUtf8(wchar_t chUtf16)
{
    // From RFC 3629
    // 0000 0000-0000 007F   0xxxxxxx
    // 0000 0080-0000 07FF   110xxxxx 10xxxxxx
    // 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx

    // max output length is 3 bytes (plus one for Nul)
    unsigned char szUtf8[4] = "";

    if (chUtf16 < 0x80){
        szUtf8[0] = static_cast<unsigned char>(chUtf16);
    } else if (chUtf16 < 0x7FF){
        szUtf8[0] = static_cast<unsigned char>(
            0xC0 | ((chUtf16>>6)&0x1F)
        );
        szUtf8[1] = static_cast<unsigned char>(
            0x80 | (chUtf16&0x3F)
        );
    } else {
        szUtf8[0] = static_cast<unsigned char>(
            0xE0 | ((chUtf16>>12)&0xF)
        );
        szUtf8[1] = static_cast<unsigned char>(
            0x80 | ((chUtf16>>6)&0x3F)
        );
        szUtf8[2] = static_cast<unsigned char>(
            0x80 | (chUtf16&0x3F)
        );
    }

    return reinterpret_cast<char *>(szUtf8);
}

static string Utf16ToUtf8(const wstring &sUtf16)
{
    string sUtf8;
    wstring::const_iterator itr;

    for (itr=sUtf16.begin(); itr!=sUtf16.end(); ++itr){
        sUtf8 += Utf16ToUtf8(*itr);
    }
    return sUtf8;
}

#endif

