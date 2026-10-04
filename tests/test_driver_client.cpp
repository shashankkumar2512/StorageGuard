#include "driver_client.h"

#include <iostream>
#include <string>

int main() {
    const std::string message = "StorageGuard: transaction 123";

    if (!sendToDriver(message)) {
        return 1;
    }

    std::cout << "PASS: Transaction sent to kernel driver.\n";
    return 0;
}
