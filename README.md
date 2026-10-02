# StorageGuard
### Linux-Based Crash-Consistency and Filesystem Recovery Analyzer

StorageGuard is a C++17 project that demonstrates transaction journaling, incomplete-operation detection, and recovery simulation in a Linux environment. It records file-operation states in a journal and demonstrates how an interrupted operation can be detected and handled in a controlled test directory.

## Features

- **Transaction Journaling:** Records transaction IDs, operations, and states in a log file.
- **Incomplete Transaction Detection:** Identifies transactions that started but have no subsequent commit record.
- **Recovery Simulation:** Demonstrates cleanup of a simulated partially created file.
- **Recovery Status Tracking:** Records transaction states such as `STARTED`, `COMMITTED`, `RECOVERY_REQUIRED`, and `RECOVERED`.
- **Automated Tests:** Tests committed and incomplete transaction detection.
- **Linux Development:** Uses standard C++17 and GNU Make in a Linux environment.

## Technologies Used

- Language: C++17
- Operating System: Linux (developed in Ubuntu on WSL2)
- Compiler: GCC / G++
- Build Tool: GNU Make
- Version Control: Git and GitHub

## Project Structure

```text
StorageGuard/
├── include/
│   └── journal.h
├── src/
│   ├── main.cpp
│   └── journal.cpp
├── tests/
│   └── test_journal.cpp
├── docs/
├── driver/
├── test_data/
├── Makefile
├── .gitignore
└── README.md
```

## How It Works

1. A transaction is recorded with the `STARTED` state.
2. The program records `COMMITTED` when an operation is considered complete.
3. The journal is checked for transactions that started but were not committed.
4. The program demonstrates a recovery action on controlled test data.
5. The outcome is recorded in the journal.

### Example Journal

```text
100|CREATE_FILE|STARTED
100|CREATE_FILE|RECOVERED
```

Each record contains a transaction ID, operation name, and transaction state, separated by the `|` character.

## Prerequisites

- A Linux environment, such as Ubuntu.
- GCC/G++ with C++17 support.
- GNU Make.
- Git (optional, for version control).

On Ubuntu, install the basic build tools with:

```bash
sudo apt update
sudo apt install build-essential git
```

## Build and Run

Clone the repository:

```bash
git clone https://github.com/shashankkumar2512/StorageGuard.git
cd StorageGuard
```

Compile the application:

```bash
make
```

Run StorageGuard:

```bash
./storageguard
```

View the transaction journal:

```bash
cat storageguard.log
```

## Run Automated Tests

Build and execute the journal tests:

```bash
make test
```

The tests verify that:
- A committed transaction is not identified as incomplete.
- An uncommitted transaction is identified as incomplete.

Expected test output:

```text
PASS: Committed transaction detected correctly.
PASS: Incomplete transaction detected correctly.
```

## Clean Build Files

Remove the generated application and test binaries:

```bash
make clean
```

## Limitations

- This project currently demonstrates application-level journaling and recovery simulation.
- It does not repair or recover the actual Linux filesystem.
- The recovery simulation is limited to controlled test data and does not guarantee recovery from every failure scenario.
- The current tests validate transaction detection, not real power-loss or kernel-crash behavior.
- A Linux character device driver component is planned separately and must be implemented and tested before claiming device-driver functionality.

## Future Improvements

- Improve journal parsing and transaction-state validation.
- Add more automated tests for malformed logs and repeated transactions.
- Improve recovery handling and error reporting.
- Add a Linux character device driver to demonstrate communication between user space and kernel space.
- Document driver build instructions, testing procedures, and architecture once implemented.

## Author

**Shashank Kumar**  
B.Tech – Computer Science and Engineering

## License

This project is intended for academic and educational purposes.
