# 🛡️ StorageGuard

### Crash-Consistent Embedded Filesystem, Journaling & Recovery Analyzer

**Explore transaction journaling. Simulate crashes. Validate recovery. Understand the Linux kernel.**

[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20WSL2-blue?logo=linux)](https://www.kernel.org/)
[![Language](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus)](https://isocpp.org/)
[![Kernel](https://img.shields.io/badge/Kernel-C%20Driver-orange?logo=linux)](https://www.kernel.org/)
[![Build](https://img.shields.io/badge/Build-GNU%20Make-green)](https://www.gnu.org/software/make/)

StorageGuard is a Linux-based educational project that demonstrates how transaction journals can help identify incomplete operations and support controlled recovery after simulated interruptions. It combines a C++17 user-space application with a Linux character-device driver prototype.

> **Project goal:** Make crash-recovery concepts observable, testable, and easier to understand.

## ✨ Key Features

- 📝 **Transaction journaling** — records transaction IDs, operation names, and state transitions.
- 🔍 **Incomplete transaction detection** — identifies operations that require recovery.
- 🔄 **Recovery simulation** — cleans up controlled test data associated with incomplete transactions.
- 🧪 **Crash-point testing** — exercises recovery after simulated interruptions at defined transaction stages.
- 🛡️ **Journal validation** — rejects malformed records and invalid state transitions.
- ⚡ **Concurrency testing** — checks competing transaction-state updates.
- 🔌 **Kernel communication** — demonstrates user-space read/write communication through a Linux character device.
- 📋 **Automated tests** — validates journaling, recovery, crash scenarios, and driver-related behavior.

## 🏗️ Architecture

```text
       USER SPACE
┌──────────────────────────────┐
│     StorageGuard App         │
│           C++17              │
└──────────────┬───────────────┘
               │
       ┌───────┴────────┐
       ▼                ▼
┌─────────────┐  ┌──────────────┐
│ Transaction │  │   Recovery   │
│   Journal   │  │    Engine    │
└──────┬──────┘  └──────┬───────┘
       └────────┬────────┘
                ▼
       Controlled Test Data

       USER / KERNEL BOUNDARY
┌──────────────────────────────┐
│      Driver Client           │
└──────────────┬───────────────┘
               ▼
┌──────────────────────────────┐
│ Linux Character Device       │
│       /dev/storageguard      │
└──────────────────────────────┘

       TESTING LAYER
┌──────────────────────────────┐
│ Journal • Recovery • Crash   │
│ Concurrency • Driver Tests   │
└──────────────────────────────┘
```

*The diagram represents the project's logical components; the driver prototype is separate from the simulated recovery engine.*

## 🔁 Transaction Lifecycle

```text
STARTED
   │
   ├── Operation succeeds ──► COMMITTED
   │
   └── Interruption ────────► RECOVERY_REQUIRED
                                      │
                                      ▼
                                  RECOVERED
```

| State | Meaning |
|---|---|
| `STARTED` | A transaction has begun. |
| `COMMITTED` | The operation completed successfully. |
| `RECOVERY_REQUIRED` | An incomplete operation requires recovery. |
| `RECOVERED` | The simulated recovery process completed. |

## 🧰 Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Application, journal, and recovery logic |
| C | Linux character-device driver |
| Linux / WSL2 | Development and execution environment |
| GNU Make | Build and test automation |
| Git & GitHub | Version control and collaboration |

## 🚀 Getting Started

### 1. Prerequisites

- Linux or Ubuntu on WSL2
- GCC/G++ with C++17 support
- GNU Make
- Git
- Matching kernel build headers if building the driver module

On Ubuntu, install the basic tools:

```bash
sudo apt update
sudo apt install build-essential git
```

### 2. Clone the repository

```bash
git clone https://github.com/shashankkumar2512/StorageGuard.git
cd StorageGuard
```

### 3. Build the application

```bash
make
```

### 4. Run StorageGuard

```bash
./storageguard
```

Inspect the journal if the application has generated it:

```bash
cat storageguard.log
```

## 🧪 Run the Tests

Run the automated application test suite:

```bash
make test
```

Run the driver-client test target:

```bash
make driver-test
```

The project's reported passing tests cover:

- Journal state handling
- Recovery and incomplete transactions
- Malformed journal rejection
- Simulated crash-recovery scenarios
- Concurrent transaction-state updates
- Driver-related failure handling

A separate real-device test also successfully wrote a message to `/dev/storageguard` and read the same message back in WSL2.

## 🔌 Linux Character-Device Driver

The driver source is located in `driver/storageguard_driver.c`. In a compatible environment, it exposes a character device at:

```text
/dev/storageguard
```

The module must be built against a kernel build tree compatible with the running kernel. Loading kernel modules may require elevated privileges and suitable WSL2 kernel support.

**Verification note:** Successful read/write round-trip communication has been demonstrated in the development environment. Driver behavior and availability may differ on other kernels or machines.

## 📁 Repository Layout

```text
StorageGuard/
├── include/       # Public headers
├── src/           # Application, journaling, recovery, driver client
├── tests/         # Automated tests
├── driver/        # Linux character-device driver
├── test_data/     # Controlled test data
├── Makefile       # Build and test targets
├── .gitignore
└── README.md
```

## ⚠️ Scope and Limitations

- StorageGuard is an educational analyzer and recovery simulation, not a production filesystem-repair tool.
- Recovery is restricted to controlled test data and does not repair the host filesystem.
- Simulated crash tests do not reproduce every real power-loss or kernel-crash condition.
- Recovery operations and journal updates are not guaranteed to be atomic under every possible interruption.
- The character-device driver is a prototype, not a production storage driver.

## 🎯 Future Improvements

- Make recovery and journal updates robust against interruption during recovery itself.
- Expand fault-injection and consistency tests.
- Improve crash-durability guarantees and transaction semantics.
- Add more detailed diagnostic and recovery reports.
- Test across additional supported Linux kernel configurations.

## 👨‍💻 Author

**Shashank Kumar**  
B.Tech — Computer Science and Engineering

🔗 **GitHub:** [shashankkumar2512](https://github.com/shashankkumar2512)

## 📄 License

No license is specified here. Add a repository license if you intend to grant others explicit permission to use, modify, or distribute the project.

---

<p align="center"><b>StorageGuard</b> · Explore failures. Validate recovery. Learn systems programming.</p>