#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

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

float *factorials = NULL;

typedef struct {
    int j;
    float x;
    float term;
} sine_arg_t;

void *sine_term(void *arg)
{
    sine_arg_t *sine_arg = arg;
    int j = sine_arg->j;
    float x = sine_arg->x;
    float term = j % 2 == 0 ? 1.f : -1.f;
    term /= factorials[2*j + 1];
    for (int jj = 0; jj < 2*j + 1; jj++) term *= x;

    printf("thread id = %ld; term = %.2f\n", pthread_self(), term);

    sine_arg->term = term;
    return NULL;
}

sine_arg_t *thread_args = NULL;

int main(void)
{
    int k = parse_int("K = ");
    int N = parse_int("N = ");
    int n = parse_int("n = ");

    char out[1024];
    printf("Output file: ");
    if (scanf("%1023s", out) != 1) {
        error("Invalid output filename: %s\n",
              strerror(errno));
    }

    factorials = calloc(2*n + 2, sizeof(float));
    factorials[0] = 1;
    for (int i = 1; i < 2*n + 2; i++) {
        factorials[i] = i * factorials[i - 1];
    }

    FILE *file = fopen(out, "w");
    if (file == NULL) error("Failed to create file \"%s\": %s\n",
                            out, strerror(errno));

    pthread_t threads[n];
    thread_args = calloc(n, sizeof(sine_arg_t));

    for (int i = 0; i < k; i++) {
        float x = 2*PI*i/N;
        while (x < 0) x += 2*PI;
        while (x > 2*PI) x -= 2*PI;
        for (int j = 0; j < n; j++) {
            thread_args[j].j = j;
            thread_args[j].x = x;
            int err = pthread_create(
                &threads[j], NULL, sine_term, &thread_args[j]);
            if (err != 0)
                error("Failed to create a thread: %s\n",
                      strerror(err));
        }

        float sum = 0.f;
        for (int j = 0; j < n; j++) {
            pthread_join(threads[j], NULL);
            sum += thread_args[j].term;
        }

        fprintf(file, "sin(2*PI*%d/%d) = %.2f\n", i, N, sum);
    }

    fclose(file);
}
