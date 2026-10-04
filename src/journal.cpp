
#include "journal.h"
#include "driver_client.h"
#include <sys/file.h>
#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace {

const char* stateToString(TransactionState state) {
    switch (state) {
        case TransactionState::STARTED:
            return "STARTED";
        case TransactionState::COMMITTED:
            return "COMMITTED";
        case TransactionState::RECOVERY_REQUIRED:
            return "RECOVERY_REQUIRED";
        case TransactionState::RECOVERED:
            return "RECOVERED";
    }
    return nullptr;
}

bool validOperation(const std::string& operation) {
    return !operation.empty() &&
           operation.find('|') == std::string::npos &&
           operation.find('\n') == std::string::npos &&
           operation.find('\r') == std::string::npos;
}

bool validStateString(const std::string& state) {
    return state == "STARTED" ||
           state == "COMMITTED" ||
           state == "RECOVERY_REQUIRED" ||
           state == "RECOVERED";
}

bool readLatestState(int transactionId,
                     const std::string& operation,
                     std::string& latestState) {
    std::ifstream journal("storageguard.log");

    if (!journal.is_open()) {
        return false;
    }

    bool found = false;
    std::string line;

    while (std::getline(journal, line)) {
        std::stringstream ss(line);
        std::string idText, loggedOperation, state, extra;

        if (!std::getline(ss, idText, '|') ||
            !std::getline(ss, loggedOperation, '|') ||
            !std::getline(ss, state, '|') ||
            std::getline(ss, extra, '|')) {
            continue;
        }

        try {
            std::size_t parsed = 0;
            int id = std::stoi(idText, &parsed);

            if (parsed != idText.size() ||
                id != transactionId ||
                loggedOperation != operation ||
                !validStateString(state)) {
                continue;
            }

            latestState = state;
            found = true;
        } catch (...) {
            continue;
        }
    }

    return found;
}

bool validTransition(const std::string& previous,
                     TransactionState next) {
    if (previous == "STARTED") {
        return next == TransactionState::COMMITTED ||
               next == TransactionState::RECOVERY_REQUIRED;
    }

    if (previous == "RECOVERY_REQUIRED") {
        return next == TransactionState::RECOVERED;
    }

    // COMMITTED and RECOVERED are terminal states.
    return false;
}

bool canAppendTransaction(const Transaction& transaction) {
    std::string previousState;

    if (readLatestState(transaction.id,
                        transaction.operation,
                        previousState)) {
        return validTransition(previousState, transaction.state);
    }

    // A transaction without a valid previous state must start first.
    if (transaction.state != TransactionState::STARTED) {
        return false;
    }

    // Distinguish a missing journal from an existing unreadable one.
    if (access("storageguard.log", F_OK) == 0) {
        std::ifstream journal("storageguard.log");
        return journal.is_open();
    }

    return errno == ENOENT;
}

} // namespace


bool appendTransaction(const Transaction& transaction,
                       bool forwardToDriver) {
    const char* state = stateToString(transaction.state);

    if (transaction.id <= 0 ||
        state == nullptr ||
        !validOperation(transaction.operation)) {
        return false;
    }

    const std::string record =
        std::to_string(transaction.id) + "|" +
        transaction.operation + "|" + state + "\n";

    int fd = open("storageguard.log",
                  O_WRONLY | O_CREAT | O_APPEND,
                  0644);

    if (fd == -1) {
        std::cerr << "StorageGuard: journal open failed: "
                  << std::strerror(errno) << '\n';
        return false;
    }

    // Serialize validation and appending across processes.
    while (flock(fd, LOCK_EX) == -1) {
        if (errno == EINTR) {
            continue;
        }

        std::cerr << "StorageGuard: journal lock failed: "
                  << std::strerror(errno) << '\n';
        close(fd);
        return false;
    }

    if (!canAppendTransaction(transaction)) {
        std::cerr << "StorageGuard: invalid transaction state transition.\n";
        flock(fd, LOCK_UN);
        close(fd);
        return false;
    }

    std::size_t written = 0;

    while (written < record.size()) {
        ssize_t result = write(
            fd,
            record.data() + written,
            record.size() - written);

        if (result < 0 && errno == EINTR) {
            continue;
        }

        if (result <= 0) {
            std::cerr << "StorageGuard: journal write failed: "
                      << std::strerror(errno) << '\n';
            flock(fd, LOCK_UN);
            close(fd);
            return false;
        }

        written += static_cast<std::size_t>(result);
    }

    if (fsync(fd) == -1) {
        std::cerr << "StorageGuard: journal fsync failed: "
                  << std::strerror(errno) << '\n';
        flock(fd, LOCK_UN);
        close(fd);
        return false;
    }

    if (flock(fd, LOCK_UN) == -1) {
        std::cerr << "StorageGuard: journal unlock failed: "
                  << std::strerror(errno) << '\n';
        close(fd);
        return false;
    }

    if (close(fd) == -1) {
        std::cerr << "StorageGuard: journal close failed.\n";
        return false;
    }

    if (!forwardToDriver) {
        return true;
    }

    return sendToDriver(record.substr(0, record.size() - 1));
}


bool detectIncompleteTransaction(int transactionId) {
    if (transactionId <= 0) {
        return false;
    }

    std::ifstream journal("storageguard.log");

    if (!journal.is_open()) {
        return false;
    }

    bool found = false;
    std::string latestState;
    std::string line;

    while (std::getline(journal, line)) {
        std::stringstream ss(line);
        std::string idText, operation, state, extra;

        if (!std::getline(ss, idText, '|') ||
            !std::getline(ss, operation, '|') ||
            !std::getline(ss, state, '|') ||
            std::getline(ss, extra, '|') ||
            operation.empty() ||
            !validStateString(state)) {
            continue;
        }

        try {
            std::size_t parsed = 0;
            int id = std::stoi(idText, &parsed);

            if (parsed != idText.size() || id != transactionId) {
                continue;
            }

            latestState = state;
            found = true;
        } catch (...) {
            continue;
        }
    }

    return found &&
           (latestState == "STARTED" ||
            latestState == "RECOVERY_REQUIRED");
}

bool isTransactionCompleted(int transactionId,
                            const std::string& operation) {
    if (transactionId <= 0 || !validOperation(operation)) {
        return false;
    }

    std::string state;

    if (!readLatestState(transactionId, operation, state)) {
        return false;
    }

    return state == "COMMITTED" || state == "RECOVERED";
}

int getNextTransactionId() {
    std::ifstream journal("storageguard.log");
    std::string line;
    int maximumId = 0;

    while (std::getline(journal, line)) {
        std::stringstream ss(line);
        std::string idText, operation, state, extra;

        if (!std::getline(ss, idText, '|') ||
            !std::getline(ss, operation, '|') ||
            !std::getline(ss, state, '|') ||
            std::getline(ss, extra, '|')) {
            continue;
        }

        try {
            std::size_t parsed = 0;
            int id = std::stoi(idText, &parsed);

            if (parsed == idText.size() &&
                id > maximumId &&
                !operation.empty() &&
                validStateString(state)) {
                maximumId = id;
            }
        } catch (...) {
            continue;
        }
    }

    if (maximumId == INT_MAX) {
        return 0;
    }

    return maximumId + 1;
}
