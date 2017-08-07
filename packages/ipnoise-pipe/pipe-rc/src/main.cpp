#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <getopt.h>
#include <time.h>
#include "mainWindow.hpp"
#include "main.hpp"

int debug_level = 0;

void usage(int argc, char **argv)
{
    PINFO("Usage:     %s [-options]                     \n"
        "options:                                       \n"
        "         -h, --help        This help           \n"
        "         -a, --address     Address for connect \n"
        "         -v, --version     Program version     \n"
        "         --debug <level>   Debug level         \n"
        "\n",
        argv[0]
    );
}

int main(int argc, char **argv)
{
#if defined(WIN32) || defined(WIN64)
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

    int i, res, err = -1;
    char         buffer[1024] = { 0x00 };
    QApplication *app         = NULL;
    MainWindow   *mainWindow  = NULL;
    Config       *config      = NULL;
    QString      address      = "127.0.0.1:2210";

    Splitter    app_path(argv[0], "/");
    QString     appdir;

    app = new QApplication(argc, argv);
    app->setApplicationName(QLatin1String(APPNAME));

    config = new Config;

    for (i = 0; i < int(app_path.size() - 1); i++){
        if (    not app_path[i].size()
            ||  (appdir.size() && "/" != appdir))
        {
            appdir += "/";
        }
        appdir += app_path[i].c_str();
    }

    if (not appdir.size()){
        appdir = QDir::currentPath();
    }

    // init rand
    srand(time(NULL));

    // init locale
    QTextCodec* tc = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForTr(tc);
    QTextCodec::setCodecForCStrings(tc);
    QTextCodec::setCodecForLocale(tc);

    mainWindow = new MainWindow();

    res = QDir::setCurrent(appdir);
    if (!res){
        QString err = "Cannot change directory to ";
        err += "'";
        err += appdir;
        err += "'";

        qFatal(err.toStdString().c_str());
        goto fail;
    }

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"address",     1, 0, 'a'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "ha:vd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 0:
                break;

            case 'h':
                usage(argc, argv);
                exit(0);

            case 'a':
                address = optarg;
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

    if (debug_level){
        PINFO("Options:\n");
        PINFO("address:     '%s'\n",
            address.toStdString().c_str());
        PINFO("debug-level: '%d'\n",
            debug_level);
        PINFO("\n");
    }

    if (not address.size()){
        PERROR("address empty\n");
        goto fail;
    }

    // update config
    (*config)["address"] = address;

    // init main window
    mainWindow->init(app, config);
    mainWindow->show();

    err = app->exec();

out:
    if (mainWindow){
        delete mainWindow;
        mainWindow = NULL;
    }
    if (app){
        delete app;
        app = NULL;
    }
    if (config){
        delete config;
        config = NULL;
    }
    return err;

fail:
    goto out;
}

