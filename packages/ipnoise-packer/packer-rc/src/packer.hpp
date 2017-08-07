class Packer;

#ifndef PACKER_HPP
#define PACKER_HPP

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <getopt.h>
#include <time.h>
#include <lzma.h>

// stl
#include <string>
#include <vector>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/path.hpp>

#ifdef COMPRESS_MULTITHREADS
#include <ipnoise-common/thread.hpp>
#endif

#include "section.hpp"

using namespace std;

typedef vector<Section *>   Sections;
typedef Sections::iterator  SectionsIt;

#define MAX_PACKER_THREADS 6

class Packer
{
    public:
        Packer();
        ~Packer();

        string              show();
        int                 parse(const Path &);
        void                deleteSection(Section *);

        void                setDir(const Path &a_dir);
        Section *           addFile(
            const Path          &,
            const ssize_t       &a_size = ssize_t(-1)
        );
        int                 addDirRecursive(const Path &);
        Section *           addDir(const Path &);
        int                 build(const Path &);

        int                 getSectionsCount();
        Section *           getSectionByPath(const Path &);
        int                 saveAll(const int &a_uncompress = 1);
        int                 save(
            const Path      &a_path,
            const int       &a_uncompress = 1
        );

        void clearPercents();

#ifdef COMPRESS_MULTITHREADS
        int             compressMultiThreads();
        static void *   compressMultiThread(void *);
#endif

        static void deCompressSectionPercentCb(
            Section     *a_section,
            const Path  &a_file,
            int         a_percent,
            void        *a_ctx
        );

        static void compressSectionPercentCb(
            Section     *a_section,
            const Path  &a_file,
            int         a_percent,
            void        *a_ctx
        );

        void setDecompressPercentCb(
            void (* a_decompress_percent_cb)(
                Packer *,       // Packer
                int,            // total percent
                const Path  &,  // current file
                int,            // current file percent
                void *          // ctx
            ),
            void *a_ctx
        );

        void setCompressPercentCb(
            void (* a_compress_percent_cb)(
                Packer *,       // Packer
                int,            // total percent
                const Path  &,  // current file
                int,            // cuurent file percent
                void *          // ctx
            ),
            void *a_ctx
        );


    private:
        Path            m_dir;
        Path            m_file;
        Sections        *m_sections;
#ifdef COMPRESS_MULTITHREADS
        Sections        *m_sections_for_threads;
        pthread_mutex_t m_mutex;
#endif

        void (* m_decompress_percent_cb)(
            Packer *,       // section
            int,            // total percent
            const Path  &,  // file
            int,            // percent
            void *          // ctx
        );
        void * m_decompress_percent_cb_ctx;

        void (* m_compress_percent_cb)(
            Packer *,       // section
            int,            // total percent
            const Path  &,  // file
            int,            // percent
            void *          // ctx
        );
        void * m_compress_percent_cb_ctx;


};

#endif

