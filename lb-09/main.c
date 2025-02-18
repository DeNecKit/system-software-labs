#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>

typedef struct seq_item_s {
    void *data;
    size_t size;
    struct seq_item_s *next;
} seq_item_t;

typedef struct {
    seq_item_t *cur;
    seq_item_t *start;
    seq_item_t *end;
} seq_t;

const size_t seq_size = 10 * 1024 + sizeof(seq_t);

seq_t *seq_new(void)
{
    seq_t *seq = VirtualAlloc(
        NULL, seq_size, MEM_COMMIT, PAGE_READWRITE);
    *seq = (seq_t) { 0 };
    return seq;
}

bool seq_is_empty(const seq_t *seq)
{
    return seq != NULL && seq->cur == NULL;
}

bool seq_set_start(seq_t *seq)
{
    if (seq == NULL) return false;
    seq->cur = seq->start;
    return true;
}

bool seq_read(seq_t *seq, void *data, int *size)
{
    if (seq == NULL || seq->cur == NULL) return false;
    if (data != NULL) memmove(data, seq->cur->data, seq->cur->size);
    if (size != NULL) *size = seq->cur->size;
    seq->cur = seq->cur->next;
    return true;
}

bool seq_add(seq_t *seq, const void *data, size_t size)
{
    if (seq == NULL) return false;
    if (data == NULL || size == 0) return false;

    if (seq->cur == NULL) {
        seq->start = (seq_item_t*)(seq + 1);
        seq->cur = seq->start;
        seq->end = seq->start;
    } else {
        seq->end->next =
            (seq_item_t*)((char*)(seq->end + 1) + seq->end->size);
        seq->end = seq->end->next;
    }

    const size_t cur_seq_size =
        (char*)(seq->end + 1) + size - (char*)seq;
    if (cur_seq_size > seq_size) return false;

    void *new_data = seq->end + 1;
    seq_item_t item = {
        .data = new_data,
        .size = size,
        .next = NULL
    };
    memmove(seq->end, &item, sizeof(item));
    memmove(new_data, data, size);
    return true;
}

bool seq_clear(seq_t *seq)
{
    if (seq == NULL) return false;
    seq->cur = NULL;
    seq->start = NULL;
    seq->end = NULL;
    return true;
}

bool seq_swap_end(seq_t *seq)
{
    if (seq == NULL || seq->cur == NULL) return false;
    const seq_item_t cur = *seq->cur;
    seq->cur->data = seq->end->data;
    seq->cur->size = seq->end->size;
    seq->end->data = cur.data;
    seq->end->size = cur.size;
    return true;
}

void seq_free(seq_t *seq)
{
    VirtualFree(seq, 0, MEM_RELEASE);
}

#define STRBOOL(bool) (bool ? "true" : "false")

int main(void)
{
    setlocale(LC_ALL, "Russian");
    
    wprintf(L"Создание новой последовательности\n");
    seq_t *seq = seq_new();
    assert(seq != NULL);

    wprintf(L"Проверка последовательности на пустоту: %S\n",
            STRBOOL(seq_is_empty(seq)));

    wprintf(L"Добавление элементов (1, 2, 3) "
            L"в конец последовательности\n");
    assert(seq_add(seq, & (int) { 1 }, sizeof(int)));
    assert(seq_add(seq, & (int) { 2 }, sizeof(int)));
    assert(seq_add(seq, & (int) { 3 }, sizeof(int)));

    wprintf(L"Проверка последовательности на пустоту: %S\n",
        STRBOOL(seq_is_empty(seq)));
    
    wprintf(L"Чтение элементов последовательности:\n");
    int item;
    while (seq_read(seq, &item, NULL)) printf("%d\n", item);

    wprintf(L"Установка указателя последовательности в начало\n");
    assert(seq_set_start(seq));

    wprintf(L"Продвижение указателя на один вперёд "
            L"и обменивание значениями текущего элмента "
            L"(второго) на последний\n");
    assert(seq_read(seq, NULL, NULL));
    assert(seq_swap_end(seq));

    wprintf(L"Установка указателя последовательности в начало "
            L"и чтение элементов последовательности:\n");
    assert(seq_set_start(seq));
    while (seq_read(seq, &item, NULL)) printf("%d\n", item);

    wprintf(L"Опустошение последовательности "
            L"и проверка последовательности на пустоту: %S\n",
            STRBOOL(seq_is_empty(seq)));
    
    wprintf(L"Освобождение памяти последовательности\n");
    seq_free(seq);
    seq = NULL;
}
