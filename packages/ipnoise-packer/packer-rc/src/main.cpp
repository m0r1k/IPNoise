#include "main.hpp"

int debug_level = 0;

void usage(int argc, char **argv)
{
    PINFO("Usage:     %s [-options]                               \n"
        "   options:                                              \n"
        "       -h, --help              This help                 \n"
        "       -a, --action <action>   add                       \n"
        "                               |show                     \n"
        "                               |del                      \n"
        "                               |save|saveall             \n"
        "                               |unpack|unpackall         \n"
        "       -i, --in-file <file>    input arch for modify     \n"
        "       -o, --out-file <file>   output arch for create    \n"
        "       -p, --path <file|dir>   path for action           \n"
        "       -v, --version           program version           \n"
        "       --debug <level>         debug level               \n"
        "\n",
        argv[0]
    );
}

int processAddFile(
    Config          *a_config,
    Packer          *a_packer,
    const Path      &a_path)
{
    string out_file = (*a_config)["out_file"];

    PDEBUG(5, "processAddFile('%s')\n",
        a_path.path().c_str());

    // add file to archive
    a_packer->addFile(a_path);

    // request build arch
    return a_packer->build(out_file);
}

int processShow(
    Config          *a_config,
    Packer          *a_packer)
{
    int     err = 0;
    string  res;

    res = a_packer->show();
    PINFO("%s\n", res.c_str());

    return err;
}

int addDir(
    Config          *a_config,
    Packer          *a_packer,
    const Path      &a_path)
{
    int res, err = -1;

    if (not a_path.isExist()){
        PERROR("cannot read path: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    if (not a_path.isDir()){
        PERROR("it is not directory: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    // add dir recursive
    res = a_packer->addDirRecursive(a_path);
    if (res){
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

int processAddDir(
    Config          *a_config,
    Packer          *a_packer,
    const Path      &a_path)
{
    int res, err = -1;
    Path    out_file;

    out_file = (*a_config)["out_file"];
    if (out_file.empty()){
        out_file = (*a_config)["in_file"];
    }

    if (out_file.empty()){
        PERROR("unknown destination arch,"
            " please specify (-i, --in-file)"
            " or (-o, --out-file)\n"
        );
        goto fail;
    }

    if (a_path.empty()){
        PERROR("Empty directory name\n");
        goto fail;
    }

    if (not a_path.isExist()){
        PERROR("cannot read path: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    if (not a_path.isDir()){
        PERROR("path is not directory: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    res = addDir(a_config, a_packer, a_path);
    if (res){
        PERROR("Failed to process dir: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    // create archive
    err = a_packer->build(out_file);

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int processAdd(
    Config      *a_config,
    Packer      *a_packer,
    const Path  &a_path)
{
    int err = -1;

    PDEBUG(5, "processAdd, path: '%s'\n",
        a_path.path().c_str()
    );

    // path
    if (not a_path.isExist()){
        PERROR("cannot read path: '%s'\n",
            a_path.path().c_str()
        );
        goto fail;
    }

    // check
    if (a_path.isFile()){
        processAddFile(a_config, a_packer, a_path);
    } else if (a_path.isDir()){
        processAddDir(a_config, a_packer, a_path);
    } else {
        PERROR("Cannot pack '%s' unsupported type\n",
           a_path.path().c_str()
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = 0;
    }
    goto out;
}

int processDel(
    Config  *,  // a_config,
    Packer  *)  // a_packer
{
    int err = 0;
    return err;
}

int processSaveAll(
    Config      *,              // a_config
    Packer      *a_packer,
    const int   &a_uncompress)
{
    int err = -1;

    // save all
    err = a_packer->saveAll(a_uncompress);

    return err;
}

int processSave(
    Config          *a_config,
    Packer          *a_packer,
    const Path      &a_path,
    const int       &a_uncompress)
{
    int     err      = -1;
    Section *section = NULL;
    Path    path;
    Path    out_file;

    // get path for extract
    path.setPath((*a_config)["path"]);

    // get path for extract
    out_file.setPath((*a_config)["out_file"]);
    if (out_file.empty()){
        out_file = path;
    }

    if (not a_path.empty()){
        // search section by path
        section = a_packer->getSectionByPath(path);
        if (not section){
            PERROR("path: '%s' not found\n",
                path.path().c_str()
            );
            goto fail;
        }

        // save section data
        section->save(out_file, a_uncompress);
    } else {
        // save all
        a_packer->saveAll(a_uncompress);
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

int main(int argc, char **argv)
{
    int res, err = -1;
    char    buffer[1024]    = { 0x00 };
    string  action          = "";
    Config  *config         = new Config;
    Packer  *packer         = new Packer;
    Path    in_file;
    Path    out_file;
    Path    l_path;

    // init rand
    srand(time(NULL));

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"action",      1, 0, 'a'},
            {"path",        1, 0, 'p'},
            {"in-file",     1, 0, 'i'},
            {"out-file",    1, 0, 'o'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "ha:p:i:o:cvd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 'p':
                l_path.setPath(optarg);
                break;

            case 'h':
                usage(argc, argv);
                exit(0);

            case 'a':
                action = optarg;
                break;

            case 'i':
                in_file.setPath(optarg);
                break;

            case 'o':
                out_file.setPath(optarg);
                break;

            case 'd':
                debug_level = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n", VERSION);
                exit(0);

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        char *buffer_ptr = buffer;
        int res;
        res = snprintf(
            buffer_ptr,
            (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: "
        );
        if (res > 0){
            buffer_ptr += res;
        }
        while (optind < argc){
            snprintf(
                buffer_ptr,
                (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ", argv[optind++]);
        }
        PERROR("%s\n", buffer);
    }

    // if out file not defined, some file will used
    if (out_file.empty() && not in_file.empty()){
        out_file = in_file;
    }

    if (debug_level){
        PINFO("Options:\n");
        PINFO("action:          '%s'\n",    action.c_str());
        PINFO("path:            '%s'\n",    l_path.path().c_str());
        PINFO("in_file:         '%s'\n",    in_file.path().c_str());
        PINFO("out_file:        '%s'\n",    out_file.path().c_str());
        PINFO("debug-level:     '%d'\n",    debug_level);
        PINFO("\n");
    }

    // load packer
    if (not in_file.empty()){
        res = packer->parse(in_file);
        if (res){
            PERROR("Cannot parse file: '%s'\n",
                l_path.path().c_str()
            );
            goto fail;
        }
    }

    // update config
    (*config)["self_name"]      = argv[0];
    (*config)["action"]         = action;
    (*config)["path"]           = l_path.path();
    (*config)["in_file"]        = in_file.path();
    (*config)["out_file"]       = out_file.path();

    if (not action.size()){
        err = 0;
        goto out;
    }

    if ("add" == action){
        err = 0;
        if (l_path.empty()){
            PERROR("missing path (--path)\n");
            err = -1;
        }
        if (    in_file.empty()
            &&  out_file.empty())
        {
            PERROR("in (--in-file) or out (--out-file)"
                " must be specified\n");
            err = -2;
        }
        if (err){
            goto fail;
        }
        err = processAdd(config, packer, l_path);
    } else if ("show" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (err){
            goto fail;
        }
        err = processShow(config, packer);
    } else if ("del" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (l_path.empty()){
            PERROR("missing path (--path)\n");
            err = -2;
        }
        if (err){
            goto fail;
        }
        err = processDel(config, packer);
    } else if ("save" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (out_file.empty()){
            PERROR("missing out-file (--out-file)\n");
            err = -2;
        }
        if (l_path.empty()){
            PERROR("missing path (--path)\n");
            err = -3;
        }
        if (err){
            goto fail;
        }
        err = processSave(config, packer, l_path, 0);
    } else if ("saveall" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (err){
            goto fail;
        }
        err = processSaveAll(config, packer, 0);
    } else if ("unpack" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (out_file.empty()){
            PERROR("missing out-file (--out-file)\n");
            err = -2;
        }
        if (l_path.empty()){
            PERROR("missing path (--path)\n");
            err = -3;
        }
        if (err){
            goto fail;
        }
        err = processSave(config, packer, l_path, 1);
    } else if ("unpackall" == action){
        err = 0;
        if (in_file.empty()){
            PERROR("missing in-file (--in-file)\n");
            err = -1;
        }
        if (err){
            goto fail;
        }
        err = processSaveAll(config, packer, 1);
    } else {
        PERROR("Unknown action: '%s'\n", action.c_str());
        goto fail;
    }

out:
    if (config){
        delete config;
        config = NULL;
    }
    return err;

fail:
    goto out;
}

