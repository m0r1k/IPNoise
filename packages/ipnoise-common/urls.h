#ifndef URLS_H
#define URLS_H

namespace utils
{

/*  parse_url - Simple url parser
 *
 *  @param[in]  url          Url for parsing
 *  @param[out] &url_info    Hash for results,
 *                           used keys: "protocol", "domain", "port", "path"
 *                           (you may fill default values)
 *
 *  @return 0 if success
 */
inline int parse_url (string url, map<string, string> &url_info)
{
    int i, err = -1;

    string::size_type pos;

    // init values
    int clear_protocol  = 1;
    int clear_domain    = 1;
    int clear_port      = 1;
    int clear_path      = 1;

    enum States {
        PROTO_START = 0,    // collect protocol,
                            // setup SEARCH_DOMAIN1 if symbol ':' found
        SEARCH_DOMAIN1,     // setup SEARCH_DOMAIN2 if symbol '/' found
        SEARCH_DOMAIN2,     // setup DOMAIN_START   if symbol '/' found
        DOMAIN_START,
        DOMAIN_START_IPV6,  // IPV6 address if symbol '[' found
        PORT_START,
        PATH_START
    } state = PROTO_START;

    // check what protocol exist
    pos = url.find("://");
    if (pos == string::npos){
        // looks like protocol not exist, skip it
        state = DOMAIN_START;
    }

    for (i = 0; i < (int)url.size(); i++){
        char symbol = url.at(i);

        switch (state){
            case (PROTO_START):
                if (symbol == ':'){
                    state = SEARCH_DOMAIN1;
                } else {
                    if (clear_protocol){
                        // clear default value
                        url_info["protocol"] = "";
                        clear_protocol = 0;
                    }
                    url_info["protocol"] += symbol;
                }
                break;

            case (SEARCH_DOMAIN1):
                if (symbol == '/'){
                    state = SEARCH_DOMAIN2;
                }
                break;

            case (SEARCH_DOMAIN2):
                if (symbol == '/'){
                    state = DOMAIN_START;
                }
                break;

            case (DOMAIN_START_IPV6):
               if (symbol == ']'){
                    state = DOMAIN_START;
                    break;
                }
                if (clear_domain){
                    // clear default value
                    url_info["domain"] = "";
                    clear_domain = 0;
                }
                url_info["domain"] += symbol;
                break;

            case (DOMAIN_START):
                if (symbol == '['){
                    state = DOMAIN_START_IPV6;
                    break;
                }
                if (symbol == ':'){
                    state = PORT_START;
                } else if (symbol == '/'){
                    state = PATH_START;
                } else {
                    if (clear_domain){
                        // clear default value
                        url_info["domain"] = "";
                        clear_domain = 0;
                    }
                    url_info["domain"] += symbol;
                }
                break;

            case (PORT_START):
                if (symbol == '/'){
                    state = PATH_START;
                } else {
                    if (clear_port){
                        // clear default value
                        url_info["port"] = "";
                        clear_port = 0;
                    }
                    url_info["port"] += symbol;
                }
                break;

            case (PATH_START):
                if (clear_path){
                    // clear default value
                    url_info["path"] = "/";
                    clear_path = 0;
                }
                url_info["path"] += symbol;
                break;

            default:
                break;
        };
    }

    // all ok
    err = 0;

    return err;
}

}; // namespace utils

#endif

