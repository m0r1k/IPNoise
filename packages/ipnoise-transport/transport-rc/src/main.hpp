#ifndef MAIN_HPP
#define MAIN_HPP

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <string>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

// Local
#include "ipnoise-common/log.h"

using namespace std;

#define IPNOISE_TRANSPORT_VERSION "0.03"

#endif

