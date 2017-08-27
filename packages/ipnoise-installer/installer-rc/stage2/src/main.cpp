#include <QtCore/QTranslator>

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <getopt.h>
#include <time.h>
#include "mainWindow.hpp"
#include "main.hpp"

#include "ipnoise-packer/packer/src/packer.hpp"

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

int noGui(Config *a_conf)
{
    int res, err = 0;

    char buffer[65535] = { 0x00 };
    getcwd(buffer, sizeof(buffer));

    QString install_dir = (*a_conf)["install_dir"];
    QString setup_dir   = buffer;
    QString self_name   = (*a_conf)["self_name"];

    if (install_dir.size()){
        res = QDir::setCurrent(install_dir);
        if (not res){
            PERROR("Cannot change dir to: '%s'\n",
                install_dir.toStdString().c_str()
            );
            goto fail;
        }
    }

    if ("install" == (*a_conf)["action"]){
        Packer packer;
        QString in_file = setup_dir + "/" + self_name;
        res = packer.parse(in_file.toStdString());
        if (res){
            PERROR("Cannot parse file: '%s'\n",
                in_file.toStdString().c_str()
            );
            goto fail;
        }
        res = packer.saveAll(1);
        if (res){
            PERROR("Unpack failed\n");
            goto fail;
        }
    }

    // all ok
    err = 0;

out:
    res = QDir::setCurrent(buffer);
    if (not res){
        PERROR("Cannot change dir to: '%s'\n", buffer);
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int main(int argc, char **argv)
{
#if defined(WIN32) || defined(WIN64)
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

    // init locale
    // never use QString before locale will be inited!!!
    QLocale locale = QLocale("ru");
    QLocale::setDefault(locale);

    //QTextCodec* tc = QTextCodec::codecForName("utf-8");
    //QTextCodec::setCodecForTr(tc);
    //QTextCodec::setCodecForCStrings(tc);
    //QTextCodec::setCodecForLocale(tc);

    int res, err = -1;
    char            buffer[1024]    = { 0x00 };
    QApplication    *app            = NULL;
    WizardInstall   *mainWindow     = NULL;
    Config          *config         = new Config;
    QString         action          = "";
    QString         install_dir     = "";
    bool            console         = false;
    QTranslator     tr;

    if (tr.load("ru.qm")){
        app->installTranslator(&tr);
    }

    // default install dir
    Path cur_dir    = Path::getCwd();
    Path setup_dir  = cur_dir.getDirPath();
    Path stage1;

    install_dir  = setup_dir.path().c_str();
    stage1       = setup_dir;
#if defined(WIN32) || defined(WIN64)
    stage1      += "setup.exe";
#else
    stage1      += "setup";
#endif

    // init rand
    srand(time(NULL));

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"action",      1, 0, 'a'},
            {"dir",         1, 0, '0'},
            {"console",     1, 0, 'c'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "ha:0:cvd:",
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

            case 'a':
                if (optarg){
                    action = optarg;
                }
                break;

            case 'c':
                console = true;
                break;

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

    if (g_debug_level){
        PINFO("Options:\n");
        PINFO("action:              '%s'\n",
            action.toStdString().c_str());
        PINFO("installation dir:    '%s'\n",
            install_dir.toStdString().c_str());
        PINFO("stage1:              '%s'\n",
            stage1.path().c_str());
        PINFO("console:             '%d'\n",
            console);
        PINFO("debug-level:         '%d'\n",
            g_debug_level);
        PINFO("\n");
    }

    // update config
    (*config)["install_dir"]    = install_dir;
    (*config)["stage1"]         = stage1.path().c_str();
    (*config)["action"]         = action;
    (*config)["self_name"]      = argv[0];

    if (    "install" == action
        &&  not install_dir.size())
    {
        PERROR("missing installation directory (--dir)\n");
        goto fail;
    }

    if (console){
        // process console
        err = noGui(config);
    } else {
        // process GUI
        app         = new QApplication(argc, argv);
        mainWindow  = new WizardInstall(config);  //new MainWindow();
        // mainWindow->init(app, config);
        mainWindow->show();
        //QLabel *label = new QLabel(cur_dir.path().c_str());
        //label->show();
        err = app->exec();
    }

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

