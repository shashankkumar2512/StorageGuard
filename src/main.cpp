
#include "journal.h"
#include "recovery.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

static constexpr const char* OPERATION = "DEMO_CREATE_FILE";
static constexpr const char* FILE_PATH = "test_data/sample.txt";
static constexpr int CRASH_EXIT_CODE = 86;

[[noreturn]] static void simulateAbruptExit(const std::string& stage) {
    std::cout << "Simulated process crash at: " << stage << '\n'
              << std::flush;
    _exit(CRASH_EXIT_CODE);
}

static int simulateCrash(const std::string& stage, bool forwardToDriver) {
    if (stage != "after-start" &&
        stage != "after-write" &&
        stage != "after-recovery-required") {
        std::cerr << "Invalid crash stage.\n"
                  << "Use: after-start, after-write, "
                     "after-recovery-required\n";
        return 2;
    }

    std::error_code ec;
    fs::create_directories("test_data", ec);
    if (ec) {
        std::cerr << "Could not create test_data directory.\n";
        return 1;
    }

    const int id = getNextTransactionId();
    if (id <= 0) {
        std::cerr << "Could not allocate transaction ID.\n";
        return 1;
    }

    if (!appendTransaction(
            {id, OPERATION, TransactionState::STARTED},
            forwardToDriver)) {
        std::cerr << "Failed to record transaction start.\n";
        return 1;
    }

    if (stage == "after-start") {
        simulateAbruptExit(stage);
    }

    {
        std::ofstream file(FILE_PATH, std::ios::trunc);
        if (!file) {
            std::cerr << "Could not create sample file.\n";
            return 1;
        }

        file << "PARTIAL DATA";
        file.flush();

        if (!file) {
            std::cerr << "Could not write sample file.\n";
            return 1;
        }
    }

    if (stage == "after-write") {
        simulateAbruptExit(stage);
    }

    if (!appendTransaction(
            {id, OPERATION, TransactionState::RECOVERY_REQUIRED},
            forwardToDriver)) {
        std::cerr << "Failed to record recovery-required state.\n";
        return 1;
    }

    simulateAbruptExit(stage);
}

static int recoverDemo(bool forwardToDriver) {
    int recoveredCount = 0;

    if (!recoverPendingTransactions(
            OPERATION, FILE_PATH, recoveredCount, forwardToDriver)) {
        std::cerr << "Recovery failed.\n";
        return 1;
    }

    if (recoveredCount == 0) {
        std::cout << "No pending demo transactions. System is clean.\n";
    } else {
        std::cout << "Recovered " << recoveredCount
                  << " demo transaction(s).\n";
    }

    return 0;
}

int main(int argc, char* argv[]) {
    bool forwardToDriver = true;
    std::string crashStage;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--no-driver") {
            forwardToDriver = false;
        } else if (arg == "--simulate-crash") {
            crashStage = "after-write";
        } else if (arg == "--crash-at") {
            if (i + 1 >= argc) {
                std::cerr << "Missing crash stage.\n";
                return 2;
            }
            crashStage = argv[++i];
        } else {
            std::cerr << "Usage: ./storageguard "
                         "[--no-driver] "
                         "[--simulate-crash | --crash-at STAGE]\n";
            return 2;
        }
    }

    if (!crashStage.empty()) {
        return simulateCrash(crashStage, forwardToDriver);
    }

    return recoverDemo(forwardToDriver);
}
