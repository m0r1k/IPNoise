#include "main.hpp"

int g_debug_level = 0;

void usage(int argc, char **argv)
{
    PINFO("Usage:     %s [-options]                             \n"
        "options:                                               \n"
        "         -h, --help            This help               \n"
        "         -c, --console         Disable GUI             \n"
        "         -a, --action <action> install                 \n"
        "             --dir <dir>       Installation directory  \n"
        "         -v, --version         Program version         \n"
        "         --debug <level>       Debug level             \n"
        "\n",
        argv[0]
    );
}

int process(Config *a_conf)
{
    int res, err = 0;
#if defined(WIN32) || defined(WIN64)
    const char *stage2 = "pipe/stage2.exe";
#else
    const char *stage2 = "pipe/stage2.sh";
#endif

    const char * files[] = {
#if defined(WIN32) || defined(WIN64)
        "pipe/libstdc++-6.dll",
        "pipe/libgcc_s_sjlj-1.dll",
        "pipe/QtGui4.dll",
        "pipe/QtCore4.dll",
        "pipe/QtNetwork4.dll",
        "pipe/zlib1.dll",
        "pipe/libpng15-15.dll",
        "pipe/liblzma-5.dll",
#else
        "pipe/stage2",
#endif
        "pipe/resources/wizard_install_page_intro.png",
        "pipe/resources/wizard_install_page_install.png",
        "pipe/resources/wizard_install_page_select_install_dir.png",
        "pipe/resources/wizard_install_page_finish.png",
        stage2,
        NULL
    };
    const char **file = files;

    Path self_name = (*a_conf)["self_name"];

    Packer  packer;
    Path    in_file;

    // create archive path (in_file)
    in_file  = Path::getCwd();

    in_file += self_name.getBaseName();

    res = packer.parse(in_file);
    if (res){
        PERROR("Cannot parse file: '%s'\n",
            in_file.path().c_str()
        );
        goto fail;
    }

    while (*file){
        Path path;
        path.setPathUnix(*file);
        res = packer.save(path, 1);
        if (res){
            PERROR("Unpack '%s' failed\n",
                path.path().c_str()
            );
            goto fail;
        }
        file++;
    }

    // stage2
    {
        string  cmd;
        Path    file;
        file.setPathUnix(stage2);

        Path    dir = file.getDirPath();
        dir.cd();

#if defined(WIN32) || defined(WIN64)
        cmd = file.getBaseName();
#else
        cmd = "./" + file.getBaseName();
#endif

        system(cmd.c_str());
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
    int err = -1;
    char buffer[65535]  = { 0x00 };
    Config  *config     = new Config;
    string  install_dir = "";

#if defined(WIN32) || defined(WIN64)
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

    // init rand
    srand(time(NULL));

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"dir",         1, 0, '0'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "hvd:0:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case '0':
                install_dir = optarg;
                break;

            case 'h':
                usage(argc, argv);
                exit(0);

            case 'd':
                g_debug_level = atoi(optarg);
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

    if (install_dir.size()){
        chdir(install_dir.c_str());
    }

    if (g_debug_level){
        PINFO("Options:\n");
        PINFO("debug-level: '%d'\n", g_debug_level);
        PINFO("install-dir: '%s'\n", install_dir.c_str());
        PINFO("\n");
    }

    // update config
    (*config)["self_name"]   = argv[0];
    (*config)["install_dir"] = install_dir;

    err = process(config);

    if (config){
        delete config;
        config = NULL;
    }
    return err;
}

