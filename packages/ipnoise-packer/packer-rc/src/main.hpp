#ifndef MAIN_HPP
#define MAIN_HPP

#include <sys/types.h>
#include <dirent.h>

#include <ipnoise-common/log.h>
#include <ipnoise-common/strings.h>
#include <ipnoise-common/path.hpp>

#include "packer.hpp"
#include "config.hpp"

#define VERSION "ver. 0.01"

void usage(int, char **);

int list(string &);
int processAddFile(
    Config          *,
    Packer          *,
    const Path      &
);
int processAddDir(
    Config          *,
    Packer          *,
    const Path      &
);
int processAdd(
    Config      *,
    Packer      *,
    const Path  &
);
int main(int, char **);
int processDel(Config  *, Packer  *);
int processSave(
    Config          *,
    Packer          *,
    const Path      &,
    const int       &
);
int processSaveAll(
    Config      *,
    Packer      *,
    const int   &
);

#endif

