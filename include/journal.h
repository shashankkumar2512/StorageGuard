
#ifndef JOURNAL_H
#define JOURNAL_H

#include <string>

enum class TransactionState {
    STARTED,
    COMMITTED,
    RECOVERY_REQUIRED
};

struct Transaction {
    int id;
    std::string operation;
    TransactionState state;
};

#endif
