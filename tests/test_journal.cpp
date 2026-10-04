
#include "journal.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

static int failures = 0;

static void check(bool condition, const std::string& message) {
    if (condition) {
        std::cout << "PASS: " << message << '\n';
    } else {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

int main() {
    char tempTemplate[] = "/tmp/storageguard-journal-XXXXXX";
    char* tempDir = mkdtemp(tempTemplate);

    if (!tempDir) {
        std::cerr << "Could not create temporary test directory.\n";
        return 1;
    }

    const fs::path originalDir = fs::current_path();
    fs::current_path(tempDir);

    // Valid initial state.
    check(appendTransaction(
              {1, "CREATE_FILE", TransactionState::STARTED}, false),
          "New transaction starts");

    // Allowed transitions.
    check(appendTransaction(
              {1, "CREATE_FILE", TransactionState::COMMITTED}, false),
          "STARTED to COMMITTED");

    check(appendTransaction(
              {2, "CREATE_FILE", TransactionState::STARTED}, false),
          "Second transaction starts");

    check(appendTransaction(
              {2, "CREATE_FILE", TransactionState::RECOVERY_REQUIRED}, false),
          "STARTED to RECOVERY_REQUIRED");

    check(appendTransaction(
              {2, "CREATE_FILE", TransactionState::RECOVERED}, false),
          "RECOVERY_REQUIRED to RECOVERED");

    // Terminal states must not transition again.
    check(!appendTransaction(
              {1, "CREATE_FILE", TransactionState::RECOVERY_REQUIRED}, false),
          "Reject transition from COMMITTED");

    check(!appendTransaction(
              {2, "CREATE_FILE", TransactionState::STARTED}, false),
          "Reject transition from RECOVERED");

    check(!appendTransaction(
              {99, "CREATE_FILE", TransactionState::RECOVERED}, false),
          "Reject non-STARTED state for unknown transaction");

    // Same transaction ID can be used for a different operation.
    check(appendTransaction(
              {1, "DELETE_FILE", TransactionState::STARTED}, false),
          "Different operation with same transaction ID");

    check(appendTransaction(
              {1, "DELETE_FILE", TransactionState::RECOVERY_REQUIRED}, false),
          "Independent operation has its own state");

    // Invalid transaction data.
    check(!appendTransaction(
              {0, "CREATE_FILE", TransactionState::STARTED}, false),
          "Reject zero transaction ID");

    check(!appendTransaction(
              {3, "", TransactionState::STARTED}, false),
          "Reject empty operation");

    check(!appendTransaction(
              {3, "BAD|OPERATION", TransactionState::STARTED}, false),
          "Reject operation containing delimiter");

    // Invalid journal lines should not be interpreted as valid states.
    {
        std::ofstream journal("storageguard.log", std::ios::app);
        journal << "not-a-valid-record\n";
        journal << "abc|CREATE_FILE|STARTED\n";
        journal << "3|CREATE_FILE|UNKNOWN_STATE\n";
        journal << "4|CREATE_FILE|STARTED|EXTRA\n";
    }

    check(appendTransaction(
              {3, "CORRUPTION_TEST", TransactionState::STARTED}, false),
          "Malformed records ignored");

    check(detectIncompleteTransaction(3),
        "Started transaction detected as incomplete");

    check(isTransactionCompleted(1, "CREATE_FILE"),
          "Committed transaction is complete");

    check(isTransactionCompleted(2, "CREATE_FILE"),
          "Recovered transaction is complete");

    check(!isTransactionCompleted(3, "CORRUPTION_TEST"),
          "Started transaction is not complete");
              // A truncated final journal record must not be treated as valid.
    {
        std::ofstream journal("storageguard.log", std::ios::app);
        journal << "50|TRUNCATED";
    }

    check(!isTransactionCompleted(50, "TRUNCATED"),
          "Truncated final record is not treated as completed");

    check(getNextTransactionId() > 0,
          "Next transaction ID remains positive with a truncated record");

    fs::current_path(originalDir);

    std::error_code ec;
    fs::remove_all(tempDir, ec);

    if (failures != 0) {
        std::cerr << failures << " journal test(s) failed.\n";
        return 1;
    }

    std::cout << "All journal tests passed.\n";
    return 0;
}
