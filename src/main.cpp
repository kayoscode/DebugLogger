#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    int counter = 0;
    logger.addVariable("c", &counter, DebugVarType::INTEGER32);

    for(counter = 0; counter < 10000; ++counter) {
        logger.trace("this is a test [c]");
    }

    return 0;
}