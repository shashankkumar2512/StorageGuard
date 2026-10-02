#include "journal.h"
#include <fstream>

bool appendTransaction(const Transaction& transaction) {
    std::ofstream journal("storageguard.log", std::ios::app);

    if (!journal.is_open()) {
        return false;
    }

    const char* state = "UNKNOWN";

    switch (transaction.state) {
        case TransactionState::STARTED:
            state = "STARTED";
            break;
        case TransactionState::COMMITTED:
            state = "COMMITTED";
            break;
        case TransactionState::RECOVERY_REQUIRED:
            state = "RECOVERY_REQUIRED";
            break;
    }

    journal << transaction.id << '|'
            << transaction.operation << '|'
            << state << '\n';

    journal.flush();
    return journal.good();
}