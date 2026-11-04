#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

bool estePrim(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main() {
    std::cout << "=== CAUTARE NUMERE PRIME PANA LA 10.000 (10 PROCESE - LINUX) ===\n\n";

    const int NUM_PROCESE = 10;
    const int DIM_INTERVAL = 1000;

    int pipePaiToCopil[NUM_PROCESE][2];
    int pipeCopilToPai[NUM_PROCESE][2];
    pid_t pids[NUM_PROCESE];

    for (int i = 0; i < NUM_PROCESE; i++) {
        if (pipe(pipePaiToCopil[i]) == -1 || pipe(pipeCopilToPai[i]) == -1) {
            std::cerr << "Eroare la crearea pipe-urilor!\n";
            return 1;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            std::cerr << "Eroare la fork()!\n";
            return 1;
        }
        else if (pids[i] == 0) {
            close(pipePaiToCopil[i][1]);
            close(pipeCopilToPai[i][0]);

            int start, end;
            read(pipePaiToCopil[i][0], &start, sizeof(int));
            read(pipePaiToCopil[i][0], &end, sizeof(int));
            close(pipePaiToCopil[i][0]);

            std::vector<int> prime;
            for (int val = start; val <= end; val++) {
                if (estePrim(val)) {
                    prime.push_back(val);
                }
            }

            int count = prime.size();
            write(pipeCopilToPai[i][1], &count, sizeof(int));
            if (count > 0) {
                write(pipeCopilToPai[i][1], prime.data(), count * sizeof(int));
            }

            close(pipeCopilToPai[i][1]);
            return 0;
        }
        else {
            close(pipePaiToCopil[i][0]);
            close(pipeCopilToPai[i][1]);

            int start = i * DIM_INTERVAL + 1;
            int end = (i + 1) * DIM_INTERVAL;
            write(pipePaiToCopil[i][1], &start, sizeof(int));
            write(pipePaiToCopil[i][1], &end, sizeof(int));
            close(pipePaiToCopil[i][1]);
        }
    }

    int totalPrime = 0;
    for (int i = 0; i < NUM_PROCESE; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        int count = 0;
        read(pipeCopilToPai[i][0], &count, sizeof(int));

        std::vector<int> prime(count);
        if (count > 0) {
            read(pipeCopilToPai[i][0], prime.data(), count * sizeof(int));
        }
        close(pipeCopilToPai[i][0]);

        std::cout << "--- Proces " << (i + 1) << " (PID " << pids[i] << ", Interval " << (i * DIM_INTERVAL + 1) << " - " << ((i + 1) * DIM_INTERVAL) << ") a gasit " << count << " numere prime ---\n";
        for (int p : prime) {
            std::cout << p << " ";
        }
        std::cout << "\n\n";

        totalPrime += count;
    }

    std::cout << "=== TOTAL NUMERE PRIME GĂSITE: " << totalPrime << " ===\n";

    return 0;
}