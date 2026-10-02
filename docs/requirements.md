# StorageGuard - Project Requirements

## 1. Objective
Develop a Linux-based C++ application that demonstrates
transaction journaling, crash simulation, and recovery.

## 2. Functional Requirements
- FR1: Perform controlled file operations in a test directory.
- FR2: Record each operation in a transaction journal.
- FR3: Simulate interruption during an operation.
- FR4: Detect incomplete transactions after restart.
- FR5: Recover or roll back incomplete transactions.
- FR6: Validate consistency after recovery.
- FR7: Demonstrate Linux kernel/user-space communication
  through a character device driver.

## 3. Non-Functional Requirements
- NFR1: Run on Linux.
- NFR2: Use C++ for the application and C for the driver.
- NFR3: Avoid modifying important system files.
- NFR4: Provide clear error messages.
- NFR5: Include reproducible tests and documentation.

## 4. Scope
StorageGuard operates on its own test data.
It does not repair the host operating system's filesystem.

## 5. Tools
- Ubuntu on WSL2
- C++
- Linux system calls
- GNU Make
- Git and GitHub
- Linux kernel module tools
