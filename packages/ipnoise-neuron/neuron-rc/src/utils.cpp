#include <fcntl.h>
#include <math.h>
#include <ipnoise-common/log_common.h>

#include "utils.hpp"

double Utils::rand(
    const double &a_min,
    const double &a_max)
{
    double res = (double)::rand() / RAND_MAX;
    double ret = a_min + res * (a_max - a_min);

    return ret;
}

void Utils::init_timeval(
    struct timeval  &a_out,
    const  double   &a_val)
{
    double          part_integer = 0.0f;
    double          part_fractal = 0.0f;

    part_fractal = modf(a_val, &part_integer);

    a_out.tv_sec  = part_integer;
    a_out.tv_usec = 1e6*part_fractal;
}

string Utils::dump(
    const char      *a_data,
    const uint64_t  &a_size)
{
    char        tmp[16] = { 0x00 };
    string      ret;
    uint32_t    i;
    string      chars;
    uint32_t    cols = 16;
    uint32_t    col  = 0;
    uint32_t    max  = cols * (a_size/cols + 1);
    for (i = 0; i < max; i++){
        uint8_t c = 0;

        if (!col){
            snprintf(tmp, sizeof(tmp),
                "0x%4.4x ",
                uint32_t(i)
            );
            ret += tmp;
        }

        if (i < a_size){
            c = a_data[i];
            snprintf(tmp, sizeof(tmp),
                " %2.2x",
                c
            );
            ret += tmp;

            if (    32 <= c
                &&  127 > c)
            {
                chars += c;
            } else {
                chars += ".";
            }
        } else {
            // end of data, just align
            ret   += "   ";
            chars += " ";
        }

        if (col == (cols/2 - 1)){
            // half of data
            ret   += "  ";
            chars += " ";
        }

        if (col == (cols - 1)){
            // end of line
            ret   += "    " + chars + "\n";
            chars = "";
            col   = 0;
            continue;
        }

        col++;
    }

    return ret;
}

string Utils::dump(
    const string &a_data)
{
    return Utils::dump(a_data.c_str(), a_data.size());
}

