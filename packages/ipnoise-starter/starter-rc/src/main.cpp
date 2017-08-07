#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

#include "ipnoise-common/log.h"
#include "main.hpp"

using namespace std;

int debug_level = 0;

void addGenericQEMUOptions(string &a_cmd)
{
    a_cmd += " -L .";
    a_cmd += " -m 256M";
    a_cmd += " -hda ipnoise.img";
    a_cmd += " -kernel vmlinuz";
    a_cmd += " -net none";
    a_cmd += " -no-hpet";
    a_cmd += " -localtime";
    a_cmd += " -append"
        " \""
        " noscsi noapic nousb nofirewire noideraid"
        " noapic nohpet noagp nomce clock=acpi_pm" // acpi=off
        " ide0=noprobe"
        " root=/dev/sda1 ro selinux=0"
        " 2 rw"
        "\"";
    a_cmd += " -vnc none"; // -vnc 0.0.0.0:0
}

int win32()
{
    int     err = -1;
    int     res;
    string  cmd = "qemu.exe";

    struct stat st;

    res = stat(cmd.c_str(), &st);
    if (res){
        string errmsg;
        errmsg = cmd + " was not found in current directory";
        PERROR("%s\n", errmsg.c_str());
        goto fail;
    }

    // execute vnc
    // system("start tvnviewer.exe 127.0.0.1:0");

    // cmd += " -append \"console=ttyS0 console=tty0 root=/dev/sda1 rw 2\"";
    // cmd += " -vnc 0.0.0.0:0";

    // add generic options
    addGenericQEMUOptions(cmd);

    // ok, execute qemu
    PWARN("Executing: '%s'\n", cmd.c_str());
    err = system(cmd.c_str());

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int lin32()
{
    int     err = -1;
    int     res;
    string  cmd = "./qemu";
    string  cur_tty;

    struct stat st;

    res = stat(cmd.c_str(), &st);
    if (res){
        string errmsg;
        errmsg = cmd + " was not found in current directory";
        PERROR("%s\n", errmsg.c_str());
        goto fail;
    }

    // get cur tty name
    if (0){
        string std_out, std_err;
        string::size_type pos;

        const char * argv[] = {
            "/bin/tty",
            NULL
        };
        my_system(argv, std_out, std_err);

        if (not std_out.empty()){
            char last_symbol = std_out.at(std_out.size() - 1);
            if ('\n' == last_symbol){
                cur_tty = std_out.substr(0, std_out.size() - 1);
            } else {
                cur_tty = std_out;
            }
        }
    }

    // cmd += " -append \"console=ttyS0 root=/dev/sda1 rw 2\"";
    // cmd += " -vnc none";
    // if (not cur_tty.empty()){
    //    cmd += " -serial " + cur_tty;
    // }

    // add generic options
    addGenericQEMUOptions(cmd);

    // ok, execute qemu
    PWARN("MORIK Executing: '%s'\n", cmd.c_str());
    err = system(cmd.c_str());

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int arm32()
{
    int     err = -1;
    int     res;
    string  cmd = "./qemu";
    string  cur_tty;

    struct stat st;

    res = stat(cmd.c_str(), &st);
    if (res){
        string errmsg;
        errmsg = cmd + " was not found in current directory";
        PERROR("%s\n", errmsg.c_str());
        goto fail;
    }

    // get cur tty name
    if (0){
        string std_out, std_err;
        string::size_type pos;

        const char * argv[] = {
            "/bin/tty",
            NULL
        };
        my_system(argv, std_out, std_err);

        if (not std_out.empty()){
            char last_symbol = std_out.at(std_out.size() - 1);
            if ('\n' == last_symbol){
                cur_tty = std_out.substr(0, std_out.size() - 1);
            } else {
                cur_tty = std_out;
            }
        }
    }

    // cmd += " -append \"console=ttyS0 root=/dev/sda1 rw 2\"";
    // cmd += " -vnc none";
    // if (not cur_tty.empty()){
    //    cmd += " -serial " + cur_tty;
    // }

    // add generic options
    addGenericQEMUOptions(cmd);

    // ok, execute qemu
    PWARN("MORIK Executing: '%s'\n", cmd.c_str());
    err = system(cmd.c_str());

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int main(int argc, char *argv[])
{
    char buffer[1024];
    int res, err = 0;

    // get args
    while (1){
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "hvd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 0:
                break;

            case 'h':
                PINFO(
                    "Usage:     %s [-options]                     \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "         --debug <level>   Debug level       \n"
                    "\n",
                    argv[0]
                );
                exit(0);

            case 'd':
                debug_level = atoi(optarg);
                break;

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        char *buffer_ptr = buffer;
        int res;
        res = snprintf(buffer_ptr, (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: ");
        if (res > 0){
            buffer_ptr += res;
        }
        while (optind < argc){
            snprintf(buffer_ptr, (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ", argv[optind++]);
        }
        PERROR("%s\n", buffer);
    }

    if (debug_level){
        PINFO("Options:\n");
        PINFO("debug-level: '%d'\n", debug_level);
        PINFO("\n");
    }

#ifdef LIN32
    err = lin32();
#else
#ifdef WIN32
    err = win32();
#else
#ifdef ARM32
    err = arm32();
#else
#error Unsupported platform
#endif
#endif
#endif

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

