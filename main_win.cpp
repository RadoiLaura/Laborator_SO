#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

bool estePrim(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "copil") {
        HANDLE hRead = (HANDLE)(uintptr_t)std::stoull(argv[2]);
        HANDLE hWrite = (HANDLE)(uintptr_t)std::stoull(argv[3]);

        int start, end;
        DWORD bytesRead;
        ReadFile(hRead, &start, sizeof(int), &bytesRead, NULL);
        ReadFile(hRead, &end, sizeof(int), &bytesRead, NULL);

        std::vector<int> prime;
        for (int i = start; i <= end; i++) {
            if (estePrim(i)) {
                prime.push_back(i);
            }
        }

        int count = prime.size();
        DWORD bytesWritten;
        WriteFile(hWrite, &count, sizeof(int), &bytesWritten, NULL);
        if (count > 0) {
            WriteFile(hWrite, prime.data(), count * sizeof(int), &bytesWritten, NULL);
        }

        CloseHandle(hRead);
        CloseHandle(hWrite);
        return 0;
    }

    std::cout << "=== CAUTARE NUMERE PRIME PANA LA 10.000 (10 PROCESE - WINDOWS) ===\n\n";

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    const int NUM_PROCESE = 10;
    const int DIM_INTERVAL = 1000;

    HANDLE hPaiToCopil_R[NUM_PROCESE], hPaiToCopil_W[NUM_PROCESE];
    HANDLE hCopilToPai_R[NUM_PROCESE], hCopilToPai_W[NUM_PROCESE];
    PROCESS_INFORMATION pi[NUM_PROCESE];

    for (int i = 0; i < NUM_PROCESE; i++) {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        CreatePipe(&hPaiToCopil_R[i], &hPaiToCopil_W[i], &saAttr, 0);
        SetHandleInformation(hPaiToCopil_W[i], HANDLE_FLAG_INHERIT, 0);

        CreatePipe(&hCopilToPai_R[i], &hCopilToPai_W[i], &saAttr, 0);
        SetHandleInformation(hCopilToPai_R[i], HANDLE_FLAG_INHERIT, 0);

        std::string cmd = std::string("\"") + exePath + "\" copil " +
            std::to_string((uintptr_t)hPaiToCopil_R[i]) + " " +
            std::to_string((uintptr_t)hCopilToPai_W[i]);

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi[i], sizeof(pi[i]));

        CreateProcessA(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi[i]);

        int start = i * DIM_INTERVAL + 1;
        int end = (i + 1) * DIM_INTERVAL;
        DWORD bytesWritten;
        WriteFile(hPaiToCopil_W[i], &start, sizeof(int), &bytesWritten, NULL);
        WriteFile(hPaiToCopil_W[i], &end, sizeof(int), &bytesWritten, NULL);

        CloseHandle(hPaiToCopil_R[i]);
        CloseHandle(hCopilToPai_W[i]);
    }

    int totalPrime = 0;
    for (int i = 0; i < NUM_PROCESE; i++) {
        WaitForSingleObject(pi[i].hProcess, INFINITE);

        int count = 0;
        DWORD bytesRead;
        ReadFile(hCopilToPai_R[i], &count, sizeof(int), &bytesRead, NULL);

        std::vector<int> prime(count);
        if (count > 0) {
            ReadFile(hCopilToPai_R[i], prime.data(), count * sizeof(int), &bytesRead, NULL);
        }

        std::cout << "Process " << (i + 1) << " (Interval " << (i * DIM_INTERVAL + 1) << " - " << ((i + 1) * DIM_INTERVAL) << ") a gasit " << count << " numere prime ---\n";
        for (int p : prime) {
            std::cout << p << " ";
        }
        std::cout << "\n\n";

        totalPrime += count;

        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
        CloseHandle(hPaiToCopil_W[i]);
        CloseHandle(hCopilToPai_R[i]);
    }

    std::cout << "=== TOTAL NUMERE PRIME GĂSITE: " << totalPrime << " ===\n";

    return 0;
}