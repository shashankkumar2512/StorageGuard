# StorageGuard

### Crash-Consistent Embedded Filesystem, Journaling & Recovery Analyzer

StorageGuard is a Linux-based C++17 academic project that demonstrates transaction journaling, incomplete transaction detection, crash-recovery simulation, journal validation, and communication with a Linux character device.

**Project status:** Automated application tests pass, and the kernel character device has passed a real user-space read/write round-trip test in WSL2.

## Features

- **Transaction Journaling:** Records transaction IDs, operation names, and transaction states.
- **Incomplete Transaction Detection:** Identifies transactions that require recovery.
- **Crash Simulation:** Tests recovery after simulated interruptions at multiple transaction stages.
- **Recovery and Rollback:** Cleans up controlled test data associated with incomplete transactions.
- **Journal Validation:** Rejects malformed entries and invalid transaction-state transitions.
- **Concurrent Journal Testing:** Tests competing transaction-state updates.
- **Character-Device Communication:** Demonstrates user-space and kernel-space communication through `/dev/storageguard`.
- **Automated Testing:** Includes journal, recovery, crash-recovery, concurrency, and driver-related tests.

## Technologies

- C++17
- C and Linux kernel module interfaces
- Ubuntu on WSL2
- GCC/G++, GNU Make
- Git and GitHub

## Project Structure

```text
StorageGuard/
├── include/       # Journal, recovery, and driver-client headers
├── src/           # Application, journaling, recovery, driver client
├── tests/         # Automated tests
├── driver/        # Linux character-device driver
├── test_data/     # Controlled test data
├── Makefile
├── .gitignore
└── README.md
```

## Prerequisites

- Ubuntu or another compatible Linux environment
- GCC/G++ with C++17 support
- GNU Make
- Linux kernel headers matching the running kernel for driver builds

Install the basic tools:

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

Run StorageGuard:

```bash
./storageguard
```

Inspect the journal, if generated:

```bash
cat storageguard.log
```

## Automated Tests

Run the application tests:

```bash
make test
```

Build and run the driver-client test:

```bash
make driver-test
```

The test suite covers journal state handling, incomplete transaction recovery, crash-recovery scenarios, malformed journal rejection, concurrent journal updates, and driver-related error handling.

## Kernel Character Device

The driver source is located in `driver/storageguard_driver.c`. When the module is built for a compatible kernel and loaded successfully, it exposes `/dev/storageguard`.

The current WSL2 environment has successfully loaded the driver, and a real read/write round-trip test verified that a message written from user space could be read back correctly.

Driver build instructions depend on the kernel build tree matching the running kernel. For example:

```bash
make -C /lib/modules/$(uname -r)/build M="$PWD/driver" modules
```

Loading kernel modules may require elevated privileges and a compatible WSL2 kernel configuration.

## Limitations

- StorageGuard simulates recovery in controlled test data; it does not repair an actual host filesystem.
- Crash tests simulate interruption at defined application stages rather than actual power loss.
- The character device is a prototype and uses a limited shared buffer.
- Recovery is not a guarantee against every possible crash or storage failure.
- Driver availability depends on kernel compatibility and environment configuration.

## Author

**Shashank Kumar**  
B.Tech, Computer Science and Engineering

## License

This project is intended for academic and educational purposes.
