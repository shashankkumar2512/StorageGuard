
# StorageGuard
### Linux-Based Crash-Consistency, Journaling and Filesystem Recovery Analyzer

StorageGuard is a C++17 academic project that demonstrates transaction journaling, incomplete-operation detection, and controlled recovery simulation in Linux. It also includes a Linux character-device driver prototype intended to demonstrate basic user-space and kernel-space communication.

**Project status:** The user-space journal simulation and its three automated tests have been verified. The kernel driver builds, but loading it has not been verified successfully because of a kernel symbol-version mismatch.

## Features

- **Transaction Journaling:** Records transaction IDs, operation names, and states in a journal.
- **Incomplete Transaction Detection:** Identifies transactions that started but lack a subsequent completion record.
- **Recovery Simulation:** Demonstrates cleanup of a simulated partially created file in a controlled test directory.
- **Recovery Status Tracking:** Supports `STARTED`, `COMMITTED`, `RECOVERY_REQUIRED`, and `RECOVERED` states.
- **Automated Tests:** Tests committed, incomplete, and recovered transaction detection.
- **Character-Device Driver Prototype:** Includes source code for a Linux character device with basic read/write operations.
- **Linux Development:** Uses C++17, GNU Make, and Linux kernel module interfaces.

## Technologies Used

- C++17
- Linux / Ubuntu on WSL2
- GCC/G++
- GNU Make
- Linux kernel module interfaces (C)
- Git and GitHub

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
├── driver/
│   ├── Makefile
│   └── storageguard_driver.c
├── test_data/
├── Makefile
├── .gitignore
└── README.md
```

## How It Works

1. A transaction is recorded with the `STARTED` state.
2. A completed operation is recorded with the `COMMITTED` state.
3. The journal is scanned for transactions that started but have no later completion or recovery record.
4. The application simulates an interrupted operation using controlled test data.
5. The simulation removes the partial test file and records the `RECOVERED` state.

Example journal records:

```text
100|CREATE_FILE|STARTED
100|CREATE_FILE|RECOVERED
```

Each record contains a transaction ID, operation name, and state separated by `|`.

## Prerequisites

- Linux or Ubuntu on WSL2
- GCC/G++ with C++17 support
- GNU Make
- Git (for cloning the repository)

Install the basic tools on Ubuntu:

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

Build the application:

```bash
make
```

Run the simulation:

```bash
./storageguard
```

View the journal:

```bash
cat storageguard.log
```

## Automated Tests

Run:

```bash
make test
```

The tests verify:

- A committed transaction is not reported as incomplete.
- An incomplete transaction is detected.
- A recovered transaction is not reported as incomplete.

Expected output:

```text
PASS: Committed transaction detected correctly.
PASS: Incomplete transaction detected correctly.
PASS: Recovered transaction detected correctly.
```

## Driver Prototype

The `driver/` directory contains a Linux character-device driver prototype.

Build instructions, for a compatible configured kernel build tree:

```bash
make -C /path/to/kernel/build M="$PWD/driver" modules
```

The kernel build tree must match the running kernel, including its configuration and symbol-version information. The module has previously built but failed to load in the current WSL2 environment because of a `module_layout` symbol-version mismatch.

**Driver status:** Successful loading and read/write testing remain unverified. The current user-space application does not communicate with the driver.

## Limitations

- This project demonstrates application-level journaling and simulated recovery, not actual filesystem repair.
- Recovery is limited to controlled test data.
- Tests validate journal-state detection; they do not simulate actual power loss or kernel crashes.
- The driver is a prototype and has not been successfully loaded and tested in the current environment.
- The project does not guarantee recovery from real filesystem failures.

## Future Improvements

- Add tests for malformed journal entries and repeated transaction IDs.
- Improve transaction-state validation and recovery error handling.
- Resolve kernel module compatibility and verify driver loading.
- Add and test user-space communication with the character device.
- Extend the simulation with more realistic crash scenarios.

## Author

**Shashank Kumar**  
B.Tech – Computer Science and Engineering

## License

This project is intended for academic and educational purposes.
