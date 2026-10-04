#ifndef DRIVER_CLIENT_H
#define DRIVER_CLIENT_H

#include <string>

bool sendToDriver(
    const std::string& message,
    const std::string& device = "/dev/storageguard"
);

#endif