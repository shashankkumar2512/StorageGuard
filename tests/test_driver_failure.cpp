#include "driver_client.h"
#include <iostream>

int main() {
    bool result = sendToDriver(
        "test transaction",
        "/tmp/nonexistent-storageguard-device"
    );

    if (result) {
        std::cerr << "FAIL: Expected driver connection to fail.\n";
        return 1;
    }

    std::cout << "PASS: Driver failure handled correctly.\n";
    return 0;
}
