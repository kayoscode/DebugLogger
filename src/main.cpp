#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    logger.trace("This is a test {^c}", 'a');
    return 0;
}