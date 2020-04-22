
* This is an answer to a question on stackoverflow: [__Creating a daemon in Linux__](https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149)
* Fork the skeleton code: [__Basic skeleton of a linux daemon written in C__](https://github.com/pasce/daemon-skeleton-linux-c)
* Read the article here: [How to create a c-style daemon](https://nullraum.net/how-to-create-a-daemon-in-c/)

# Basic skeleton of a linux daemon written in C

Daemons work in the background and (usually...) don't belong to a TTY that's why you can't use stdout/stderr in the way you probably want.
Usually a syslog daemon (_syslogd_) is used for logging messages to files (debug, error,...).

Besides that, there are a few _required steps_ to daemonize a process.

## Required Steps 

 - __fork__ off the parent process & let it terminate if forking was successful. -> Because the parent process has terminated, the child process now runs in the background.
 - __setsid__ - Create a new session. The calling process becomes the leader of the new session and the process group leader of the new process group. The process is now detached from its controlling terminal (CTTY).
 - __Catch signals__ - Ignore and/or handle signals.
 - __fork again__ & let the parent process terminate to ensure that you get rid of the session leading process. (Only session leaders may get a TTY again.)
 - __chdir__ - Change the working directory of the daemon.
 - __umask__ - Change the file mode mask according to the needs of the daemon.
 - __close__ - Close all open file descriptors that may be inherited from the parent process.



Look at this skeleton code that shows the basic steps:

```c
/*
* daemonize.c
* This example daemonizes a process, writes a few log messages,
* sleeps 20 seconds and terminates afterwards.
* This is an answer to the stackoverflow question:
* https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux/17955149#17955149
* Fork this code: https://github.com/pasce/daemon-skeleton-linux-c
* Read about it: https://nullraum.net/how-to-create-a-daemon-in-c/
*/
    
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
   
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
    /*TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
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
```  
```c
int main()
{
    skeleton_daemon();
    
    while (1)
    {
        //TODO: Insert daemon code here.
        syslog (LOG_NOTICE, "First daemon started.");
        sleep (20);
        break;
    }
   
    syslog (LOG_NOTICE, "First daemon terminated.");
    closelog();
    
    return EXIT_SUCCESS;
}
```
## Compile and run
 - Compile the code: `gcc -o firstdaemon daemonize.c`
 - Start the daemon: `./firstdaemon`
 - Check if everything is working properly: `ps -xj | grep firstdaemon`

## Test the output
 - The output should be similar to this one:
<pre>
+------+------+------+------+-----+-------+------+------+------+-----+
| PPID | PID  | PGID | SID  | TTY | TPGID | STAT | UID  | TIME | CMD |
+------+------+------+------+-----+-------+------+------+------+-----+
|    1 | 3387 | 3386 | 3386 | ?   |    -1 | S    | 1000 | 0:00 | ./  |
+------+------+------+------+-----+-------+------+------+------+-----+
</pre>

__What you should see here is:__

 - The daemon has no controlling terminal (__TTY = ?__)
 - The parent process ID (__PPID__) is __1__ (The init process)
 - The __PID != SID__ which means that our process is NOT the session leader<br>
   (because of the second fork())
 - Because PID != SID our process __can't take control of a TTY again__

__Reading the syslog:__

 - Locate your syslog file. Mine is here: `/var/log/syslog`
 - Do a: `grep firstdaemon /var/log/syslog`

 - The output should be similar to this one:
<pre>
<time> <user> firstdaemon[3387]: First daemon started.
<time> <user> firstdaemon[3387]: First daemon terminated.
</pre>

__A note:__
In reality you would also want to implement a signal handler and set up the logging properly (Files, log levels...).

__Further reading:__

 - [Linux-UNIX-Programmierung - German](http://openbook.galileocomputing.de/linux_unix_programmierung/Kap07-000.htm#Xxx999234)
 - [Unix Daemon Server Programming](http://www.enderunix.org/docs/eng/daemon.php)
