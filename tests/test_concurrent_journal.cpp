
#include "journal.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

int main() {
    char templatePath[] = "/tmp/storageguard-concurrent-XXXXXX";
    char* dir = mkdtemp(templatePath);

    if (dir == nullptr) {
        std::cerr << "FAIL: Could not create temporary directory\n";
        return 1;
    }

    const fs::path originalDir = fs::current_path();
    fs::current_path(dir);

    const bool started = appendTransaction(
        {1, "CREATE_FILE", TransactionState::STARTED}, false);

    if (!started) {
        fs::current_path(originalDir);
        fs::remove_all(dir);
        std::cerr << "FAIL: Could not start transaction\n";
        return 1;
    }

    int readyPipe[2];
    if (pipe(readyPipe) == -1) {
        fs::current_path(originalDir);
        fs::remove_all(dir);
        return 1;
    }

    pid_t children[2];

    for (int i = 0; i < 2; ++i) {
        children[i] = fork();

        if (children[i] == 0) {
            close(readyPipe[0]);

            const char ready = 'R';
            if (write(readyPipe[1], &ready, 1) != 1) {
                _exit(2);
            }
            close(readyPipe[1]);

            const bool result = appendTransaction(
                {1, "CREATE_FILE",
                 TransactionState::COMMITTED}, false);

            _exit(result ? 0 : 1);
        }

        if (children[i] < 0) {
            close(readyPipe[0]);
            close(readyPipe[1]);
            fs::current_path(originalDir);
            fs::remove_all(dir);
            return 1;
        }
    }

    close(readyPipe[1]);

    char ready[2];
    std::size_t received = 0;

    while (received < sizeof(ready)) {
        ssize_t n = read(readyPipe[0], ready + received,
                         sizeof(ready) - received);
        if (n <= 0) {
            break;
        }
        received += static_cast<std::size_t>(n);
    }

    close(readyPipe[0]);

    int successCount = 0;
    int failureCount = 0;

    for (pid_t child : children) {
        int status = 0;
        if (waitpid(child, &status, 0) == -1 ||
            !WIFEXITED(status)) {
            failureCount++;
            continue;
        }

        if (WEXITSTATUS(status) == 0) {
            successCount++;
        } else if (WEXITSTATUS(status) == 1) {
            failureCount++;
        } else {
            fs::current_path(originalDir);
            fs::remove_all(dir);
            return 1;
        }
    }

    const bool validResult =
        received == sizeof(ready) &&
        successCount == 1 &&
        failureCount == 1 &&
        isTransactionCompleted(1, "CREATE_FILE");

    fs::current_path(originalDir);
    fs::remove_all(dir);

    if (!validResult) {
        std::cerr << "FAIL: Concurrent journal transitions were not serialized\n";
        return 1;
    }

    std::cout << "PASS: Concurrent transitions serialized correctly\n";
    return 0;
}
