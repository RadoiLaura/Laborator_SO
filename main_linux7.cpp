#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_mutex_t turnstile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t whiteMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blackMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t resourceAccess;

int whiteCount = 0;
int blackCount = 0;

struct ThreadParam {
    int id;
    int color;
};

void* whiteThread(void* arg) {
    ThreadParam* p = (ThreadParam*)arg;
    int id = p->id;
    delete p;

    usleep((rand() % 300) * 1000);

    std::cout << "White " << id << " requesting access to resource.\n";

    pthread_mutex_lock(&turnstile);
    pthread_mutex_lock(&whiteMutex);
    whiteCount++;
    if (whiteCount == 1) {
        sem_wait(&resourceAccess);
    }
    pthread_mutex_unlock(&whiteMutex);
    pthread_mutex_unlock(&turnstile);

    std::cout << "White " << id << " entered resource. (Active white threads: " << whiteCount << ")\n";
    usleep((200 + rand() % 300) * 1000);
    std::cout << "White " << id << " leaving resource.\n";

    pthread_mutex_lock(&whiteMutex);
    whiteCount--;
    if (whiteCount == 0) {
        sem_post(&resourceAccess);
    }
    pthread_mutex_unlock(&whiteMutex);

    return NULL;
}

void* blackThread(void* arg) {
    ThreadParam* p = (ThreadParam*)arg;
    int id = p->id;
    delete p;

    usleep((rand() % 300) * 1000);

    std::cout << "Black " << id << " requesting access to resource.\n";

    pthread_mutex_lock(&turnstile);
    pthread_mutex_lock(&blackMutex);
    blackCount++;
    if (blackCount == 1) {
        sem_wait(&resourceAccess);
    }
    pthread_mutex_unlock(&blackMutex);
    pthread_mutex_unlock(&turnstile);

    std::cout << "Black " << id << " entered resource. (Active black threads: " << blackCount << ")\n";
    usleep((200 + rand() % 300) * 1000);
    std::cout << "Black " << id << " leaving resource.\n";

    pthread_mutex_lock(&blackMutex);
    blackCount--;
    if (blackCount == 0) {
        sem_post(&resourceAccess);
    }
    pthread_mutex_unlock(&blackMutex);

    return NULL;
}

int main() {
    srand((unsigned int)time(NULL));
    sem_init(&resourceAccess, 0, 1);

    const int NUM_THREADS = 12;
    std::vector<pthread_t> threads(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) {
        ThreadParam* p = new ThreadParam{ i + 1, rand() % 2 };
        if (p->color == 0) {
            pthread_create(&threads[i], NULL, whiteThread, p);
        }
        else {
            pthread_create(&threads[i], NULL, blackThread, p);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&resourceAccess);
    pthread_mutex_destroy(&turnstile);
    pthread_mutex_destroy(&whiteMutex);
    pthread_mutex_destroy(&blackMutex);

    std::cout << "All threads finished execution without starvation.\n";
    return 0;
}