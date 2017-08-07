class HUID;

#ifndef HUID_HPP
#define HUID_HPP

#include <string.h>
#include <string>

using namespace std;

#include <ipnoise-common/strings.h>
#include <ipnoise-common/log_common.h>

class HUID
{
    public:
        HUID(const string &a_huid = "");
        HUID(
            const string    &a_login,
            const string    &a_password
        );
        virtual ~HUID();

        enum Places {
            PLACE_ALL       = 0,
            PLACE_HOME_1,
            PLACE_HOME_2,
            PLACE_MOBILE_1,
            PLACE_MOBILE_2,
            PLACE_WORK_1,
            PLACE_WORK_2,
            PLACE_OTHER
        };

        uint32_t    getLocalId();
        int32_t     setHuid(const string &a_huid);
        bool        isLocal();
        void        setLocal(bool);
        void        resetPlace();
        void        setPlace(Places);
        Places      getPlace();
        string      toString();

    private:
        unsigned short m_huid[8];
};

#endif

