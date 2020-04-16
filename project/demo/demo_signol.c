///* ISO C99 signals.  */
//#define SIGINT      2   /* Interactive attention signal.  */
//#define SIGILL      4   /* Illegal instruction.  */
//#define SIGABRT     6   /* Abnormal termination.  */
//#define SIGFPE      8   /* Erroneous arithmetic operation.  */
//#define SIGSEGV     11  /* Invalid access to storage.  */
//#define SIGTERM     15  /* Termination request.  */
//
///* Historical signals specified by POSIX. */
//#define SIGHUP      1   /* Hangup.  */
//#define SIGQUIT     3   /* Quit.  */
//#define SIGTRAP     5   /* Trace/breakpoint trap.  */
//#define SIGKILL     9   /* Killed.  */
//#define SIGBUS      10  /* Bus error.  */
//#define SIGSYS      12  /* Bad system call.  */
//#define SIGPIPE     13  /* Broken pipe.  */
//#define SIGALRM     14  /* Alarm clock.  */
//
///* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */
//#define SIGURG      16  /* Urgent data is available at a socket.  */
//#define SIGSTOP     17  /* Stop, unblockable.  */
//#define SIGTSTP     18  /* Keyboard stop.  */
//#define SIGCONT     19  /* Continue.  */
//#define SIGCHLD     20  /* Child terminated or stopped.  */
//#define SIGTTIN     21  /* Background read from control terminal.  */
//#define SIGTTOU     22  /* Background write to control terminal.  */
//#define SIGPOLL     23  /* Pollable event occurred (System V).  */
//#define SIGXCPU     24  /* CPU time limit exceeded.  */
//#define SIGXFSZ     25  /* File size limit exceeded.  */
//#define SIGVTALRM   26  /* Virtual timer expired.  */
//#define SIGPROF     27  /* Profiling timer expired.  */
//#define SIGUSR1     30  /* User-defined signal 1.  */
//#define SIGUSR2     31  /* User-defined signal 2.  */
//
///* Nonstandard signals found in all modern POSIX systems
//   82    (including both BSD and Linux).  */
//#define SIGWINCH    28  /* Window size change (4.3 BSD, Sun).  */
//
///* Archaic names for compatibility.  */
//#define SIGIO       SIGPOLL /* I/O now possible (4.2 BSD).  */
//#define SIGIOT      SIGABRT /* IOT instruction, abort() on a PDP-11.  */
//#define SIGCLD      SIGCHLD /* Old System V name */
//
//#define SIGSTKFLT   16  /* Stack fault (obsolete).  */
//#define SIGPWR      30  /* Power failure imminent.  */


#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define MACRO_BODY \
ENUM_D(SIGINT     ),   /* Interactive attention signal.  */\
ENUM_D(SIGILL     ),   /* Illegal instruction.  */\
ENUM_D(SIGABRT    ),   /* Abnormal termination.  */\
ENUM_D(SIGFPE     ),   /* Erroneous arithmetic operation.  */\
ENUM_D(SIGSEGV    ),   /* Invalid access to storage.  */\
ENUM_D(SIGTERM    ),   /* Termination request.  */\
\
/* Historical signals specified by POSIX. */\
ENUM_D(SIGHUP     ),   /* Hangup.  */\
ENUM_D(SIGQUIT    ),   /* Quit.  */\
ENUM_D(SIGTRAP    ),   /* Trace/breakpoint trap.  */\
ENUM_D(SIGKILL    ),   /* Killed.  */\
ENUM_D(SIGBUS     ),   /* Bus error.  */\
ENUM_D(SIGSYS     ),   /* Bad system call.  */\
ENUM_D(SIGPIPE    ),   /* Broken pipe.  */\
ENUM_D(SIGALRM    ),   /* Alarm clock.  */\
\
/* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */\
ENUM_D(SIGURG     ),   /* Urgent data is available at a socket.  */\
ENUM_D(SIGSTOP    ),   /* Stop, unblockable.  */\
ENUM_D(SIGTSTP    ),   /* Keyboard stop.  */\
ENUM_D(SIGCONT    ),   /* Continue.  */\
ENUM_D(SIGCHLD    ),   /* Child terminated or stopped.  */\
ENUM_D(SIGTTIN    ),   /* Background read from control terminal.  */\
ENUM_D(SIGTTOU    ),   /* Background write to control terminal.  */\
ENUM_D(SIGPOLL    ),   /* Pollable event occurred (System V).  */\
ENUM_D(SIGXCPU    ),   /* CPU time limit exceeded.  */\
ENUM_D(SIGXFSZ    ),   /* File size limit exceeded.  */\
ENUM_D(SIGVTALRM  ),   /* Virtual timer expired.  */\
ENUM_D(SIGPROF    ),   /* Profiling timer expired.  */\
ENUM_D(SIGUSR1    ),   /* User-defined signal 1.  */\
ENUM_D(SIGUSR2    ),   /* User-defined signal 2.  */\
/*ENUM_D(SIGEMT     ),*/ \
ENUM_D(SIGIO      ), \
ENUM_D(SIGIOT     ), \
ENUM_D(SIGPWR     ), \
ENUM_D(SIGSTKFLT  ), \
ENUM_D(SIGWINCH   ), \

#ifdef ENUM_D
#undef ENUM_D
#endif
#define ENUM_D(x) x

static int gas32Msg[] = {
    MACRO_BODY
};


#ifdef ENUM_D
#undef ENUM_D
#endif
#define ENUM_D(x) #x

static const char *gaps8Msg[] = {
    MACRO_BODY
};

#define SIGNUM  sizeof(gas32Msg) / sizeof(int)

static void sig_fun(int sig)
{
    size_t i;

    fprintf(stdout, "signo:%d", sig);
    for (i = 0; i < SIGNUM; i++) {
        if (sig == gas32Msg[i])
            fprintf(stdout, ", signame:%s", gaps8Msg[i]);
    }
    putchar('\n');
}


// Note: trap -l can see all signal of linux
int main(void)
{
    size_t i;

    fprintf(stdout, "linux signum:%lu\n", SIGNUM);

    for (i = 0; i < SIGNUM; i++) {
        if (signal(gas32Msg[i], sig_fun) == SIG_ERR)
            fprintf(stderr, "err: signame:%s\n", gaps8Msg[i]);
    }

    for (; ;)
        pause();

    return 0;
}


