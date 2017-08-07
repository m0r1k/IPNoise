#ifndef KLADR_HPP
#define KLADR_HPP

#define KLADR_PATH "static/kladr.db"

class KladrReqInfo
{
    public:
        KladrReqInfo(){
            m_split_by_comma = 0;
        }
        ObjectVectorSptr    m_results;
        int32_t             m_split_by_comma;
};

typedef int32_t (*KladrCb)(
    void    *a_kladr_req_info,
    int32_t a_argc,
    char    **a_argv,
    char    **a_col_name
);

#endif

