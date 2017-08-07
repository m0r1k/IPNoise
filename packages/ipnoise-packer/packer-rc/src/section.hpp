class Section;

#ifndef SECTION_HPP
#define SECTION_HPP

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdint.h>
#include <lzma.h>

// stl
#include <string>
#include <map>

// ipnoise
#include <ipnoise-common/log_common.h>
#include <ipnoise-common/strings.h>
#include <ipnoise-common/path.hpp>

using namespace std;

#define MAGIC 0x2210

typedef map<string, string>     SectionVars;
typedef SectionVars::iterator   SectionVarsIt;

// File:
//
//  ----------
// | Section1 |
//  ----------
// | Section2 |
//  ----------
// | SectionN |
//  ----------
//

enum CompressType
{
    COMPRESS_TYPE_NULL  = 0,
    COMPRESS_TYPE_LZMA  = 1
};

class SectionHeader
{
    public:
        SectionHeader(){
            content_offset  = 0;
            content_size    = 0;
            vars_offset     = 0;
            vars_size       = 0;
            compress_type   = 0;
            magic           = 0;
        }
        ~SectionHeader(){};

        uint32_t    content_offset;
        uint32_t    content_size;
        uint32_t    vars_offset;
        uint32_t    vars_size;
        uint32_t    compress_type;
        uint32_t    magic;
};

// Section:
//
// content offset /-----> -----------------------------------------
//                |      |..ELF....................................|
// vars offset    | /---> -----------------------------------------
//                | |    |create_date=Sat Oct 20 03:31:36 MSK 2012 |
//                | |    |create_time=1350689590                   |
// header         | | /-> -----------------------------------------
//                | | |  | struct SectionHeader                    |
//                | | |  | {                                       |
//                | | |  |     uint32_t        content_offset;     |
//                | | |  |     uint32_t        content_size;       |
//                | | |  |     uint32_t        vars_offset;        |
//                | | |  |     uint32_t        vars_size;          |
//                | | |  |     uint32_t        magic;              |
//                | | |  | };                                      |
// offset 0 ------------ ------------------------------------------

class Section
{
    public:
        Section();
        ~Section();

        void                setType(const string &);
        string              getType();
        void                setPath(const Path &);
        Path                getPath();
        void                setPerm(const string &);
        string              getPerm();
        string              debug();
        int                 isCompressed();
        void                getContentFile(Path &);
        int                 setContentFile(const Path &);
        void                getContentPos(ssize_t &);
        void                setContentPos(const ssize_t &);
        void                getContentSize(ssize_t &);
        void                setContentSize(const ssize_t &);
        void                setHeader(SectionHeader *);
        SectionHeader *     getHeader();
        SectionHeader *     getCreateHeader();
        SectionVars *       getVars();
        void                serializeVars(string &);
        SectionVars *       getCreateVars();
        string              getVar(const string &);
        void                setVar(const string &, const string &);
        void                setVars(SectionVars *);
        void                setSectionPos(const ssize_t &);
        ssize_t             getSectionPos();
        void                setDir(const Path &);
        int                 save(
            const Path      &a_file,
            const int       &a_uncompress = 1
        );

        static void         parseVars(
            SectionVars *a_vars,
            char        *a_buffer
        );
        static Section *    parse(FILE *);

        // compress
        int     compressFile(
            const Path  &a_path_in,
            const Path  &a_path_out
        );
        int             initEncoder(
            lzma_stream     *a_strm,
            uint32_t        a_preset
        );
        int             compress(
            lzma_stream     *a_strm,
            const Path      &a_inname,
            FILE            *a_infile,
            FILE            *a_outfile,
            ssize_t         a_max_rd_size = -1
        );
        int             compress(
            const Path      &a_inname,
            FILE            *a_infile,
            FILE            *a_outfile,
            ssize_t         a_max_rd_size = -1
        );
        void compressPercent(
            const Path      &,  // file name
            int                 // percent
        );

        // decompress
        int             initDecoder(lzma_stream *a_strm);
        int             decompress(
            lzma_stream     *a_strm,
            const Path      &a_inname,
            FILE            *a_infile,
            FILE            *a_outfile,
            ssize_t         a_max_rd_size = -1
        );
        int             decompress(
            const Path      &a_inname,
            FILE            *a_infile,
            FILE            *a_outfile,
            ssize_t         a_max_rd_size
        );
        void deCompressPercent(
            const Path      &,  // file name
            int                 // percent
        );
        int deCompressFile(
            const Path  &a_path_in,
            const Path  &a_path_out
        );

        void setDeCompressPercentCb(
            void (* a_decompress_percent_cb)(
                Section     *,
                const Path  &,
                int,
                void *
            ),
            void *a_ctx
        );

        void setCompressPercentCb(
            void (* a_compress_percent_cb)(
                Section     *,
                const Path  &,
                int,
                void *
            ),
            void *a_ctx
        );

        void    setPercent(const int &);
        int     getPercent();
        void    clearPercent();

    private:
        Path            m_dir;          // root dir (for extract, pack etc.)
        ssize_t         m_section_pos;  // section pos (from start)
        Path            m_content_file; // content for load
        ssize_t         m_content_pos;  // content pos in file (from start)
        ssize_t         m_content_size; // content size
        SectionHeader   *m_header;      // section header
        SectionVars     *m_vars;        // section vars
        void (* m_decompress_percent_cb)(
            Section     *,  // section
            const Path  &,  // file
            int,            // percent
            void *          // ctx
        );
        void * m_decompress_percent_cb_ctx;

        void (* m_compress_percent_cb)(
            Section     *,  // section
            const Path  &,  // file
            int,            // percent
            void *          // ctx
        );
        void * m_compress_percent_cb_ctx;

        int m_percent;
};

#endif

