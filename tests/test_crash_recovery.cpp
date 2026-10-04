
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

static int runProcess(const fs::path& executable,
                      const fs::path& workingDir,
                      const std::vector<std::string>& arguments) {
    const pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        if (chdir(workingDir.c_str()) != 0) {
            _exit(120);
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(executable.c_str()));

        std::vector<std::string> args = arguments;
        for (auto& arg : args) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);

        execv(executable.c_str(), argv.data());
        perror("execv");
        _exit(121);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

static bool containsText(const fs::path& filePath,
                         const std::string& expected) {
    std::ifstream file(filePath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.find(expected) != std::string::npos) {
            return true;
        }
    }

    return false;
}

static bool runScenario(const fs::path& executable,
                        const fs::path& root,
                        const std::string& stage) {
    const fs::path dir = root / stage;
    fs::create_directories(dir);

    const fs::path sampleFile = dir / "test_data/sample.txt";
    const fs::path journal = dir / "storageguard.log";

    int code = runProcess(
        executable, dir,
        {"--crash-at", stage, "--no-driver"});

    if (code != 86) {
        std::cerr << "FAIL: " << stage
                  << " did not exit with crash code 86 (got "
                  << code << ").\n";
        return false;
    }

    if (!containsText(journal, "STARTED")) {
        std::cerr << "FAIL: STARTED entry missing for "
                  << stage << ".\n";
        return false;
    }

    if (stage == "after-recovery-required" &&
        !containsText(journal, "RECOVERY_REQUIRED")) {
        std::cerr << "FAIL: RECOVERY_REQUIRED entry missing.\n";
        return false;
    }

    if (stage != "after-start" && !fs::exists(sampleFile)) {
        std::cerr << "FAIL: Partial file missing before recovery.\n";
        return false;
    }

    code = runProcess(executable, dir, {"--no-driver"});

    if (code != 0) {
        std::cerr << "FAIL: Restart recovery returned "
                  << code << " for " << stage << ".\n";
        return false;
    }

    if (fs::exists(sampleFile)) {
        std::cerr << "FAIL: Incomplete file still exists after recovery.\n";
        return false;
    }

    if (!containsText(journal, "RECOVERED")) {
        std::cerr << "FAIL: RECOVERED entry missing for "
                  << stage << ".\n";
        return false;
    }

    code = runProcess(executable, dir, {"--no-driver"});
    if (code != 0) {
        std::cerr << "FAIL: Repeated recovery failed for "
                  << stage << ".\n";
        return false;
    }

    std::cout << "PASS: " << stage
              << " crash, restart, and idempotent recovery.\n";
    return true;
}

int main() {
    const fs::path executable = fs::absolute("./storageguard");

    if (!fs::exists(executable)) {
        std::cerr << "Build storageguard first with make.\n";
        return 1;
    }

    const fs::path root =
        fs::temp_directory_path() /
        ("storageguard-crash-" + std::to_string(getpid()));

    fs::create_directories(root);

    bool passed = true;
    passed &= runScenario(executable, root, "after-start");
    passed &= runScenario(executable, root, "after-write");
    passed &= runScenario(
        executable, root, "after-recovery-required");

    std::error_code ec;
    fs::remove_all(root, ec);

    if (!passed) {
        return 1;
    }

    std::cout << "All crash-recovery integration tests passed.\n";
    return 0;
}
