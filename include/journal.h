
#ifndef JOURNAL_H
#define JOURNAL_H

#include <string>

enum class TransactionState {
    STARTED,
    COMMITTED,
    RECOVERY_REQUIRED,
    RECOVERED
};

struct Transaction {
    int id;
    std::string operation;
    TransactionState state;
};

bool appendTransaction(const Transaction& transaction,
                       bool forwardToDriver = true);

bool detectIncompleteTransaction(int transactionId);

bool isTransactionCompleted(int transactionId,
                            const std::string& operation);

int getNextTransactionId();

#endif
