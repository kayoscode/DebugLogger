#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    char c = 'c';
    logger.addVariable("newChar", &c, DebugVarType::CHAR);
    logger.trace("[newChar]");
    return 0;
}