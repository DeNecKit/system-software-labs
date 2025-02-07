#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define error(...)                    \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    } while (0)

typedef struct {
    char *data;
    size_t size;
    size_t count;
    size_t capacity;
} array_t;

array_t *array_new(size_t size, size_t capacity)
{
    array_t *array = malloc(sizeof(array_t));
    array->size = size;
    array->count = 0;
    array->capacity = capacity;
    array->data = calloc(capacity, size);
    return array;
}

void *array_at(const array_t *array, size_t idx)
{
    assert(array != NULL);
    assert(idx < array->count);
    return array->data + idx * array->size;
}

void array_add(array_t *array, const void *elem)
{
    assert(array != NULL);
    assert(elem != NULL);
    array->count++;
    if (array->count > array->capacity) {
        array->capacity *= 2;
        void *new_data = calloc(array->capacity, array->size);
        memmove(new_data, array->data, array->count * array->size);
        free(array->data);
        array->data = new_data;
    }
    memmove(array_at(array, array->count - 1), elem, array->size);
}

void array_free(array_t *array)
{
    free(array->data);
    free(array);
}

typedef struct {
    FILE *file;
    const char *path;
    const char *name;
} file_t;

void collect_files(const char *path, array_t *array)
{
    DIR *dir = opendir(path);
    if (dir == NULL) error("Failed to open directory \"%s\": %s\n",
                           path, strerror(errno));
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0 ||
            strcmp(d->d_name, "..") == 0) continue;
        char *filepath = malloc(1024);
        memset(filepath, 0x00, 1024);
        sprintf(filepath, "%s/%s", path, d->d_name);
        if (d->d_type == DT_DIR) {
            collect_files(filepath, array);
            continue;
        }
        FILE *file = fopen(filepath, "rb");
        if (file == NULL) error("Failed to open file \"%s\": %s\n",
                                filepath, strerror(errno));
        array_add(array, & (file_t) { .file = file, .path = filepath, .name = d->d_name });
    }
}

int cmpsize(const void *file1, const void *file2)
{
    struct stat st1;
    stat(((file_t*)file1)->path, &st1);
    struct stat st2;
    stat(((file_t*)file2)->path, &st2);
    return st1.st_size - st2.st_size;
}

int cmpname(const void *file1, const void *file2)
{
    const char *name1 = ((file_t*)file1)->name;
    const char *name2 = ((file_t*)file2)->name;
    return strcmp(name1, name2);
}

int main(int argc, char **argv)
{
    if (argc < 2) error("Expected input directory\n");
    if (argc < 3) error("Expected sort type\n");
    if (argc < 4) error("Expected output directory\n");

    const int sorttype = atoi(argv[2]);
    if (sorttype != 1 && sorttype != 2) {
        error("Invalid sort type\n"
              "Must be 1 or 2\n");
    }

    const char *in = argv[1];
    array_t *files = array_new(sizeof(file_t), 1);
    collect_files(in, files);

    qsort(files->data, files->count, files->size,
          sorttype == 1 ? cmpsize : cmpname);

    const char *out = argv[3];
    for (size_t i = 0; i < files->count; i++) {
        file_t *file = array_at(files, i);
        char filepath[1024] = { 0 };
        sprintf(filepath, "%s/%03ld (%s)", out, i + 1, file->name);
        FILE *infile = file->file;
        mkdir(out, 0777);
        FILE *outfile = fopen(filepath, "wb");
        int c;
        while ((c = fgetc(infile)) != EOF) fputc(c, outfile);
        fclose(infile);
    }
}
