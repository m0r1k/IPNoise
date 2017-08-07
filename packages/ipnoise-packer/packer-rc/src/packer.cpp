#include "packer.hpp"

Packer::Packer()
{
    m_sections = new Sections;

    m_compress_percent_cb       = NULL;
    m_compress_percent_cb_ctx   = NULL;
    m_decompress_percent_cb     = NULL;
    m_decompress_percent_cb_ctx = NULL;

#ifdef COMPRESS_MULTITHREADS
    m_sections_for_threads = new Sections;
    memset(&m_mutex, 0x00, sizeof(m_mutex));
#endif
}

Packer::~Packer()
{
    if (m_sections){
        delete m_sections;
        m_sections = NULL;
    }
#ifdef COMPRESS_MULTITHREADS
    if (m_sections_for_threads){
        delete m_sections_for_threads;
        m_sections_for_threads = NULL;
    }
#endif
}

string Packer::show()
{
    string ret;
    SectionsIt sections_it;

    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section  *section = *sections_it;
        ret += section->debug();
    }

    return ret;
}

#ifdef COMPRESS_MULTITHREADS
void * Packer::compressMultiThread(void *a_ctx)
{
    int         res;
    void        *retval     = NULL;
    Packer      *packer     = (Packer *)a_ctx;
    Section     *section    = NULL;
    SectionsIt  sections_it;

again:
    // clear
    section = NULL;

    // lock packer
    res = pthread_mutex_lock(&packer->m_mutex);
    if (res){
        PERROR("pthread_mutex_lock failed\n");
        goto fail;
    }

    // search section
    if (packer->m_sections_for_threads->size()){
        sections_it = packer->m_sections_for_threads->begin();
        section     = *sections_it;
        packer->m_sections_for_threads->erase(sections_it);
    }

    // unlock packer
    res = pthread_mutex_unlock(&packer->m_mutex);
    if (res){
        PERROR("pthread_mutex_unlock failed\n");
        goto fail;
    }

    if (section){
        Path          content_file;
        Path          out_file;
        Path          out_file_tmp;
        ssize_t       content_size  = -1;
        ssize_t       content_pos   = 0;
        Path          path          = section->getPath();
        SectionHeader *header       = NULL;
        int           need_compress = 0;
        FILE          *fin          = NULL;
        FILE          *fout         = NULL;
        string        debug;

        header = section->getCreateHeader();
        section->getContentFile(content_file);
        section->getContentSize(content_size);
        section->getContentPos(content_pos);

        if ("dir" == section->getType()){
            // skip section
            goto again;
        }

        if ("boot" == section->getType()){
            // skip section
            goto again;
        }

        if (section->isCompressed()){
            // skip section
            goto again;
        }

        // show debug
        debug = section->debug();
        PDEBUG(5, "compress section:\n'%s'\n",
            debug.c_str());

        // open content file
        fin = content_file.fopen("rb");
        if (not fin){
            PERROR("Cannot open file: '%s' for read\n",
                content_file.path().c_str()
            );
            goto fail;
        }

        // go to content
        if (content_pos > 0){
            if (fseek(fin, content_pos, SEEK_SET)){
                PERROR("Cannot seek to end of file: '%s'\n",
                    content_file.path().c_str()
                );
                goto fail;
            }
        }

        PDEBUG(5, "seek on '%d' byte(s) to content,"
            " file: '%s'\n",
            content_pos,
            content_file.path().c_str()
        );

        // open output file
        out_file_tmp    = content_file.path() + ".7z.tmp";
        out_file        = content_file.path() + ".7z";
        fout            = out_file_tmp.fopen("wb");
        if (not fout){
            fclose(fin); fin = NULL;
            PERROR("Cannot open file for write: '%s'\n",
                out_file_tmp.path().c_str()
            );
            goto fail;
        }

        PDEBUG(5, "before compress:\n"
            " ftell(fout):  '%ld'\n"
            " ftell(in):    '%ld'\n",
            ftell(fout),
            ftell(fin)
        );

        res = section->compress(
            content_file,
            fin,
            fout,
            content_size
        );

        PDEBUG(5, "after compress:\n"
            " ftell(fout):  '%ld'\n"
            " ftell(in):    '%ld'\n",
            ftell(fout),
            ftell(fin)
        );

        if (not res){
            // close fds
            fclose(fin);  fin  = NULL;
            fclose(fout); fout = NULL;
            // rename tmp file
            rename(
                out_file_tmp.path().c_str(),
                out_file.path().c_str()
            );
            // remove content file
            remove(content_file.path().c_str());
            // update section info
            section->setContentFile(out_file.path());
            section->setContentPos(0);
            header->compress_type = COMPRESS_TYPE_LZMA;

            // show debug
            debug = section->debug();
            PDEBUG(5, "after compress section:\n'%s'\n",
                debug.c_str());

        } else {
            fclose(fin);  fin  = NULL;
            fclose(fout); fout = NULL;
            PERROR("Failed process file: '%s'\n",
                content_file.path().c_str());
            goto fail;
        }

        // get next section
        goto again;
    }

out:
    return retval;

fail:
    goto out;
}

int Packer::compressMultiThreads()
{
    int         res, i, err = 0;
    Threads     threads;
    ThreadsIt   threads_it;

    // copy current sections list
    *m_sections_for_threads = *m_sections;

    // create threads
    for (i = 0; i < MAX_PACKER_THREADS; i++){
        Thread *thread = new Thread;
        thread->setFunc(Packer::compressMultiThread);
        res = thread->start(this);
        if (res){
            PERROR("pthread_create failed\n");
            err = -1;
            break;
        }
        // store thread
        threads.push_back(thread);
    }

    // wait when threads done work
    while (threads.size()){
        threads_it = threads.begin();
        (*threads_it)->wait();
        delete (*threads_it);
        threads.erase(threads_it);
    }

    return err;
}
#endif

int Packer::build(
    const Path &a_out_file)
{
    int         res, err    = -1;
    FILE        *fout       = NULL;
    Path        out_file    = a_out_file.path() + ".tmp";
    Sections    sections;
    SectionsIt  sections_it;

    // clear current progress
    clearPercents();

#ifdef COMPRESS_MULTITHREADS
    // compress via multithreads
    compressMultiThreads();
#endif

    // open output file
    fout = out_file.fopen("wb");
    if (not fout){
        PERROR("Cannot open file for write: '%s'\n",
            out_file.path().c_str()
        );
        goto fail;
    }

    // first of all 'boot' section
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *section = *sections_it;
        if ("boot" != section->getType()){
            continue;
        }
        sections.push_back(section);
        break;
    }

    // then dirs
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *section = *sections_it;
        if ("dir" != section->getType()){
            continue;
        }
        sections.push_back(section);
    }

    // then all others
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *section = *sections_it;
        if (    "dir"   == section->getType()
            ||  "boot"  == section->getType())
        {
            continue;
        }
        sections.push_back(section);
    }

    for (sections_it = sections.begin();
        sections_it != sections.end();
        sections_it++)
    {
        Section         *section        = *sections_it;
        SectionHeader   *header         = section->getCreateHeader();
        ssize_t         out_fpos_before = ssize_t(-1);
        ssize_t         out_fpos_after  = ssize_t(-1);
        string          vars;
        Path            content_file;
        ssize_t         content_size    = -1;
        ssize_t         content_pos     = 0;
        Path            path            = section->getPath();
        string          debug;

        section->getContentFile(content_file);
        section->getContentSize(content_size);
        section->getContentPos(content_pos);

        debug = section->debug();

        // get current output pos
        out_fpos_before = ftell(fout);;
        out_fpos_after  = out_fpos_before;

        PDEBUG(5, "build section:\n'%s'\n", debug.c_str());

        // if content file exist, add content from file
        if ("dir" != section->getType()){
            FILE    *fin            = NULL;
            int     need_compress   = 0;

            // open content file
            fin = content_file.fopen("rb");
            if (not fin){
                PERROR("Cannot open file: '%s' for read\n",
                    content_file.path().c_str()
                );
                goto fail;
            }

            // go to content
            if (content_pos > 0){
                if (fseek(fin, content_pos, SEEK_SET)){
                    fclose(fin);
                    PERROR("Cannot seek"
                        " to end of file: '%s'\n",
                        content_file.path().c_str()
                    );
                    goto fail;
                }
            }

            PDEBUG(5, "seek on '%d' byte(s) to content,"
                " file: '%s'\n",
                content_pos,
                content_file.path().c_str()
            );

            if (not section->isCompressed()){
                need_compress = 1;
            }

            if ("boot" == section->getType()){
                need_compress = 0;
            }

            if (need_compress){
                PWARN("compress, slow path\n");

                PDEBUG(5, "before compress:\n"
                    " ftell(fout):  '%ld'\n"
                    " ftell(in):    '%ld'\n",
                    ftell(fout),
                    ftell(fin)
                );

                res = section->compress(
                    content_file,
                    fin,
                    fout,
                    content_size
                );

                PDEBUG(5, "after compress:\n"
                    " ftell(fout):  '%ld'\n"
                    " ftell(in):    '%ld'\n",
                    ftell(fout),
                    ftell(fin)
                );

                if (!res){
                    // setup compress type
                    header->compress_type = COMPRESS_TYPE_LZMA;
                } else {
                    fclose(fin);
                    PERROR("Failed process file: '%s'\n",
                        content_file.path().c_str());
                    goto fail;
                }
            } else {
                int     buffer_size = 1*1024*1024; // Mb
                uint8_t *buffer     = new uint8_t[buffer_size];
                ssize_t need_read   = content_size;

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
                        buffer_size,
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
                        res, a_out_file.path().c_str()
                    );
                } while (res > 0);

                delete [] buffer;
            }
            fclose(fin);
        }

        // get output position
        out_fpos_after          = ftell(fout);
        header->content_size    = out_fpos_after - out_fpos_before;
        section->setContentSize(header->content_size);

        // write section vars
        section->serializeVars(vars);
        if (vars.size()){
            fwrite(vars.c_str(), 1, vars.size(), fout);
        }
        header->vars_size   = vars.size();
        header->vars_offset = sizeof(*header) + header->vars_size;

        // calculate content offset
        header->content_offset = header->vars_offset + header->content_size;

        // write section header
        fwrite(header, 1, sizeof(*header), fout);

        debug = section->debug();
    }

    // rename tmp file
    rename(out_file.path().c_str(), a_out_file.path().c_str());

    // all ok
    err = 0;

out:
    if (fout){
        fclose(fout);
        fout = NULL;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void Packer::deleteSection(
    Section *a_section)
{
    SectionsIt  sections_it;

    if (not a_section){
        goto out;
    }

    if (not m_sections){
        goto out;
    }

again:
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *cur_section = *sections_it;
        if (cur_section == a_section){
            m_sections->erase(sections_it);
            delete cur_section;
            goto again;
        }
    }

out:
    return;
}

Section * Packer::addFile(
    const Path      &a_file,
    const ssize_t   &a_size)
{
    int             res;
    struct stat     st;
    Section         *section    = NULL;
    SectionHeader   *header     = NULL;
    Path            file        = a_file;
    string          type        = "file";
    string          perm        = "";

    PDEBUG(5, "Packer::addfile('%s')\n", a_file.path().c_str());

    res = stat(
        a_file.path().c_str(),
        &st
    );
    if (res){
        PERROR("cannot read file: '%s'\n", a_file.path().c_str());
        goto fail;
    }

    // get file perm
    perm = Path::parseMode(st.st_mode);

    // remove first '/' from name
    file.setAbsolute(0);

    // may be already exist
    section = getSectionByPath(file);
    if (section){
        deleteSection(section);
        section = NULL;
    }

    // create new section
    section = new Section;
    if (not section){
        PERROR("Cannot allocate memory\n");
        goto fail;
    }

    // create header
    header = section->getCreateHeader();

    // setup magic
    header->magic = MAGIC;

    // setup content file for section
    section->setContentFile(a_file);
    section->setContentSize(a_size);
    section->setContentPos(0);

    // store vars
    if (not m_sections->size()){
        type = "boot";
    }
    section->setType(type);
    section->setPath(file);
    section->setPerm(perm);

    // store section
    m_sections->push_back(section);

out:
    return section;

fail:
    if (section){
        delete section;
        section = NULL;
    }
    goto out;
}

int Packer::addDirRecursive(
    const Path  &a_dir_path)
{
    int res,        err     = -1;
    DIR             *dir    = NULL;
    struct dirent   *dent   = NULL;

    // try open dir
    dir = opendir(a_dir_path.path().c_str());
    if (not dir){
        PERROR("Cannot open directory: '%s'\n",
            a_dir_path.path().c_str()
        );
        goto fail;
    }

    // add dir to archive
    addDir(a_dir_path);

    do {
        dent = readdir(dir);
        if (not dent){
            break;
        }
        string dent_name = dent->d_name;
        if (    "."     == dent_name
            ||  ".."    == dent_name)
        {
            continue;
        }

        Path path(a_dir_path);
        path += dent_name;

        if (not path.isExist()){
            PERROR("cannot read path: '%s'\n", path.path().c_str());
            goto fail;
        }
        if (path.isDir()){
            // process dir
            res = addDirRecursive(path);
            if (res){
                PERROR("Cannot add dir: '%s'\n",
                    path.path().c_str()
                );
                goto fail;
            }
        } else {
            // add file to archive
            addFile(path);
        }
    } while (dent);

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

Section * Packer::addDir(
    const Path  &a_dir_path)
{
    Section         *section    = NULL;
    SectionHeader   *header     = NULL;
    Path            dir_path    = a_dir_path;
    string          perm        = "";

    PDEBUG(5, "Packer::addDir('%s')\n", a_dir_path.path().c_str());

    if (not a_dir_path.isExist()){
        PERROR("cannot read path: '%s'\n", a_dir_path.path().c_str());
        goto fail;
    }

    if (not a_dir_path.isDir()){
        PERROR("path: '%s' is not directory\n",
            a_dir_path.path().c_str());
        goto fail;
    }

    // get dir perm
    perm = a_dir_path.getPermStr();

    // remove first '/' from name
    dir_path.setAbsolute(0);

    if (dir_path.empty()){
        PERROR("Cannot add dir: '%s'\n", a_dir_path.path().c_str());
        goto fail;
    }

    // may be already exist
    section = getSectionByPath(dir_path);
    if (section){
        deleteSection(section);
        section = NULL;
    }

    // create new section
    section = new Section;
    if (not section){
        PERROR("Cannot allocate memory\n");
        goto fail;
    }

    // create header
    header = section->getCreateHeader();

    // setup magic
    header->magic = MAGIC;

    // store vars
    section->setType("dir");
    section->setPath(dir_path);
    section->setPerm(perm);

    // store section
    m_sections->push_back(section);

out:
    return section;

fail:
    if (section){
        delete section;
        section = NULL;
    }
    goto out;
}

void Packer::clearPercents()
{
    SectionsIt  sections_it;

    if (m_sections){
        for (sections_it = m_sections->begin();
            sections_it != m_sections->end();
            sections_it++)
        {
            Section *cur_section = *sections_it;
            cur_section->clearPercent();
        }
    }
}

void Packer::setCompressPercentCb(
    void (* a_compress_percent_cb)(
        Packer *,
        int,
        const Path  &,
        int,
        void *
    ),
    void *a_ctx)
{
    m_compress_percent_cb       = a_compress_percent_cb;
    m_compress_percent_cb_ctx   = a_ctx;
}

void Packer::setDecompressPercentCb(
    void (* a_decompress_percent_cb)(
        Packer *,
        int,
        const Path  &,
        int,
        void *
    ),
    void *a_ctx)
{
    m_decompress_percent_cb       = a_decompress_percent_cb;
    m_decompress_percent_cb_ctx   = a_ctx;
}

void Packer::deCompressSectionPercentCb(
    Section     *a_section,
    const Path  &a_file,
    int         a_percent,
    void        *a_ctx)
{
    Packer      *packer = (Packer *)a_ctx;
    SectionsIt  sections_it;

    int total_percent = 0;

    if (packer->m_sections){
        for (sections_it = packer->m_sections->begin();
            sections_it != packer->m_sections->end();
            sections_it++)
        {
            Section *cur_section = *sections_it;
            total_percent += cur_section->getPercent();
        }
    }

    if (packer->m_decompress_percent_cb){
        packer->m_decompress_percent_cb(
            packer,
            total_percent,
            a_section->getPath(),
            a_percent,
            packer->m_decompress_percent_cb_ctx
        );
    }
}

void Packer::compressSectionPercentCb(
    Section     *a_section,
    const Path  &a_file,
    int         a_percent,
    void        *a_ctx)
{
    Packer      *packer = (Packer *)a_ctx;
    SectionsIt  sections_it;

    int total_percent = 0;

    if (packer->m_sections){
        for (sections_it = packer->m_sections->begin();
            sections_it != packer->m_sections->end();
            sections_it++)
        {
            Section *cur_section = *sections_it;
            total_percent += cur_section->getPercent();
        }
    }

    if (packer->m_compress_percent_cb){
        packer->m_compress_percent_cb(
            packer,
            total_percent,
            a_section->getPath(),
            a_percent,
            packer->m_compress_percent_cb_ctx
        );
    }
}

void Packer::setDir(const Path &a_dir)
{
    m_dir = a_dir;
}

int Packer::parse(
    const Path    &a_file)
{
    int         err    = -1;
    FILE        *file  = NULL;
    SectionsIt  sections_it;

    // store file name
    m_file = a_file;

    // delete current sections info
    while (m_sections->size()){
        sections_it = m_sections->begin();
        delete *sections_it;
    }

    file = m_file.fopen("rb");
    if (not file){
        PERROR("Cannot open file: '%s' for read\n",
            m_file.path().c_str()
        );
        goto fail;
    }

    // goto end of file
    if (fseek(file, 0, SEEK_END)){
        PERROR("Cannot seek to end of file\n");
        goto fail;
    }

    // read sections
    do {
        Section *section = NULL;
        string  debug;
        section = section->parse(file);
        if (not section){
            break;
        }
        section->setContentFile(m_file);
        section->setDeCompressPercentCb(
            Packer::deCompressSectionPercentCb,
            (void *)this
        );
        section->setCompressPercentCb(
            Packer::compressSectionPercentCb,
            (void *)this
        );

        debug = section->debug();
        m_sections->push_back(section);
        PDEBUG(5, "was read section:\n%s\n",
            debug.c_str()
        );
    } while (1);

    // all ok
    err = 0;

out:
    PINFO("was read '%d' section(s)\n",
        m_sections->size());

    if (file){
        fclose(file);
        file = NULL;
    }

    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int Packer::getSectionsCount()
{
    int ret = 0;
    if (m_sections){
        ret = m_sections->size();
    }
    return ret;
}

Section * Packer::getSectionByPath(
    const Path &a_path)
{
    Section     *section = NULL;
    SectionsIt  sections_it;

    PDEBUG(5, "getSectionByPath, m_sections->size(): '%d'\n",
        m_sections->size());

    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Path        cur_path;
        Section     *cur_section    = *sections_it;
        string      cur_type        = cur_section->getType();

        PDEBUG(5, "cur_type: '%s'\n", cur_type.c_str());

        if (    "file"  != cur_type
            &&  "dir"   != cur_type)
        {
            continue;
        }

        cur_path = cur_section->getPath();
        PDEBUG(5, "cur_path: '%s'\n", cur_path.path().c_str());
        if (a_path.pathUnix() == cur_path.pathUnix()){
            section = cur_section;
            break;
        }
    }

    PDEBUG(5, "Packer::getSectionByPath,"
        " path: '%s', section: 0x'%x'\n",
        a_path.path().c_str(),
        (unsigned int)section
    );
    return section;
}

int Packer::save(
    const Path      &a_path,        // file path in unix format
    const int       &a_uncompress)
{
    int res, err = -1;
    Section *section = NULL;

    // clear current progress
    clearPercents();

    // search section
    section = getSectionByPath(a_path);
    if (not section){
        goto fail;
    }

    PDEBUG(5, "packer save path: '%s'\n",
        a_path.path().c_str());

    // set section's dir
    section->setDir(m_dir);

    // create dir
    {
        Path dir;
        dir.setPath(a_path.getDirPath());
        dir.mkdirr(0755);
    }

    // save
    res = section->save(a_path, 1);
    if (res){
        PERROR("Cannot save: '%s'\n", a_path.path().c_str());
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

int Packer::saveAll(
    const int &a_uncompress)
{
    int res, err = -1;

    SectionsIt sections_it;

    // clear current progress
    clearPercents();

    // create all dirs
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *section = *sections_it;
        if ("dir" != section->getType()){
            continue;
        }
        res = section->save(Path(), a_uncompress);
        if (res){
            PERROR("Cannot save: '%s'\n",
                section->getPath().path().c_str()
            );
            goto fail;
        }
    }

    // create all files
    for (sections_it = m_sections->begin();
        sections_it != m_sections->end();
        sections_it++)
    {
        Section *section = *sections_it;

        if (    "dir"   == section->getType()
            ||  "boot"  == section->getType())
        {
            continue;
        }
        res = section->save(Path(), a_uncompress);
        if (res){
            PERROR("Cannot save: '%s'\n",
                section->getPath().path().c_str()
            );
            goto fail;
        }
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

