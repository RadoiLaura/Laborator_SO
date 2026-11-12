#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

const char* SHM_NAME = "Local\\ShmNumaratoare";
const char* SEM_NAME = "Local\\SemNumaratoare";
const int MAX_VAL = 1000;

int main() {
    srand((unsigned int)time(NULL) ^ GetCurrentProcessId());

    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(int),
        SHM_NAME
    );

    if (hMapFile == NULL) {
        return 1;
    }

    int* valoare_memorie = (int*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(int)
    );

    if (valoare_memorie == NULL) {
        CloseHandle(hMapFile);
        return 1;
    }

    HANDLE hSemaphore = CreateSemaphoreA(
        NULL,
        1,
        1,
        SEM_NAME
    );

    if (hSemaphore == NULL) {
        UnmapViewOfFile(valoare_memorie);
        CloseHandle(hMapFile);
        return 1;
    }

    DWORD pid = GetCurrentProcessId();
    std::cout << "[Proces PID " << pid << "] A pornit si este gata de numarat.\n";

    while (true) {
        WaitForSingleObject(hSemaphore, INFINITE);

        int val_curenta = *valoare_memorie;

        if (val_curenta >= MAX_VAL) {
            ReleaseSemaphore(hSemaphore, 1, NULL);
            break;
        }

        while ((rand() % 2 == 1) && (*valoare_memorie < MAX_VAL)) {
            (*valoare_memorie)++;
            std::cout << "[Proces PID " << pid << "] A dat cu banul 2 -> A scris in memorie: " << *valoare_memorie << "\n";
        }

        ReleaseSemaphore(hSemaphore, 1, NULL);
        Sleep(10);
    }

    std::cout << "[Proces PID " << pid << "] S-a oprit deoarece s-a atins valoarea " << MAX_VAL << ".\n";

    UnmapViewOfFile(valoare_memorie);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

    return 0;
}