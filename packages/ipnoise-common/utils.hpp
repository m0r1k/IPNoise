#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

using namespace std;

#include <ipnoise-common/log.h>

#define MAX_IPNOISE_ELEMENT_SIZE 1024*1024*10 // 10Mb

inline string cutIPNoisePackets(
    string          &a_buffer,
    vector<string>  &a_out)
{
    int     i;
    int     clear = 1;
    string  xml;

    if (a_buffer.size() > MAX_IPNOISE_ELEMENT_SIZE){
        PERROR(
            "buffer size exceed limit: '%d'"
            " and will be cleared\n",
            MAX_IPNOISE_ELEMENT_SIZE
        );
        goto out;
    }

    // remove <?xml ?> TODO store attributes (encoding etc)
    do {
        string::size_type pos_start = string::npos;
        string::size_type pos_end   = string::npos;

        pos_start = a_buffer.find("<?xml");
        if (string::npos == pos_start){
            break;
        }

        pos_end = a_buffer.find("?>", pos_start);
        if (string::npos == pos_end){
            break;
        }
        pos_end += strlen("?>");

        // get xml
        xml = a_buffer.substr(
            pos_start,
            pos_end - pos_start
        );
        if (xml.empty()){
            break;
        }

        // remove from buffer
        a_buffer.replace(pos_start, pos_end - pos_start, "");

    } while (a_buffer.size());

    do {
        string cmd;
        string::size_type   pos_start = string::npos;
        string::size_type   pos_end   = string::npos;

        pos_start = a_buffer.find("<ipnoise");
        if (string::npos == pos_start){
            break;
        }

        // search end via next packet
        pos_end = a_buffer.find("<ipnoise", pos_start + 1);

        // search end via '</ipnoise>'
        if (string::npos == pos_end){
            pos_end = a_buffer.find("</ipnoise>", pos_start + 1);
            if (string::npos != pos_end){
                pos_end += sizeof("</ipnoise>");
            }
        }

        // search end via '/>'
        if (string::npos == pos_end){
            for (i = pos_start + 1; i < int(a_buffer.size()); i++){
                char c = a_buffer.at(i);
                if ('<' == c){
                    // data was read not fully, wait next data
                    break;
                } else if ('/' == c
                    && ((i + 1) < int(a_buffer.size()))
                    && '>' == a_buffer.at(i + 1))
                {
                    // we have found '/>'
                    pos_end = i + sizeof("/>");
                    break;
                }
            }
        }

        if (string::npos == pos_end){
            break;
        }

        // get commands
        cmd = a_buffer.substr(pos_start, pos_end - pos_start);
        if (cmd.empty()){
            break;
        }
        a_out.push_back(cmd);

        // remove from buffer
        a_buffer.replace(pos_start, pos_end - pos_start, "");
    } while (a_buffer.size());

    // clear if only spaces
    for (i = 0; i < int(a_buffer.size()); i++){
        char c = a_buffer.at(i);
        if (    '\n' != c
            &&  ' '  != c
            &&  '\r' != c)
        {
            clear = 0;
            break;
        }
    }

out:
    if (clear){
        a_buffer.clear();
    }
    return xml;
}

#endif

