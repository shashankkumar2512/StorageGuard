#include "recovery.h"
#include "journal.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace {

using Key = std::pair<int, std::string>;

bool validState(const std::string& state) {
    return state == "STARTED" ||
           state == "COMMITTED" ||
           state == "RECOVERY_REQUIRED" ||
           state == "RECOVERED";
}

bool validTransition(const std::string& previous,
                     const std::string& next) {
    if (previous.empty()) {
        return next == "STARTED";
    }

    if (previous == "STARTED") {
        return next == "COMMITTED" ||
               next == "RECOVERY_REQUIRED";
    }

    if (previous == "RECOVERY_REQUIRED") {
        return next == "RECOVERED";
    }

    return false;
}

bool readLatestStates(std::map<Key, std::string>& latest) {
    latest.clear();

    std::ifstream journal("storageguard.log");
    if (!journal.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(journal, line)) {
        std::stringstream ss(line);
        std::string idText, operation, state, extra;

        if (!std::getline(ss, idText, '|') ||
            !std::getline(ss, operation, '|') ||
            !std::getline(ss, state, '|') ||
            std::getline(ss, extra, '|') ||
            operation.empty() ||
            operation.find_first_of("|\n\r") != std::string::npos ||
            !validState(state)) {
            return false;
        }

        int id;
        try {
            std::size_t parsed = 0;
            id = std::stoi(idText, &parsed);

            if (parsed != idText.size() || id <= 0) {
                return false;
            }
        } catch (...) {
            return false;
        }

        const Key key{id, operation};
        const auto it = latest.find(key);
        const std::string previous =
            it == latest.end() ? "" : it->second;

        if (!validTransition(previous, state)) {
            return false;
        }

        latest[key] = state;
    }

    return !journal.bad();
}

} // namespace

bool recoverTransaction(int transactionId,
                        const std::string& operation,
                        const std::string& filePath,
                        bool forwardToDriver) {
    if (transactionId <= 0 || operation.empty() || filePath.empty()) {
        return false;
    }

    std::map<Key, std::string> latest;
    if (!readLatestStates(latest)) {
        return false;
    }

    const auto it = latest.find({transactionId, operation});
    if (it == latest.end()) {
        return false;
    }

    const std::string state = it->second;

    if (state == "COMMITTED" || state == "RECOVERED") {
        return true;
    }

    if (state != "STARTED" && state != "RECOVERY_REQUIRED") {
        return false;
    }

    if (state == "STARTED" &&
        !appendTransaction(
            {transactionId, operation,
             TransactionState::RECOVERY_REQUIRED},
            forwardToDriver)) {
        return false;
    }

    std::error_code error;
    fs::remove(filePath, error);

    if (error || fs::exists(filePath)) {
        return false;
    }

    return appendTransaction(
        {transactionId, operation, TransactionState::RECOVERED},
        forwardToDriver);
}

bool recoverPendingTransactions(const std::string& operation,
                                const std::string& filePath,
                                int& recoveredCount,
                                bool forwardToDriver) {
    recoveredCount = 0;

    if (operation.empty() || filePath.empty()) {
        return false;
    }

    std::map<Key, std::string> latest;
    if (!readLatestStates(latest)) {
        return false;
    }

    for (const auto& entry : latest) {
        if (entry.first.second != operation ||
            (entry.second != "STARTED" &&
             entry.second != "RECOVERY_REQUIRED")) {
            continue;
        }

        if (!recoverTransaction(entry.first.first, operation,
                                filePath, forwardToDriver)) {
            return false;
        }

        ++recoveredCount;
    }

    return true;
}
