#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// stl
#include <string>

#include "ipnoise-common/log_common.h"

using namespace std;

inline int my_system(
    const char * const  *argv,
    string              &std_out,
    string              &std_err)
{
    char buffer[1024]   = { 0x00 };
    int i, res, err     = -1;
    pid_t child_pid     = 0;

    int stdout_fds[2];
    int stderr_fds[2];

    const char * const  *_argv  = argv;
    std_out                     = "";
    std_err                     = "";
    string cmd                  = "";

    // join all arguments in one line
    while (*_argv != NULL){
        cmd += *_argv;
        cmd += " ";
        _argv++;
    }

    // remove last space
    cmd = cmd.substr(0, cmd.size() - 1);

    for (i = 0; i < 2; i++){
        // create pipe for stdout
        res = pipe(stdout_fds);
        if (!res){
            break;
        }
    }
    if (res){
        PERROR("cannot create pipe while execute command: '%s'\n",
            cmd.c_str());
        goto fail;
    }

    for (i = 0; i < 2; i++){
        // create pipe for stderr
        res = pipe(stderr_fds);
        if (!res){
            break;
        }
    }
    if (res){
        PERROR("cannot create pipe while execute command: '%s'\n",
            cmd.c_str());
        goto fail;
    }

    // fork now
    child_pid = fork();
    if (!child_pid){
        // this is child, replace stdout and stderr
        dup2(stdout_fds[1], 1);
        dup2(stderr_fds[1], 2);
        // replace process
        // XXX HACK (arguments is "const char *", not is "char *")
        execv(argv[0], (char * const *)argv);
        // this "_exit" will called only if execv failed
        _exit (1);
    } else if (child_pid > 0){
        // this is parent, wait when child have done work
        waitpid(child_pid, &err, 0);
        do {
            // read stdout
            fcntl(stdout_fds[0], F_SETFL, O_NONBLOCK);
            res = read(stdout_fds[0], buffer, sizeof(buffer) - 1);
            if (res > 0){
                buffer[res] = 0x00;
                std_out += buffer;
            }
        } while (res > 0);
        do {
            // read stderr
            fcntl(stderr_fds[0], F_SETFL, O_NONBLOCK);
            res = read(stderr_fds[0], buffer, sizeof(buffer) - 1);
            if (res > 0){
                buffer[res] = 0x00;
                std_err += buffer;
            }
        } while (res > 0);
    } else {
        // fork failed
        PERROR("fork failed\n");
        goto fail;
    }

out:
    if (stdout_fds[0] > 0){
        close (stdout_fds[0]);
    }
    if (stdout_fds[1] > 0){
        close (stdout_fds[1]);
    }
    if (stderr_fds[0] > 0){
        close (stderr_fds[0]);
    }
    if (stderr_fds[1] > 0){
        close (stderr_fds[1]);
    }

    if (err != 0){
        // debug
        string msg = "";
        int i = 1;

        msg += "Executing: '";
        msg += argv[0];
        while (argv[i] != NULL){
            msg += " ";
            msg += argv[i];
            i++;
        }
        msg += "'\n";
        snprintf(buffer, sizeof(buffer), "Exit code: '%d'\n", err);
        msg += buffer;
        msg += "Stdout: '" +std_out + "'\n";
        msg += "Stderr: '" +std_err + "'\n";

        // PERROR("%s\n", msg.c_str());
    }
    return err;

fail:
    if (!err){
        err = -1;
    }
    goto out;
}

/*

    Example:

    int main(void)
    {
        int res, err = 0;
        string std_out;
        string std_err;
        const char * const argv[] = {
            (char *)"/sbin/ip",
            (char *)"link",
            (char *)"show",
            (char *)"tap0",
            NULL
        };

        res = my_system(argv, std_out, std_err);
        fprintf(stderr,
            "exit_code: '%d'\n"
            "stdout: '%s'\n"
            "stderr: '%s'\n",
            res,
            std_out.c_str(),
            std_err.c_str()
        );

        return err;
    }

*/

#endif

