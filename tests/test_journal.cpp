#include <iostream>
#include <cstdio>
#include "journal.h"

int main() {
    std::remove("storageguard.log");

    Transaction started1{1, "CREATE_FILE", TransactionState::STARTED};
    Transaction committed1{1, "CREATE_FILE", TransactionState::COMMITTED};

    Transaction started2{2, "DELETE_FILE", TransactionState::STARTED};

    Transaction started3{3, "UPDATE_FILE", TransactionState::STARTED};
    Transaction recovered3{3, "UPDATE_FILE", TransactionState::RECOVERED};

    if (!appendTransaction(started1) ||
        !appendTransaction(committed1) ||
        !appendTransaction(started2) ||
        !appendTransaction(started3) ||
        !appendTransaction(recovered3)) {
        std::cerr << "FAIL: Could not write journal.\n";
        return 1;
    }

    if (detectIncompleteTransaction(1)) {
        std::cerr << "FAIL: Committed transaction marked incomplete.\n";
        return 1;
    }

    if (!detectIncompleteTransaction(2)) {
        std::cerr << "FAIL: Incomplete transaction not detected.\n";
        return 1;
    }

    if (detectIncompleteTransaction(3)) {
        std::cerr << "FAIL: Recovered transaction marked incomplete.\n";
        return 1;
    }

    std::cout << "PASS: Committed transaction detected correctly.\n";
    std::cout << "PASS: Incomplete transaction detected correctly.\n";
    std::cout << "PASS: Recovered transaction detected correctly.\n";

    return 0;
}