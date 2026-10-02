#include <iostream>
#include "journal.h"

int main() {
    Transaction transaction{
        1,
        "CREATE_FILE",
        TransactionState::STARTED
    };

    if (!appendTransaction(transaction)) {
        std::cerr << "Error: Could not write to journal.\n";
        return 1;
    }

    std::cout << "Transaction recorded successfully.\n";
    return 0;
}