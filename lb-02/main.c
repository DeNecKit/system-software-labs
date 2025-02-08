#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PI 3.14159265358979323846

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

int parse_int(const char *msg)
{
    int x;
    printf("%s", msg);
    if (scanf("%d", &x) != 1) error("Invalid number\n");
    return x;
}

int main(void)
{
    int k = parse_int("K = ");
    int N = parse_int("N = ");
    int n = parse_int("n = ");

    char out[1024];
    printf("Output file: ");
    if (scanf("%1023s", out) != 1) {
        error("Invalid output filename\n");
    }

    float factorials[2*n + 2];
    factorials[0] = 1;
    for (int i = 1; i < 2*n + 2; i++) {
        factorials[i] = i * factorials[i - 1];
    }

    FILE *file = fopen(out, "w");
    if (file == NULL) error("Failed to create file \"%s\": %s\n",
                            out, strerror(errno));

    float sines[k];

    for (int i = 0; i < k; i++) {
        pid_t pids[n];
        int fd[2];
        pipe(fd);
        float x = 2*PI*i/N;
        while (x < 0) x += 2*PI;
        while (x > 2*PI) x -= 2*PI;
        for (int j = 0; j < n; j++) {
            if ((pids[j] = fork()) < 0) {
                error("Failed to fork the process\n");
            } else if (pids[j] == 0) {
                float elem = j % 2 == 0 ? 1.f : -1.f;
                elem /= factorials[2*j + 1];
                for (int jj = 0; jj < 2*j + 1; jj++) elem *= x;
                printf("pid = %d; term = %.2f\n", getpid(), elem);
                write(fd[1], &elem, sizeof(float));
                close(fd[0]);
                close(fd[1]);
                exit(0);
            }
        }

        float sum = 0.f;
        int pidc = n;
        while (pidc > 0) {
            wait(NULL);
            float x;
            read(fd[0], &x, sizeof(float));
            sum += x;
            pidc--;
        }
        close(fd[0]);
        close(fd[1]);

        sines[i] = sum;
    }

    for (int i = 0; i < k; i++) {
        fprintf(file, "sin(2*PI*%d/%d) = %.2f\n", i, N, sines[i]);
    }

    fclose(file);
}
