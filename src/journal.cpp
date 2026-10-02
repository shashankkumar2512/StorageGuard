#include "journal.h"
#include <fstream>
#include <sstream>
#include <iostream>

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
        case TransactionState::RECOVERED:
            state = "RECOVERED";
            break;
    }

    journal << transaction.id << '|'
            << transaction.operation << '|'
            << state << '\n';

    journal.flush();
    return journal.good();
}

bool detectIncompleteTransaction(int transactionId) {
    std::ifstream journal("storageguard.log");

    if (!journal.is_open()) {
        return false;
    }

    std::string line;
    bool started = false;
    bool committed = false;

    while (std::getline(journal, line)) {
        std::stringstream ss(line);
        std::string idText;
        std::string operation;
        std::string state;

        std::getline(ss, idText, '|');
        std::getline(ss, operation, '|');
        std::getline(ss, state, '|');

        try {
            if (std::stoi(idText) != transactionId) {
                continue;
            }
        } catch (...) {
            continue;
        }

        if (state == "STARTED") {
            started = true;
            committed = false;
        } else if (state == "COMMITTED") {
            committed = true;
        }
    }

    return started && !committed;
}