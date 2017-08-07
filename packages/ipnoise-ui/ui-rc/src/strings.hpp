#ifndef STRING_HPP
#define STRING_HPP

#include <string>
#include <vector>

using namespace std;

/*
 *  Splitter
 *
 *  src     - source string
 *  delim   - delimeter (only one delimeter, use Splitter2 for multiple)
 *
 */
class Splitter
{
    public:
        typedef vector<string>::size_type size_type;

        Splitter (){};

        Splitter (const string &src, const string &delim){
            reset (src, delim);
        }

        string & operator[] (size_type i){
            return _tokens.at (i);
        }

        size_type size() const {
            return _tokens.size();
        }

        bool find(const string &val){
            // check what value exist
            bool ret = false;
            for (size_type i = 0; i < _tokens.size(); i++){
                if (_tokens.at(i) == val){
                    ret = true;
                    break;
                }
            }
            return ret;
        }

        void reset (const string &src, const string &delim){
            vector<string> tokens;
            string::size_type start = 0;
            string::size_type end;

            for (;;){
                end = src.find(delim, start);
                tokens.push_back(src.substr(start, end - start));

                if (string::npos == end){
                    break;
                }

                start = end + delim.size();
            }

            _tokens.swap (tokens);
        }

    private:
        vector<string> _tokens;
};

/*
 *  Splitter2
 *
 *  src     - source string
 *  delims  - multiple delimeters (1 symbol = 1 delimeter)
 *
 */
class Splitter2
{
    public:
        typedef vector<string>::size_type size_type;
        Splitter2 (){};
        Splitter2 (const string &src, const string &delims){
            reset (src, delims);
        }

        string & operator[] (size_type i){
            return _tokens.at (i);
        }

        size_type size() const {
            return _tokens.size();
        }

        bool find(const string &val){
            // check what value exist
            bool ret = false;
            for (size_type i = 0; i < _tokens.size(); i++){
                if (_tokens.at(i) == val){
                    ret = true;
                    break;
                }
            }
            return ret;
        }

        void find_delims(
            const string         &src,
            const string         &delims,
            string::size_type    &search_from,
            string::size_type    &delims_start,
            string::size_type    &delims_end)
        {
            string::size_type i, m;

            delims_start    = string::npos;
            delims_end      = string::npos;

            // search first delimeter
            for (i = 0; i < delims.size(); i++){
                char            cur_delim;
                string::size_type    pos;

                cur_delim = delims.at(i);
                pos = src.find(cur_delim, search_from);
                if (string::npos != pos){
                    if (    string::npos == delims_start
                        ||  pos < delims_start)
                    {
                        delims_start = pos;
                        if (string::npos == delims_end){
                            delims_end = delims_start;
                        }
                    }
                }
            }

            // search last delimeter
            if (string::npos != delims_start){
                for (i = delims_start + 1; i < src.size(); i++){
                    bool delim_found = false;
                    for (m = 0; m < delims.size(); m++){
                        const char cur_delim = delims.at(m);
                        if (src.at(i) == cur_delim){
                            // it is delim
                            delim_found = true;
                            break;
                        }
                    }
                    if (delim_found){
                        delims_end = i;
                    } else {
                        // no more delims
                        break;
                    }
                }
            }
        }

        void reset (const string &src, const string &delims){
            vector<string> tokens;
            string::size_type pos = 0;

            for (;;){
                string::size_type delims_start  = string::npos;
                string::size_type delims_end    = string::npos;

                find_delims(
                    src,
                    delims,
                    pos,
                    delims_start,
                    delims_end
                );

                if (string::npos == delims_start){
                    tokens.push_back(src.substr(pos));
                    break;
                }

                tokens.push_back(src.substr(pos, delims_start - pos));
                pos = delims_end + 1;
            }

            _tokens.swap(tokens);
        }

    private:
        vector<string> _tokens;
};

#endif

