#include "journal.h"
#include "recovery.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

static bool check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }

    std::cout << "PASS: " << message << '\n';
    return true;
}

int main()
{
    const fs::path original = fs::current_path();
    const fs::path temporary =
        fs::temp_directory_path() /
        ("storageguard-recovery-" + std::to_string(getpid()));

    std::error_code error;
    fs::remove_all(temporary, error);
    fs::create_directories(temporary / "test_data");
    fs::current_path(temporary);

    bool ok = true;

    // Test 1: Recover a partially created file.
    {
        std::ofstream file("test_data/partial.txt");
        file << "PARTIAL DATA";
    }

    ok &= check(
        appendTransaction(
            {101, "TEST_CREATE", TransactionState::STARTED}, false),
        "Started transaction recorded");

    ok &= check(
        recoverTransaction(101, "TEST_CREATE",
                           "test_data/partial.txt", false),
        "Interrupted file operation recovered");

    ok &= check(
        !fs::exists("test_data/partial.txt"),
        "Partial file removed");

    ok &= check(
        !detectIncompleteTransaction(101) &&
            isTransactionCompleted(101, "TEST_CREATE"),
        "Recovered transaction is complete");

    // Test 2: Repeating recovery must preserve the completed state.
    ok &= check(
        recoverTransaction(101, "TEST_CREATE",
                           "test_data/partial.txt", false),
        "Repeated recovery succeeds");

    ok &= check(
        !detectIncompleteTransaction(101) &&
            isTransactionCompleted(101, "TEST_CREATE"),
        "Repeated recovery leaves journal state unchanged");

    // Test 3: A committed file must not be removed.
    {
        std::ofstream file("test_data/committed.txt");
        file << "VALID DATA";
    }

    ok &= check(
        appendTransaction(
            {102, "TEST_CREATE", TransactionState::STARTED}, false) &&
            appendTransaction(
                {102, "TEST_CREATE", TransactionState::COMMITTED}, false),
        "Committed transaction recorded");

    ok &= check(
        recoverTransaction(102, "TEST_CREATE",
                           "test_data/committed.txt", false),
        "Committed transaction left untouched");

    std::ifstream committed("test_data/committed.txt");
    std::string contents;
    std::getline(committed, contents);

    ok &= check(
        fs::exists("test_data/committed.txt") &&
            contents == "VALID DATA",
        "Committed file contents preserved");

    // Test 4: Corrupted journal must not trigger recovery.
    {
        std::ofstream file("test_data/corrupted.txt");
        file << "IMPORTANT DATA";
    }

    {
        std::ofstream journal("storageguard.log", std::ios::app);
        journal << "103|TEST_CREATE|STARTED\n";
        journal << "CORRUPTED JOURNAL RECORD\n";
    }

    ok &= check(
        !recoverTransaction(103, "TEST_CREATE",
                            "test_data/corrupted.txt", false),
        "Recovery rejects a corrupted journal");

    ok &= check(
        fs::exists("test_data/corrupted.txt"),
        "File remains untouched when journal integrity is uncertain");

    fs::current_path(original);
    fs::remove_all(temporary, error);

    if (!ok)
    {
        return 1;
    }

    std::cout << "All end-to-end recovery tests passed.\n";
    return 0;
}
