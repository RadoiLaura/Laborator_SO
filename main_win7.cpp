#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <windows.h>

HANDLE turnstile;
HANDLE resourceAccess;
HANDLE whiteMutex;
HANDLE blackMutex;

int whiteCount = 0;
int blackCount = 0;

struct ThreadParam {
    int id;
    int color;
};

DWORD WINAPI WhiteThread(LPVOID lpParam) {
    ThreadParam* p = (ThreadParam*)lpParam;
    int id = p->id;
    delete p;

    Sleep(rand() % 300);

    std::cout << "White " << id << " requesting access to resource.\n";

    WaitForSingleObject(turnstile, INFINITE);
    WaitForSingleObject(whiteMutex, INFINITE);
    whiteCount++;
    if (whiteCount == 1) {
        WaitForSingleObject(resourceAccess, INFINITE);
    }
    ReleaseMutex(whiteMutex);
    ReleaseMutex(turnstile);

    std::cout << "White " << id << " entered resource. (Active white threads: " << whiteCount << ")\n";
    Sleep(200 + rand() % 300);
    std::cout << "White " << id << " leaving resource.\n";

    WaitForSingleObject(whiteMutex, INFINITE);
    whiteCount--;
    if (whiteCount == 0) {
        ReleaseSemaphore(resourceAccess, 1, NULL);
    }
    ReleaseMutex(whiteMutex);

    return 0;
}

DWORD WINAPI BlackThread(LPVOID lpParam) {
    ThreadParam* p = (ThreadParam*)lpParam;
    int id = p->id;
    delete p;

    Sleep(rand() % 300);

    std::cout << "Black " << id << " requesting access to resource.\n";

    WaitForSingleObject(turnstile, INFINITE);
    WaitForSingleObject(blackMutex, INFINITE);
    blackCount++;
    if (blackCount == 1) {
        WaitForSingleObject(resourceAccess, INFINITE);
    }
    ReleaseMutex(blackMutex);
    ReleaseMutex(turnstile);

    std::cout << "Black " << id << " entered  resource. (Active black threads: " << blackCount << ")\n";
    Sleep(200 + rand() % 300);
    std::cout << "Black " << id << " leaving resource.\n";

    WaitForSingleObject(blackMutex, INFINITE);
    blackCount--;
    if (blackCount == 0) {
        ReleaseSemaphore(resourceAccess, 1, NULL);
    }
    ReleaseMutex(blackMutex);

    return 0;
}

int main() {
    srand((unsigned int)time(NULL));

    turnstile = CreateMutex(NULL, FALSE, NULL);
    resourceAccess = CreateSemaphore(NULL, 1, 1, NULL);
    whiteMutex = CreateMutex(NULL, FALSE, NULL);
    blackMutex = CreateMutex(NULL, FALSE, NULL);

    const int NUM_THREADS = 12;
    std::vector<HANDLE> threads;

    for (int i = 1; i <= NUM_THREADS; i++) {
        ThreadParam* p = new ThreadParam{ i, rand() % 2 };
        HANDLE hThread;
        if (p->color == 0) {
            hThread = CreateThread(NULL, 0, WhiteThread, p, 0, NULL);
        }
        else {
            hThread = CreateThread(NULL, 0, BlackThread, p, 0, NULL);
        }
        threads.push_back(hThread);
    }

    WaitForMultipleObjects(NUM_THREADS, threads.data(), TRUE, INFINITE);

    for (HANDLE h : threads) {
        CloseHandle(h);
    }

    CloseHandle(turnstile);
    CloseHandle(resourceAccess);
    CloseHandle(whiteMutex);
    CloseHandle(blackMutex);

    std::cout << "All threads finished execution without starvation.\n";
    return 0;
}