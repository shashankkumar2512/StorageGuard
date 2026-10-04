#ifndef RECOVERY_H
#define RECOVERY_H

#include <string>

bool recoverTransaction(int transactionId,
                        const std::string& operation,
                        const std::string& filePath,
                        bool forwardToDriver = true);

bool recoverPendingTransactions(const std::string& operation,
                                const std::string& filePath,
                                int& recoveredCount,
                                bool forwardToDriver = true);

#endif
