#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    float variable = 10.12f;
    int variable2 = 10;
    logger.addVariable("var", &variable, DebugVarType::FLOAT32);
    logger.addVariable("var2", &variable2, DebugVarType::INTEGER32);
    logger.trace(std::cout, "[.100var] this sux");
    return 0;
}