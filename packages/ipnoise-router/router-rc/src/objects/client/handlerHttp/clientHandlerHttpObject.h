/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Mar 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <algorithm>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Request;
class Answer;
class ClientHandlerHttpObject;

typedef map<string, string> HEADERS;

#ifndef CLIENT_HANDLER_HTTP_OBJECT_H
#define CLIENT_HANDLER_HTTP_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

// local
#include <ipnoise-common/log.h>
#include "objects/client/clientHandler.h"

class DomDocument;

#include <ipnoise-common/strings.h>

class Request
{
    public:
        Request(){
            do_init();
        };
        ~Request(){};

        enum State {
            STATE_PARSE_REQ = 0,
            STATE_PARSE_HEADERS,
            STATE_PARSE_CONTENT
        };

        void do_init(){
            req         = "";
            method      = "";
            url         = "";
            protocol    = "";
            content     = "";
            state       = STATE_PARSE_REQ;
            headers.clear();
        }

        void parse_first_line(string &line)
        {
            int i;

            // store request
            req = line;

            // parse
            Splitter parts(line.c_str(), " ");
            for (i = 0; i < (int)parts.size(); i++){
                switch (i){
                    case (0):
                        method = parts[i];
                        transform(method.begin(), method.end(), method.begin(), ::toupper);
                        break;
                    case (1):
                        url = parts[i];
                        break;
                    case (2):
                        protocol = parts[i];
                        transform(protocol.begin(), protocol.end(), protocol.begin(), ::toupper);
                        break;
                };
            }
        }

        void parse_header_line(string &line)
        {
            string key              = "";
            string val              = "";
            string::size_type pos   = string::npos;

            if (!line.size()){
                goto out;
            }

            pos = line.find(":");
            if (pos != string::npos){
                key = line.substr(0, pos);
                transform(key.begin(), key.end(), key.begin(), ::tolower);
                if ((pos + 1) < line.size()){
                    val = line.substr(pos + 1);
                }
                while (val.at(0) == ' ' && val.size() > 1){
                    val = val.substr(1);
                }
                headers[key] = val;
            }

        out:
            return;
        }

        string dump(const char *descr = NULL){
            HEADERS::iterator headers_i;
            string ret = "";
            ret += "----------- Request --------- {\n";
            if (descr){
                ret += descr;
            }
            ret += "Request:    '" + req         + "'\n";
            ret += "Method:     '" + method      + "'\n";
            ret += "URL:        '" + url         + "'\n";
            ret += "Protocol:   '" + protocol    + "'\n";
            ret += "Headers:\n";
            for (headers_i = headers.begin();
                headers_i != headers.end();
                headers_i++)
            {
                ret += headers_i->first;
                ret += ": ";
                ret += headers_i->second;
                ret += "\n";
            }
            ret += "Content:\n" + content;
            ret += "----------- Request --------- }\n";
            return ret;
        }

        string      req;
        string      method;
        string      url;
        string      protocol;
        string      content;
        HEADERS     headers;
        State       state;
};

class Answer
{
    public:
        Answer(){
            do_init();
        };
        ~Answer(){};

        void do_init(){
            need_close      = 1;

            // answer data
            resp_code       = 500;
            resp_line       = "Internal Error";
            resp_proto      = "HTTP/1.1";
            content_type    = "";
            content_length  = -1;
            content         = "";
        }

        void needClose(int val){
            need_close = val;
        }

        int needClose(){
            return need_close;
        }

        string getData(){
            char buff[1024];
            HEADERS::iterator headers_i;
            string data = "";

            snprintf(buff, sizeof(buff), "%s %d %s\n",
                resp_proto.c_str(),
                resp_code,
                resp_line.c_str()
            );

            // add status line
            data += buff;

            // add headers
            for (headers_i = headers.begin();
                headers_i != headers.end();
                headers_i++)
            {
                data += headers_i->first;
                data += ": ";
                data += headers_i->second;
                data += "\n";
            }

            if (content.size()){
                // add content type and length
                if (content_length < 0){
                    content_length = content.size();
                }
                snprintf(buff, sizeof(buff),
                    "Content-type: %s\n"
                    "Content-length: %d\n",
                    content_type.c_str(),
                    content_length
                );
                data += buff;

                // add content
                data += "\n";
                data += content;
            }

            return data;
        }

        string dump(const char *descr = NULL){
            string ret = "";
            ret += "----------- Answer --------- {\n";
            if (descr){
                ret += descr;
            }
            ret += getData();
            ret += "----------- Answer --------- }\n";
            return ret;
        }

        int         resp_code;
        string      resp_line;
        string      resp_proto;
        string      content;
        string      content_type;
        int         content_length;
        HEADERS     headers;


    private:
        int need_close;
};

class ClientHandlerHttpObject
    :   public ClientHandler
{
    public:
        ClientHandlerHttpObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_http"
        );
        ~ClientHandlerHttpObject();

        // internal
        virtual ClientHandlerHttpObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_http"
        );

        // interface
        virtual     int     do_init();
        virtual     void    closed_cb();
        virtual     int     read_cb(
            const string    &a_data,
            ReadState       rd_state
        );

        string dump(const char *descr = NULL);

        Answer answer;
        int process_answer();

    protected:
        void parse_first_line(string &line);
        void parse_header_line(string &line);
        void process_req();

    private:
        Request request;

        // signal|splots implementation
        virtual void registrySignals();
        virtual void registrySlots();
        virtual void slot(
            const string    &signal_name,
            DomElement      *object
        );
};

#endif

