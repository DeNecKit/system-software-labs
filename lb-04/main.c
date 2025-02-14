#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(EXIT_FAILURE);           \
    } while (0)

#define N 4
const char *filename = "/tmp/sem";

struct timeval time_start, time_cur;

int idx;
sem_t *sems[N] = { NULL };

FILE *file = NULL;

void handle_process()
{
    if (sem_wait(sems[idx]) == -1)
        error("Failed to wait for a semaphore: %s\n",
              strerror(errno));

    file = fopen(filename, "r");
    if (file == NULL)
        error("Failed to read from file \"%s\": %s\n",
              filename, strerror(errno));
    char msg[1024] = { 0 };
    char *msg_ptr = msg;
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (msg_ptr - msg >= 1023) error("Message too long\n");
        sprintf(msg_ptr++, "%c", c);
    }
    printf("%s", msg);
    fclose(file);

    usleep(100000);

    file = fopen(filename, "w");
    if (file == NULL)
        error("Failed to write to file \"%s\": %s\n",
              filename, strerror(errno));
    memset(msg, 0x00, sizeof(msg));
    gettimeofday(&time_cur, NULL);
    int ms = (time_cur.tv_sec - time_start.tv_sec) * 1000;
    ms += (time_cur.tv_usec - time_start.tv_usec) / 1000;
    sprintf(msg, "#%d pid: %d, ppid: %d, %dms\n",
                    idx + 1, getpid(), getppid(), ms);
    fwrite(msg, sizeof(char), strlen(msg), file);
    fclose(file);

    if (sem_post(sems[(idx + 1) % N]) == -1)
        error("Failed to post a semaphore: %s\n",
                strerror(errno));
}

void quit(int)
{
    for (int i = 0; i < N; i++) {
        sem_close(sems[i]);
        char name[1024] = { 0 };
        sprintf(name, "/sem%d", i);
        sem_unlink(name);
    }
    printf("\nExited successfully\n");
    exit(EXIT_SUCCESS);
}

int main(void)
{
    gettimeofday(&time_start, NULL);

    file = fopen(filename, "w");
    if (file == NULL)
        error("Failed to create file \"%s\": %s\n",
              filename, strerror(errno));
    fclose(file);

    for (int i = 0; i < N; i++) {
        char name[1024] = { 0 };
        sprintf(name, "/sem%d", i);
        sems[i] = sem_open(name, O_CREAT | O_EXCL, O_RDWR, i == 0);
        if (sems[i] == SEM_FAILED)
            error("Failed to create a semaphore: %s\n",
                  strerror(errno));
    }

    for (int i = 0; i < N - 1; i++) {
        pid_t pid = fork();
        if (pid > 0) {
            idx = i;
            if (i == 0) signal(SIGINT, quit);
            while (1) handle_process();
        } else if (pid < 0)
            error("Failed to fork the process: %s\n",
                  strerror(errno));
    }

    idx = N - 1;
    while (1) handle_process();
}
