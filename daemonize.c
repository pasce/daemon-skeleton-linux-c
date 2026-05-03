/*
 * daemonize.c
 * This example daemonizes a process, writes a few log messages,
 * sleeps 20 seconds and terminates afterwards.
 * This is an answer to the stackoverflow question:
 * https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149
 * Fork this code: https://github.com/pasce/daemon-skeleton-linux-c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>

int flag_reconf = 0;

void signal_handler(int sig){
    /*
    The waitpid() function suspends the current process until the child corresponding to the pid 
    passed in the argument terminates, or until the current process receives a termination signal.
    If the current process executes the waitpid() function and the child identified by the pid is in 
    a zombie state, then the function returns immediately and any resources in the child are freed.
    */
    int status;
    pid_t child_pid;
    if(sig == SIGCHLD){ //Handles automatic cleanup of subprocesses created with fork()
        // Clears ALL terminated children (cycle required if multiple signals arrive at once)
        while ( (child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0){

            if (WIFEXITED(status))
                syslog(LOG_INFO, "child %d exited normally with code %d", child_pid, WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                syslog(LOG_ERR, "child %d terminated by signal %d", child_pid, WTERMSIG(status));
            else if (WIFSTOPPED(status))
                syslog(LOG_WARNING, "child %d stopped by signal %d", child_pid, WSTOPSIG(status));
            else if (WIFCONTINUED(status))
                syslog(LOG_INFO, "child %d resumed with SIGCONT", child_pid);

        }

    } else if(sig == SIGHUP){   //Ensures that the program does not die if you close the terminal window.
        flag_reconf = 1;
    }

}

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    signal(SIGCHLD, signal_handler);
    signal(SIGHUP, signal_handler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}
/*
for testing: 
    1. make
    2. ./firstdaemon
    3. pgrep firstdaemon --> for pid
    4. enter with a signal: kill -SIGCHLD <result_of_pgrep> or -SIGHUP
    5. journalctl -t firstdaemon --> to see log
*/
int main()
{
    skeleton_daemon();

    while (1)
    {
        //for SIGHUP
        if(flag_reconf){
            flag_reconf = 0;
            closelog();
            openlog("firstdaemon", LOG_PID, LOG_DAEMON);
            syslog(LOG_INFO, "configuration reloaded");
        }

        //to test SIGCHLD
        pid_t child = fork();
        if (child < 0) {
            syslog(LOG_ERR, "fork error");
        } else if (child == 0) {
            
            sleep(20);
            exit(EXIT_SUCCESS);
        }

        //TODO: Insert daemon code here.
        syslog (LOG_NOTICE, "First daemon started.");
        sleep (20);
        //break;
    }

    syslog (LOG_NOTICE, "First daemon terminated.");
    closelog();

    return EXIT_SUCCESS;
}
