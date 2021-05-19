#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    char v = 'c';
    logger.addVariable("randomChar", &v, DebugVarType::CHAR);
    logger.trace(std::cout, "randomChar value: [randomChar]");
    return 0;
}