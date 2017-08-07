#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdint.h>

#include <QThread>
#include <string>

#include "context.hpp"
#include "thread.hpp"

using namespace std;

#define CHECKED_CONNECT(source, signal, receiver, slot)     \
    if (!connect(source, signal, receiver, slot)){          \
        qt_assert_x(                                        \
            Q_FUNC_INFO,                                    \
            "CHECKED_CONNECT failed",                       \
            __FILE__,                                       \
            __LINE__                                        \
        );                                                  \
    }

#define CHECKED_DISCONNECT(source, signal, receiver, slot)  \
    if (!disconnect(source, signal, receiver, slot)){       \
        qt_assert_x(                                        \
            Q_FUNC_INFO,                                    \
            "CHECKED_DISCONNECT failed",                    \
            __FILE__,                                       \
            __LINE__                                        \
        );                                                  \
    }

class Utils
{
    public:
        enum ChompState {
            CHOMP_STATE_ERROR   = 0,
            CHOMP_STATE_FIRST_N,
            CHOMP_STATE_FIRST_R,
            CHOMP_STATE_DONE
        };
        static void removeSpacesFromStart(string    &);
        static void removeSpacesFromEnd(string      &);
        static void removeSpaces(
            string  &a_data,
            int32_t a_from_start    = 1,
            int32_t a_from_end      = 1
        );
        static void chomp(string &);
        static void chomp2(string &);

        static double fRand(
            const double &a_min,
            const double &a_max
        );
};

#endif

