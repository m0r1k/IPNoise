#include "section.hpp"

#ifndef min
#define min(a, b) (a < b) ? a : b
#endif

Section::Section()
{
    m_section_pos       = -1;
    m_header            = NULL;
    m_vars              = NULL;
    m_content_pos       = 0;
    m_content_size      = -1;

    m_percent                   = 0;
    m_compress_percent_cb       = NULL;
    m_compress_percent_cb_ctx   = NULL;
    m_decompress_percent_cb     = NULL;
    m_decompress_percent_cb_ctx = NULL;
}

Section::~Section()
{
    if (m_header){
        delete m_header;
        m_header = NULL;
    }
    if (m_vars){
        delete m_vars;
        m_vars = NULL;
    }
}

void Section::setType(const string &a_type)
{
    setVar("section.type", a_type);
}

string Section::getType()
{
    return getVar("section.type");
}

Path Section::getPath()
{
    Path l_path;
    // file path in unix format
    l_path.setPathUnix(getVar("section.path"));
    return l_path;
}

void Section::setPath(const Path &a_path)
{
    // file path in unix format
    string full_path = a_path.pathUnix();
    setVar("section.path", full_path);
}

void Section::setPerm(const string &a_perm)
{
    setVar("section.perm", a_perm);
}

string Section::getPerm()
{
    return getVar("section.perm");
}

string Section::debug()
{
    string          ret;
    char            buffer[1024]    = { 0x00 };
    SectionVarsIt   vars_it;

    ret += "Section:\n";

    // section pos
    snprintf(buffer, sizeof(buffer),
        "    Section pos:  '%d'\n",
        m_section_pos
    );
    ret += buffer;

    // content file
    snprintf(buffer, sizeof(buffer),
        "    Content file: '%s'\n",
        m_content_file.path().c_str()
    );
    ret += buffer;

    // content pos
    snprintf(buffer, sizeof(buffer),
        "    Content pos:  '%d'\n",
        m_content_pos
    );
    ret += buffer;

    // content size
    snprintf(buffer, sizeof(buffer),
        "    Content size: '%d'\n",
        m_content_size
    );
    ret += buffer;

    // fheader
    if (m_header){
        snprintf(buffer, sizeof(buffer),
            "    Header:\n"
            "        Content offset:    '%d'\n"
            "        Content size:      '%d'\n"
            "        Vars offset:       '%d'\n"
            "        Vars size:         '%d'\n"
            "        Compress type:     '%d'\n"
            "        Magic:           0x'%x'\n",
            m_header->content_offset,
            m_header->content_size,
            m_header->vars_offset,
            m_header->vars_size,
            m_header->compress_type,
            m_header->magic
        );
        ret += buffer;
    } else {
        snprintf(buffer, sizeof(buffer),
            "    Header:\n"
            "        Content offset:    '?'\n"
            "        Content size:      '?'\n"
            "        Vars offset:       '?'\n"
            "        Vars size:         '?'\n"
            "        Compress type:     '?'\n"
            "        Magic:             '?'\n"
        );
        ret += buffer;
    }

    // vars
    ret += "    Vars:\n";
    if (m_vars){
        for (vars_it = m_vars->begin();
            vars_it != m_vars->end();
            vars_it++)
        {
            string key = vars_it->first;
            string val = vars_it->second;
            ret += "        " + key + ": " + val + "\n";
        }
    }

    return ret;
}

int Section::isCompressed()
{
    SectionHeader *header = getCreateHeader();

    return (header->compress_type > COMPRESS_TYPE_NULL);
}

void Section::serializeVars(string &a_out)
{
    SectionVarsIt vars_it;

    if (not m_vars){
        goto out;
    }

    for (vars_it = m_vars->begin();
        vars_it != m_vars->end();
        vars_it++)
    {
        string key = vars_it->first;
        string val = vars_it->second;
        a_out += key + "=" + val + "\n";
    }

out:
    return;
}

SectionHeader * Section::getHeader()
{
    return m_header;
}

SectionHeader * Section::getCreateHeader()
{
    if (not m_header){
        m_header = new SectionHeader;
        if (not m_header){
            PERROR("Cannot allocate memory\n");
            goto fail;
        }
    }

out:
    return m_header;

fail:
    if (m_header){
        delete m_header;
        m_header = NULL;
    }
    goto out;
}

void Section::setHeader(
    SectionHeader *a_header)
{
    if (m_header){
        delete m_header;
        m_header = NULL;
    }
    m_header = a_header;
}

void Section::setVar(
    const string &a_key,
    const string &a_val)
{
    SectionVars *section_vars = NULL;

    section_vars = getCreateVars();

    PDEBUG(5, "Set section var: '%s' in '%s'\n",
        a_key.c_str(),
        a_val.c_str()
    );

    (*section_vars)[a_key] = a_val;
}

void Section::getContentFile(Path &a_out)
{
    a_out = m_content_file;
}

int Section::setContentFile(
    const Path &a_content_file)
{
    int err = 0;

    // TODO check what file exist
    m_content_file = a_content_file;

    return err;
}

void Section::getContentPos(ssize_t &a_out)
{
    a_out = m_content_pos;
}

void Section::setContentPos(const ssize_t &a_pos)
{
    m_content_pos = a_pos;
}

void Section::getContentSize(ssize_t &a_size)
{
    a_size = m_content_size;
}

void Section::setContentSize(const ssize_t &a_size)
{
    m_content_size = a_size;
}

SectionVars * Section::getVars()
{
    return m_vars;
}

SectionVars * Section::getCreateVars()
{
    if (not m_vars){
        m_vars = new SectionVars;
        if (not m_vars){
            PERROR("Cannot allocate memory\n");
            goto fail;
        }
    }

out:
    return m_vars;

fail:
    if (m_vars){
        delete m_vars;
        m_vars = NULL;
    }
    goto out;
}

void Section::setVars(
    SectionVars *a_vars)
{
    if (m_vars){
        delete m_vars;
        m_vars = NULL;
    }
    m_vars = a_vars;
}

string Section::getVar(
    const string &a_var_name)
{
    string          ret;
    SectionVarsIt   vars_it;

    if (not m_vars){
        goto out;
    }

    vars_it = m_vars->find(a_var_name);
    if (m_vars->end() != vars_it){
        ret = vars_it->second;
    }

out:
    return ret;
}

void Section::setSectionPos(
    const ssize_t &a_section_pos)
{
    m_section_pos = a_section_pos;
}

ssize_t Section::getSectionPos()
{
    return m_section_pos;
}

void Section::parseVars(
    SectionVars *a_vars,
    char        *a_buffer)
{
    int i;
    Splitter vars(a_buffer, "\n");

    for (i = 0; i < (int)vars.size(); i++){
        string line = vars[i];
        if (not line.size()){
            continue;
        }
        if ('#' == line.at(0)){
            continue;
        }
        Splitter    key_val(line.c_str(), "=");
        string      key;
        string      val;
        if (not key_val.size()){
            continue;
        }
        key = key_val[0];
        if (key_val.size() > 1){
            val = key_val[1];
        }
        (*a_vars)[key] = val;
    }
}

void Section::setDir(const Path &a_dir)
{
    m_dir = a_dir;
}

int Section::save(
    const Path      &a_file,
    const int       &a_uncompress)
{
    int    res, err  = -1;
    FILE   *fin         = NULL;
    FILE   *fout        = NULL;
    string  perm        = getPerm();

    Path    out_file;
    Path    content_file;
    ssize_t content_pos     = -1;
    ssize_t content_size    = 0;

    if (not a_file.empty()){
        out_file = a_file;
    } else {
        out_file = m_dir;
        out_file += getPath();
    }

    if (not m_header){
        PERROR("Cannot save file: '%s', cannot read m_header\n",
            out_file.path().c_str()
        );
        goto fail;
    }

    if ("dir" == getType()){
        Path dir_name = out_file;

        PDEBUG(5, "trying create directory: '%s'\n",
            dir_name.path().c_str());

        // update percents
        deCompressPercent(dir_name, 0);

        // creating new dir
        res = dir_name.mkdirr(Path::parseMode(perm));

        // update percents
        deCompressPercent(dir_name, 100);

        // check result
        if (res){
            PERROR("Cannot create dir: '%s'\n",
                dir_name.path().c_str())
            goto fail;
        }

        // all ok
        err = 0;
        goto out;
    }

    // collect information about content
    getContentFile(content_file);
    getContentPos(content_pos);
    content_size = m_header->content_size;

    PDEBUG(5,
        "content_file: '%s',"
        " path: '%s',"
        " content_pos: '%d',"
        " content_size: '%d'\n",
        content_file.path().c_str(),
        out_file.path().c_str(),
        content_pos,
        content_size
    );

    // open content file
    fin = content_file.fopen("rb");

    if (not fin){
        PERROR("Cannot open file: '%s' for read\n",
            content_file.path().c_str()
        );
        goto fail;
    }

    // seek to content
    if (fseek(fin, content_pos, SEEK_SET)){
        PERROR("cannot seek to content pos: '%d', file: '%s'\n",
            content_pos,
            content_file.path().c_str()
        );
        goto fail;
    }

    // if already exist
    if (out_file.isExist()){
        // try to remove
        res = remove(out_file.path().c_str());
        if (res){
            PDEBUG(5, "cannot unpack file '%s',"
                " file already exist, will be skiped",
                out_file.path().c_str()
            );
            err = 0;
            goto out;
        }
    }

    // open out file
    fout = out_file.fopen("wb");
    if (not fout){
        PERROR("Cannot open file: '%s' for write\n",
            out_file.path().c_str()
        );
        goto fail;
    }

    if (a_uncompress){
        if (COMPRESS_TYPE_LZMA == m_header->compress_type){
            res = decompress(
                content_file,
                fin,
                fout,
                content_size
            );
            if (res){
                PERROR("Failed process file: '%s'\n",
                    content_file.path().c_str()
                );
                goto fail;
            }
        }
    } else {
        int     buffer_size = 1*1024*1024; // Mb
        uint8_t *buffer     = new uint8_t[buffer_size];
        ssize_t need_read   = content_size;
        ssize_t res;

        if (not buffer){
            PERROR("Cannot allocate memory,"
                " was needed: '%d' byte(s)\n",
                buffer_size
            );
            goto fail;
        }
        do {
            res = fread(
                buffer,
                1,
                min(buffer_size, need_read),
                fin
            );
            PDEBUG(5, "Was read: '%d' byte(s) from: '%s'\n",
                res, content_file.path().c_str());
            if (res <= 0){
                break;
            }
            need_read   -= res;
            res         = fwrite(buffer, 1, res, fout);
            PDEBUG(5, "Was wrote: '%d' byte(s) to: '%s'\n",
                res, a_file.path().c_str()
            );
        } while (res > 0);

        delete [] buffer;
    }

    // all ok
    err = 0;

out:
    if (fin){
        fclose(fin);
        fin = NULL;
    }
    if (fout){
        fclose(fout);
        fout = NULL;
#if defined(WIN32) || defined(WIN64)
        // TODO
#else
        // return permissions
        res = out_file.chmod(perm);
        if (res){
            PERROR("chmod: '%s' for: '%s' failed\n",
                perm.c_str(),
                out_file.path().c_str()
            );
        }
#endif
    }
    return err;

fail:
    if (0 >= err){
        err = -1;
    }
    goto out;
}

Section * Section::parse(
    FILE    *a_file)
{
    Section         *section        = new Section;
    SectionHeader   *header         = new SectionHeader;
    SectionVars     *vars           = new SectionVars;
    ssize_t         res             = 0;
    char            *buffer         = NULL;

    if (not section){
        PERROR("Cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            sizeof(*section)
        );
        goto fail;
    }

    if (not header){
        PERROR("Cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            sizeof(*header)
        );
        goto fail;
    }

    if (not vars){
        PERROR("Cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            sizeof(*vars)
        );
        goto fail;
    }

    PDEBUG(5, "read section\n");

    // clear section header
    memset(header, 0x00, sizeof(*header));

    // trying to read header
    PDEBUG(5, "read section header\n");
    if (fseek(a_file, -sizeof(*header), SEEK_CUR)){
        PDEBUG(5, "cannot seek to section header\n");
        goto fail;
    }
    res = fread(header, 1, sizeof(*header), a_file);
    if (res != sizeof(*header)){
        PDEBUG(5, "cannot read section header\n");
        goto fail;
    }
    if (fseek(a_file, -sizeof(*header), SEEK_CUR)){
        PDEBUG(5, "cannot seek to section header\n");
        goto fail;
    }
    PDEBUG(5, "was read section header:\n"
        "   content_offset:     '%d'\n"
        "   content_size:       '%d'\n"
        "   var_offset:         '%d'\n"
        "   var_size:           '%d'\n"
        "   magic:            0x'%x'\n",
        header->content_offset,
        header->content_size,
        header->vars_offset,
        header->vars_size,
        header->magic
    );
    if (MAGIC != header->magic){
        PDEBUG(5, "invalid section magic: '%x'\n", header->magic);
        goto fail;
    }

    // store header
    section->setHeader(header);

    // store section file position
    section->setSectionPos(ftell(a_file));

    // store content size
    section->setContentSize(header->content_size);

    // read vars
    if (fseek(a_file, -(header->vars_size), SEEK_CUR)){
        PDEBUG(5, "cannot seek to section vars\n");
        goto fail;
    }
    buffer = new char[header->vars_size + 1];
    if (not buffer){
        PERROR("Cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            header->vars_size
        );
        goto fail;
    }
    memset(buffer, 0x00, header->vars_size + 1);
    //
    res = fread(buffer, 1, header->vars_size, a_file);
    if (res != ssize_t(header->vars_size)){
        PDEBUG(5, "cannot read section vars\n");
        goto fail;
    }
    if (fseek(a_file, -(header->vars_size), SEEK_CUR)){
        PDEBUG(5, "cannot seek to section vars\n");
        goto fail;
    }
    Section::parseVars(vars, buffer);
    delete [] buffer;

    // store vars
    section->setVars(vars);

    // move file pointer to possible next section
    if (fseek(a_file, -(header->content_size), SEEK_CUR)){
        PDEBUG(5, "cannot seek to next section\n");
        goto fail;
    }

    // store file content pos
    section->setContentPos(ftell(a_file));

out:
    return section;

fail:
    if (section){
        delete section;
        section = NULL;
    }
    if (header){
        delete header;
        header = NULL;
    }
    if (vars){
        delete vars;
        vars = NULL;
    }
    goto out;
}

int Section::initEncoder(
    lzma_stream     *a_strm,
    uint32_t        a_preset)
{
    int         err     = -1;
    const char  *msg    = NULL;

    // Initialize the encoder using a preset. Set the integrity to check
    // to CRC64, which is the default in the xz command line tool. If
    // the .xz file needs to be decompressed with XZ Embedded, use
    // LZMA_CHECK_CRC32 instead.

    lzma_ret ret = lzma_easy_encoder(a_strm, a_preset, LZMA_CHECK_CRC64);

    // Return successfully if the initialization went fine.
    if (ret == LZMA_OK){
        err = 0;
        goto out;
    }

    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).

    switch (ret){
        case LZMA_MEM_ERROR:
            msg = "Memory allocation failed";
            break;

        case LZMA_OPTIONS_ERROR:
            msg = "Specified preset is not supported";
            break;

        case LZMA_UNSUPPORTED_CHECK:
            msg = "Specified integrity check is not supported";
            break;

    default:
        // This is most likely LZMA_PROG_ERROR indicating a bug in
        // this program or in liblzma. It is inconvenient to have a
        // separate error message for errors that should be impossible
        // to occur, but knowing the error code is important for
        // debugging. That's why it is good to print the error code
        // at least when there is no good error message to show.
        msg = "Unknown error, possibly a bug";
        break;
    }

    PERROR("Error initializing the encoder: %s (error code %u)\n",
        msg, ret
    );

out:
    return err;
}

int Section::compress(
    const Path      &a_inname,
    FILE            *a_infile,
    FILE            *a_outfile,
    ssize_t         a_max_rd_size)
{
    int res, err = -1;

    // process via lzma
    lzma_stream strm = LZMA_STREAM_INIT;

    res = initEncoder(&strm, LZMA_PRESET_EXTREME);
    if (res){
        PERROR("Cannot init lzma encoder\n");
        goto fail;
    }

    res = compress(
        &strm,
        a_inname,
        a_infile,
        a_outfile,
        a_max_rd_size
    );

    lzma_end(&strm);

    if (res){
        PERROR("Compress '%s' failed\n",
            a_inname.path().c_str()
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int Section::compress(
    lzma_stream     *a_strm,
    const Path      &a_inname,
    FILE            *a_infile,
    FILE            *a_outfile,
    ssize_t         a_max_rd_size)
{
    int         err         = -1;
    const char  *msg        = NULL;
    int64_t     was_read    = 0;

    PDEBUG(5, "Section::compress: '%s',"
        " a_max_rd_size: '%d'\n",
        a_inname.path().c_str(),
        a_max_rd_size
    );

    // This will be LZMA_RUN until the end of the input file is reached.
    // This tells lzma_code() when there will be no more input.

    lzma_action action = LZMA_RUN;

    // Buffers to temporarily hold uncompressed input
    // and compressed output.

    uint8_t inbuf[BUFSIZ];
    uint8_t outbuf[BUFSIZ];

    // Initialize the input and output pointers. Initializing next_in
    // and avail_in isn't really necessary when we are going to encode
    // just one file since LZMA_STREAM_INIT takes care of initializing
    // those already. But it doesn't hurt much and it will be needed
    // if encoding more than one file like we will in 02_decompress.c.
    //
    // While we don't care about strm->total_in or strm->total_out in this
    // example, it is worth noting that initializing the encoder will
    // always reset total_in and total_out to zero. But the encoder
    // initialization doesn't touch next_in, avail_in, next_out, or
    // avail_out.

    a_strm->next_in     = NULL;
    a_strm->avail_in    = 0;
    a_strm->next_out    = outbuf;
    a_strm->avail_out   = sizeof(outbuf);

    // Loop until the file has been successfully compressed or until
    // an error occurs.

    compressPercent(a_inname, 0);

    while (true) {
        // Fill the input buffer if it is empty.
        if (    a_strm->avail_in == 0
            &&  !feof(a_infile))
        {
            a_strm->next_in = inbuf;
            int size = sizeof(inbuf);
            if (a_max_rd_size >= 0){
                size = min(size, (a_max_rd_size - was_read));
            }
            a_strm->avail_in = fread(
                inbuf,
                1,
                size,
                a_infile
            );

            if (ferror(a_infile)){
                PERROR("%s: Read error\n",
                    a_inname.path().c_str()
                );
                goto fail;
            }

            if (a_strm->avail_in > 0){
                was_read += a_strm->avail_in;
                if (    a_max_rd_size >= 0
                    &&  was_read >= a_max_rd_size)
                {
                    action = LZMA_FINISH;
                }
            }

            if (a_max_rd_size > 0){
                compressPercent(
                    a_inname,
                    (was_read * 100) / a_max_rd_size
                );
            }

            // Once the end of the input file has been reached,
            // we need to tell lzma_code() that no more input
            // will be coming and that it should finish the
            // encoding.

            if (feof(a_infile)){
                action = LZMA_FINISH;
            }
        }

        // Tell liblzma do the actual encoding.
        //
        // This reads up to strm->avail_in bytes of input starting
        // from strm->next_in. avail_in will be decremented and
        // next_in incremented by an equal amount to match the
        // number of input bytes consumed.
        //
        // Up to strm->avail_out bytes of compressed output will be
        // written starting from strm->next_out. avail_out and next_out
        // will be incremented by an equal amount to match the number
        // of output bytes written.
        //
        // The encoder has to do internal buffering, which means that
        // it may take quite a bit of input before the same data is
        // available in compressed form in the output buffer.

        lzma_ret ret = lzma_code(a_strm, action);

        // If the output buffer is full or if the compression finished
        // successfully, write the data from the output bufffer to
        // the output file.

        if (a_strm->avail_out == 0 || ret == LZMA_STREAM_END){
            // When lzma_code() has returned LZMA_STREAM_END,
            // the output buffer is likely to be only partially
            // full. Calculate how much new data there is to
            // be written to the output file.

            size_t write_size = sizeof(outbuf) - a_strm->avail_out;
            size_t res;
            res = fwrite(outbuf, 1, write_size, a_outfile);
            PDEBUG(10, "was wrote: '%d' byte(s)\n", res);
            if (res != write_size){
                PERROR("Write error\n");
                goto fail;
            }

            // Reset next_out and avail_out.
            a_strm->next_out  = outbuf;
            a_strm->avail_out = sizeof(outbuf);
        }

        // Normally the return value of lzma_code() will be LZMA_OK
        // until everything has been encoded.

        if (ret != LZMA_OK){
            // Once everything has been encoded successfully, the
            // return value of lzma_code() will be LZMA_STREAM_END.
            //
            // It is important to check for LZMA_STREAM_END. Do not
            // assume that getting ret != LZMA_OK would mean that
            // everything has gone well.

            if (ret == LZMA_STREAM_END){
                err = 0;
                goto out;
            }

            // It's not LZMA_OK nor LZMA_STREAM_END,
            // so it must be an error code. See lzma/base.h
            // (src/liblzma/api/lzma/base.h in the source package
            // or e.g. /usr/include/lzma/base.h depending on the
            // install prefix) for the list and documentation of
            // possible values. Most values listen in lzma_ret
            // enumeration aren't possible in this example.

            switch (ret){
                case LZMA_MEM_ERROR:
                    msg = "Memory allocation failed";
                    break;

                case LZMA_DATA_ERROR:
                    // This error is returned if the compressed
                    // or uncompressed size get near 8 EiB
                    // (2^63 bytes) because that's where the .xz
                    // file format size limits currently are.
                    // That is, the possibility of this error
                    // is mostly theoretical unless you are doing
                    // something very unusual.
                    //
                    // Note that strm->total_in and strm->total_out
                    // have nothing to do with this error. Changing
                    // those variables won't increase or decrease
                    // the chance of getting this error.
                    msg = "File size limits exceeded";
                    break;

                default:
                    // This is most likely LZMA_PROG_ERROR, but
                    // if this program is buggy (or liblzma has
                    // a bug), it may be e.g. LZMA_BUF_ERROR or
                    // LZMA_OPTIONS_ERROR too.
                    //
                    // It is inconvenient to have a separate
                    // error message for errors that should be
                    // impossible to occur, but knowing the error
                    // code is important for debugging. That's why
                    // it is good to print the error code at least
                    // when there is no good error message to show.
                    msg = "Unknown error, possibly a bug";
                    break;
            }

            PERROR("%s: Encoder error: %s (error code %u)\n",
                a_inname.path().c_str(), msg, ret);
            goto fail;
        }
    }

out:
    compressPercent(a_inname, 100);
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int Section::initDecoder(lzma_stream *a_strm)
{
    // Initialize a .xz decoder. The decoder supports a memory usage limit
    // and a set of flags.
    //
    // The memory usage of the decompressor depends on the settings used
    // to compress a .xz file. It can vary from less than a megabyte to
    // a few gigabytes, but in practice (at least for now) it rarely
    // exceeds 65 MiB because that's how much memory is required to
    // decompress files created with "xz -9". Settings requiring more
    // memory take extra effort to use and don't (at least for now)
    // provide significantly better compression in most cases.
    //
    // Memory usage limit is useful if it is important that the
    // decompressor won't consume gigabytes of memory. The need
    // for limiting depends on the application. In this example,
    // no memory usage limiting is used. This is done by setting
    // the limit to UINT64_MAX.
    //
    // The .xz format allows concatenating compressed files as is:
    //
    //     echo foo | xz > foobar.xz
    //     echo bar | xz >> foobar.xz
    //
    // When decompressing normal standalone .xz files, LZMA_CONCATENATED
    // should always be used to support decompression of concatenated
    // .xz files. If LZMA_CONCATENATED isn't used, the decoder will stop
    // after the first .xz stream. This can be useful when .xz data has
    // been embedded inside another file format.
    //
    // Flags other than LZMA_CONCATENATED are supported too, and can
    // be combined with bitwise-or. See lzma/container.h
    // (src/liblzma/api/lzma/container.h in the source package or e.g.
    // /usr/include/lzma/container.h depending on the install prefix)
    // for details.
    lzma_ret ret = lzma_stream_decoder(
        a_strm,
        UINT64_MAX,
        LZMA_CONCATENATED
    );

    // Return successfully if the initialization went fine.
    if (ret == LZMA_OK)
        return true;

    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).
    //
    // Note that LZMA_MEMLIMIT_ERROR is never possible here. If you
    // specify a very tiny limit, the error will be delayed until
    // the first headers have been parsed by a call to lzma_code().
    const char *msg;
    switch (ret) {
    case LZMA_MEM_ERROR:
        msg = "Memory allocation failed";
        break;

    case LZMA_OPTIONS_ERROR:
        msg = "Unsupported decompressor flags";
        break;

    default:
        // This is most likely LZMA_PROG_ERROR indicating a bug in
        // this program or in liblzma. It is inconvenient to have a
        // separate error message for errors that should be impossible
        // to occur, but knowing the error code is important for
        // debugging. That's why it is good to print the error code
        // at least when there is no good error message to show.
        msg = "Unknown error, possibly a bug";
        break;
    }

    PINFO("Error initializing the decoder: %s (error code %u)\n",
            msg, ret);
    return false;
}

int Section::decompress(
    const Path      &a_inname,
    FILE            *a_infile,
    FILE            *a_outfile,
    ssize_t         a_max_rd_size)
{
    int res, err = -1;

    lzma_stream strm = LZMA_STREAM_INIT;

    res = initDecoder(&strm);
    if (not res){
        goto fail;
    }

    res = decompress(
        &strm,
        a_inname,
        a_infile,
        a_outfile,
        a_max_rd_size
    );

    lzma_end(&strm);

    if (res){
        PERROR("Cannot decompress: '%s'\n",
            a_inname.path().c_str()
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}


int Section::decompress(
    lzma_stream     *a_strm,
    const Path      &a_inname,
    FILE            *a_infile,
    FILE            *a_outfile,
    ssize_t         a_max_rd_size)
{
    int     err             = -1;
    int64_t was_read        = 0;
    int     last_percent    = 0;

    uint8_t inbuf[BUFSIZ];
    uint8_t outbuf[BUFSIZ];

    // When LZMA_CONCATENATED flag was used when initializing the decoder,
    // we need to tell lzma_code() when there will be no more input.
    // This is done by setting action to LZMA_FINISH instead of LZMA_RUN
    // in the same way as it is done when encoding.
    //
    // When LZMA_CONCATENATED isn't used, there is no need to use
    // LZMA_FINISH to tell when all the input has been read, but it
    // is still OK to use it if you want. When LZMA_CONCATENATED isn't
    // used, the decoder will stop after the first .xz stream. In that
    // case some unused data may be left in a_strm->next_in.
    lzma_action action = LZMA_RUN;

    a_strm->next_in   = NULL;
    a_strm->avail_in  = 0;
    a_strm->next_out  = outbuf;
    a_strm->avail_out = sizeof(outbuf);

    deCompressPercent(a_inname, 0);

    while (true){
        if (    a_strm->avail_in == 0
            &&  !feof(a_infile))
        {
            a_strm->next_in = inbuf;
            int size = sizeof(inbuf);
            if (a_max_rd_size >= 0){
                size = min(size, (a_max_rd_size - was_read));
            }

            PDEBUG(10, "attempt to read: '%d' byte(s) from: '%s'"
                " ('%ld' filepos)\n",
                size,
                a_inname.path().c_str(),
                ftell(a_infile)
            );

            a_strm->avail_in = fread(
                inbuf,
                1,
                size,
                a_infile
            );

            PDEBUG(10, "was read: '%d' byte(s) from: '%s'"
                " ('%d' requested)\n",
                a_strm->avail_in,
                a_inname.path().c_str(),
                size
            );

            if (ferror(a_infile)) {
                PERROR("%s: Read error\n",  a_inname.path().c_str());
                goto fail;
            }

            if (a_strm->avail_in > 0){
                was_read += a_strm->avail_in;
                if (    a_max_rd_size >= 0
                    &&  was_read >= a_max_rd_size)
                {
                    action = LZMA_FINISH;
                }
            }

            if (a_max_rd_size > 0){
                int percent = (was_read * 100) / a_max_rd_size;
                if (percent != last_percent){
                    deCompressPercent(
                        a_inname,
                        percent
                    );
                    last_percent = percent;
                }
            }

            // Once the end of the input file has been reached,
            // we need to tell lzma_code() that no more input
            // will be coming. As said before, this isn't required
            // if the LZMA_CONATENATED flag isn't used when
            // initializing the decoder.
            if (feof(a_infile)){
                action = LZMA_FINISH;
            }
        }

        lzma_ret ret = lzma_code(a_strm, action);

        if (a_strm->avail_out == 0 || ret == LZMA_STREAM_END) {
            size_t write_size = sizeof(outbuf) - a_strm->avail_out;

            if (fwrite(outbuf, 1, write_size, a_outfile) != write_size){
                PERROR("Write error\n");
                goto fail;
            }

            a_strm->next_out    = outbuf;
            a_strm->avail_out   = sizeof(outbuf);
        }

        if (ret != LZMA_OK) {
            // Once everything has been decoded successfully, the
            // return value of lzma_code() will be LZMA_STREAM_END.
            //
            // It is important to check for LZMA_STREAM_END. Do not
            // assume that getting ret != LZMA_OK would mean that
            // everything has gone well or that when you aren't
            // getting more output it must have successfully
            // decoded everything.
            if (ret == LZMA_STREAM_END){
                err = 0;
                goto out;
            }

            // It's not LZMA_OK nor LZMA_STREAM_END,
            // so it must be an error code. See lzma/base.h
            // (src/liblzma/api/lzma/base.h in the source package
            // or e.g. /usr/include/lzma/base.h depending on the
            // install prefix) for the list and documentation of
            // possible values. Many values listen in lzma_ret
            // enumeration aren't possible in this example, but
            // can be made possible by enabling memory usage limit
            // or adding flags to the decoder initialization.
            const char *msg;
            switch (ret) {
                case LZMA_MEM_ERROR:
                    msg = "Memory allocation failed";
                    break;

                case LZMA_FORMAT_ERROR:
                    // .xz magic bytes weren't found.
                    msg = "The input is not in the .xz format";
                    break;

                case LZMA_OPTIONS_ERROR:
                    // For example, the headers specify a filter
                    // that isn't supported by this liblzma
                    // version (or it hasn't been enabled when
                    // building liblzma, but no-one sane does
                    // that unless building liblzma for an
                    // embedded system). Upgrading to a newer
                    // liblzma might help.
                    //
                    // Note that it is unlikely that the file has
                    // accidentally became corrupt if you get this
                    // error. The integrity of the .xz headers is
                    // always verified with a CRC32, so
                    // unintentionally corrupt files can be
                    // distinguished from unsupported files.
                    msg = "Unsupported compression options";
                    break;

                case LZMA_DATA_ERROR:
                    msg = "Compressed file is corrupt";
                    break;

                case LZMA_BUF_ERROR:
                    // Typically this error means that a valid
                    // file has got truncated, but it might also
                    // be a damaged part in the file that makes
                    // the decoder think the file is truncated.
                    // If you prefer, you can use the same error
                    // message for this as for LZMA_DATA_ERROR.
                    msg = "Compressed file is truncated or "
                            "otherwise corrupt";
                    break;

                default:
                    // This is most likely LZMA_PROG_ERROR.
                    msg = "Unknown error, possibly a bug";
                    break;
            }

            PERROR("%s: Decoder error: %s (error code %u)\n",
                a_inname.path().c_str(), msg, ret
            );
            goto fail;
        }
    }

out:
    deCompressPercent(a_inname, 100);
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void Section::compressPercent(
    const Path      &a_file_name,
    int             a_percent)
{
    setPercent(a_percent);
    if (m_compress_percent_cb){
        m_compress_percent_cb(
            this,
            a_file_name,
            a_percent,
            m_decompress_percent_cb_ctx
        );
    }
}

void Section::setDeCompressPercentCb(
    void (* a_decompress_percent_cb)(
        Section     *,
        const Path  &,
        int,
        void *
    ),
    void *a_ctx)
{
    m_decompress_percent_cb     = a_decompress_percent_cb;
    m_decompress_percent_cb_ctx = a_ctx;
}

void Section::setCompressPercentCb(
    void (* a_compress_percent_cb)(
        Section     *,
        const Path  &,
        int,
        void *
    ),
    void *a_ctx)
{
    m_compress_percent_cb       = a_compress_percent_cb;
    m_compress_percent_cb_ctx   = a_ctx;
}

void Section::deCompressPercent(
    const Path      &a_file_name,
    int             a_percent)
{
    setPercent(a_percent);
    if (m_decompress_percent_cb){
        m_decompress_percent_cb(
            this,
            a_file_name,
            a_percent,
            m_decompress_percent_cb_ctx
        );
    }
}

int Section::deCompressFile(
    const Path  &a_path_in,
    const Path  &a_path_out)
{
    int res, err = -1;

    char    buffer[512]     = { 0x00 };
    ssize_t pos             = -1;
    ssize_t start_offset    = -1;
    FILE    *infile         = NULL;
    FILE    *outfile        = NULL;

    time_t  build_time  = 0;
    ssize_t size        = 0;

    lzma_stream strm = LZMA_STREAM_INIT;

    res = initDecoder(&strm);
    if (!res){
        goto fail;
    }

    // in file
    infile = a_path_in.fopen("rb");
    if (not infile){
        PERROR("Cannot open file: '%s' for read\n",
            a_path_in.path().c_str()
        );
        goto fail;
    }

    // out file
    outfile = a_path_out.fopen("wb");
    if (not outfile){
        PERROR("Cannot open file: '%s' for write\n",
            a_path_in.path().c_str()
        );
        goto fail;
    }

    // search "\n\n"
    {
        char prev = '\0';
        for (pos = 1; pos < ssize_t(sizeof(buffer) - 1); pos++){
            char cur;
            if (fseek(infile, -pos, SEEK_END)){
                PERROR("content validation failed\n");
                goto fail;
            }
            if (0 >= fread(&cur, 1, 1, infile)){
                PERROR("content validation failed\n");
                goto fail;
            }
            if (    '\n' == cur
                &&  '\n' == prev)
            {
                // found
                start_offset = pos;
                break;
            }
            prev = cur;
        }
    }

    if (start_offset < 0){
        PERROR("content validation failed\n");
        goto fail;
    }

    // seek to header
    if (fseek(infile, -start_offset, SEEK_END)){
        PERROR("content validation failed\n");
        goto fail;
    }

    // clear buffer
    memset (buffer, 0x00, sizeof(buffer));
    if (0 >= fread(buffer, start_offset, 1, infile)){
        PERROR("content validation failed\n");
        goto fail;
    }

    // read header
    sscanf(buffer,
        "\n\n"
        "BUILD_TIME=%ld\n"
        "SIZE=%d\n",
        &build_time,
        &size
    );

    if (g_debug_level > 10){
        PERROR("buffer: '%s'\n",    buffer);
        PERROR("BUILD_TIME=%ld\n",  build_time);
        PERROR("SIZE=%d\n",         size);
    }

    // seek to content
    if (fseek(infile, -(size + start_offset), SEEK_END)){
        PERROR("content validation failed\n");
        goto fail;
    }

    err = decompress(
        &strm,
        a_path_in,
        infile,
        outfile,
        size
    );

    lzma_end(&strm);

out:
    if (infile){
        fclose(infile);
        infile = NULL;
    }

    if (outfile){
        fclose(outfile);
        outfile = NULL;
    }

    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void Section::setPercent(const int &a_percent)
{
    m_percent = a_percent;
}

int Section::getPercent()
{
    return m_percent;
}

void Section::clearPercent()
{
    m_percent = 0;
}

