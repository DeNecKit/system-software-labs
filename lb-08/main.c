#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <time.h>

#define MAX_CLIENT_QUEUE 1024
#define MAX_CLIENTS 5

typedef enum { ORDINARY, VIP } client_type_t;

size_t client_cur = 0;
client_type_t client_type_cur = -1;

typedef struct {
    size_t clients[MAX_CLIENT_QUEUE];
    int start, count;
} client_queue_t;

client_queue_t client_queue, client_vip_queue;

size_t client_arrive(void)
{
    client_type_t client_type = rand() % 2;
    switch (client_type) {
        case ORDINARY:
            assert(client_queue.count < MAX_CLIENT_QUEUE
                && "Queue overflow");
            client_queue.clients[client_queue.count++] = client_cur++;
            break;
        case VIP:
            assert(client_vip_queue.count < MAX_CLIENT_QUEUE
                && "Queue overflow");
            client_vip_queue.clients[client_vip_queue.count++] = client_cur++;
            break;
        default: assert(FALSE && "Unreachable");
    }
    client_type_cur = client_type;
    return client_cur - 1;
}

BOOL client_next(size_t *client, client_type_t *type)
{
    if (client_vip_queue.start < client_vip_queue.count) {
        *client = client_vip_queue.clients[client_vip_queue.start++];
        *type = VIP;
        return TRUE;
    }
    if (client_queue.start < client_queue.count) {
        *client = client_queue.clients[client_queue.start++];
        *type = ORDINARY;
        return TRUE;
    }
    return FALSE;
}

BOOL client_try_leave(client_type_t type)
{
    int count = client_queue.count - client_queue.start;
    count += client_vip_queue.count - client_vip_queue.start;
    if (count > MAX_CLIENTS) {
        if (type == ORDINARY) client_queue.count--;
        else client_vip_queue.count--;
        return TRUE;
    }
    return FALSE;
}

CRITICAL_SECTION is_client;
BOOL barber_done = FALSE;

DWORD barber_routine(LPVOID)
{
    while (TRUE) {
        wprintf(L"Парикмахер засыпает\n");
        EnterCriticalSection(&is_client);
        wprintf(L"Парикмахер просыпается\n");
        size_t client;
        client_type_t type;
        while (client_next(&client, &type)) {
            wprintf(L"Парикмахер начинает обслуживать клиента №%d\n", client + 1);
            Sleep(800);
            wprintf(L"Парикмахер закончил обслуживать клиента №%d\n", client + 1);
        }
        LeaveCriticalSection(&is_client);
        barber_done = TRUE;
        Sleep(100);
    }
}

int main(void)
{
    setlocale(LC_ALL, "Russian");

    srand(time(NULL));

    InitializeCriticalSection(&is_client);
    EnterCriticalSection(&is_client);

    wprintf(L"В парикмахерской %d стульев\n", MAX_CLIENTS);

    CreateThread(NULL, 0, barber_routine, NULL, 0, NULL);
    Sleep(500);

    while (TRUE) {
        size_t client = client_arrive();
        wprintf(L"Пришёл клиент №%d (%ls)\n",
                client + 1, client_type_cur == ORDINARY
                ? L"обычный" : L"\"блатной\"");
        if (client_try_leave(client_type_cur)) {
            wprintf(L"Клиент №%d ушёл\n", client + 1);
        } else LeaveCriticalSection(&is_client);
        for (int i = 0; i < rand() % 16 + 5; i++) {
            if (barber_done) {
                EnterCriticalSection(&is_client);
                barber_done = FALSE;
            }
            Sleep(100);
        }
    }
}
