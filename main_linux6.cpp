#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

const char* SHM_NAME = "/shm_numaratoare";
const char* SEM_NAME = "/sem_numaratoare";
const int MAX_VAL = 1000;

int main() {
    srand(time(NULL) ^ getpid());

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        return 1;
    }

    ftruncate(shm_fd, sizeof(int));

    int* valoare_memorie = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (valoare_memorie == MAP_FAILED) {
        return 1;
    }

    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        return 1;
    }

    std::cout << "[Proces PID " << getpid() << "] A pornit si este gata de numarat.\n";

    while (true) {
        sem_wait(sem);

        int val_curenta = *valoare_memorie;

        if (val_curenta >= MAX_VAL) {
            sem_post(sem);
            break;
        }

        while ((rand() % 2 == 1) && (*valoare_memorie < MAX_VAL)) {
            (*valoare_memorie)++;
            std::cout << "[Proces PID " << getpid() << "] A dat cu banul 2 -> A scris in memorie: " << *valoare_memorie << "\n";
        }

        sem_post(sem);
        usleep(10000);
    }

    std::cout << "[Proces PID " << getpid() << "] S-a oprit deoarece s-a atins valoarea " << MAX_VAL << ".\n";

    munmap(valoare_memorie, sizeof(int));
    close(shm_fd);
    sem_close(sem);

    return 0;
}