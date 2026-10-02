
#include <iostream>
#include <fstream>
#include <filesystem>
#include "journal.h"

namespace fs = std::filesystem;

int main() {
    fs::create_directories("test_data");

    const std::string filePath = "test_data/sample.txt";
    const int transactionId = 100;

    // Start the transaction.
    Transaction started{
        transactionId,
        "CREATE_FILE",
        TransactionState::STARTED
    };

    if (!appendTransaction(started)) {
        std::cerr << "Failed to record transaction start.\n";
        return 1;
    }

    // Simulate a partially created file.
    {
        std::ofstream file(filePath);
        if (!file) {
            std::cerr << "Could not create test file.\n";
            return 1;
        }
        file << "PARTIAL DATA";
    }

    std::cout << "Simulated interruption after partial file creation.\n";

    // Detect the incomplete transaction.
    if (!detectIncompleteTransaction(transactionId)) {
        std::cerr << "Expected incomplete transaction.\n";
        return 1;
    }

    std::cout << "Incomplete transaction detected.\n";

    // Recovery policy for this controlled CREATE_FILE test:
    // remove the partial file.
    std::error_code error;
    fs::remove(filePath, error);

    if (error) {
        std::cerr << "Recovery failed: " << error.message() << '\n';
        return 1;
    }

    if (fs::exists(filePath)) {
        std::cerr << "Partial file still exists.\n";
        return 1;
    }

    Transaction recovered{
        transactionId,
        "CREATE_FILE",
        TransactionState::RECOVERED
    };

    if (!appendTransaction(recovered)) {
        std::cerr << "Failed to record recovery status.\n";
        return 1;
    }

    std::cout << "Recovery test passed: partial file removed.\n";
    return 0;
}
