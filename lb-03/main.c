#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(EXIT_FAILURE);           \
    } while (0)

#define N 4

struct timeval time_start, time_cur;

pid_t first_pid;
pid_t self_pid;
pid_t child_pid;
int idx;

void sighandler(int signum)
{
    gettimeofday(&time_cur, NULL);
    int ms = (time_cur.tv_sec - time_start.tv_sec) * 1000;
    ms += (time_cur.tv_usec - time_start.tv_usec) / 1000;
    printf("pid: %d, ppid: %d, %dms (#%d get %s)\n",
           getpid(), getppid(), ms, idx,
           signum == SIGUSR1 ? "SIGUSR1" : "SIGUSR2");
    usleep(100000);

    if (idx < N) kill(child_pid, signum);
    else kill(first_pid, signum == SIGUSR1 ? SIGUSR2 : SIGUSR1);
    gettimeofday(&time_cur, NULL);
    ms = (time_cur.tv_sec - time_start.tv_sec) * 1000;
    ms += (time_cur.tv_usec - time_start.tv_usec) / 1000;
    printf("pid: %d, ppid: %d, %dms (#%d put %s)\n",
           getpid(), getppid(), ms, idx,
           signum == SIGUSR1 ? (idx < N ? "SIGUSR1" : "SIGUSR2")
                             : (idx < N ? "SIGUSR2" : "SIGUSR1"));
}

int main(void)
{
    gettimeofday(&time_start, NULL);

    signal(SIGUSR1, sighandler);
    signal(SIGUSR2, sighandler);

    first_pid = getpid();
    

    for (int i = 0; i < N - 1; i++) {
        child_pid = fork();
        if (child_pid > 0) {
            self_pid = getpid();
            idx = i + 1;
            if (i == 0) {
                usleep(100000);
                kill(child_pid, SIGUSR1);
            }
            while (1) pause();
        } else if (child_pid < 0)
            error("Failed to fork the process: %s\n",
                  strerror(errno));
    }

    idx = N;
    self_pid = getpid();
    while (1) pause();
}
