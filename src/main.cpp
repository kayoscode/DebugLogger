#include <iostream>
#include "DebugLogger.h"

int main() {
    DebugLogger logger;
    int v = 0x1A;
    float vflt = 1012.123f;
    char test = 'c';
    std::string string = "testString :D!";
    logger.addVariable("int", &v, DebugVarType::INTEGER32);
    logger.addVariable("float", &vflt, DebugVarType::FLOAT32);
    logger.addVariable("test", &test, DebugVarType::CHAR);
    logger.addVariable("string", &string, DebugVarType::STRING);
    logger.trace(std::cout, "test: [20>string]");
    return 0;
}