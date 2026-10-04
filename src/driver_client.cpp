#include "driver_client.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

bool sendToDriver(const std::string& message,
                  const std::string& device) {
    int fd = open(device.c_str(), O_WRONLY);

    if (fd == -1) {
        std::cerr << "StorageGuard driver open failed: "
                  << std::strerror(errno) << '\n';
        return false;
    }

    std::size_t totalWritten = 0;

    while (totalWritten < message.size()) {
        ssize_t written = write(
            fd,
            message.data() + totalWritten,
            message.size() - totalWritten
        );

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "StorageGuard driver write failed: "
                      << std::strerror(errno) << '\n';
            close(fd);
            return false;
        }

        if (written == 0) {
            std::cerr << "StorageGuard driver write made no progress.\n";
            close(fd);
            return false;
        }

        totalWritten += static_cast<std::size_t>(written);
    }

    if (close(fd) == -1) {
        std::cerr << "StorageGuard driver close failed: "
                  << std::strerror(errno) << '\n';
        return false;
    }

    return true;
}