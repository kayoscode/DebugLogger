#ifndef INCLUDE_DEBUG_LOGGER_H
#define INCLUDE_DEBUG_LOGGER_H

#include <iostream>
#include <ostream>
#include <map>
#include <string>
#include <stdarg.h>

constexpr int OUTPUTFORMAT_DECIMAL = 0;
constexpr int OUTPUTFORMAT_HEX = 1;
constexpr int OUTPUTFORMAT_UPPERHEX = 2;
constexpr int OUTPUTFORMAT_BIN = 3;
constexpr int CAPITALIZEDFORMAT_NONE = 0;
constexpr int CAPITALIZEDFORMAT_CAPS = 1;
constexpr int CAPITALIZEDFORMAT_LOWER = 2;


/**
 * Levels for debugging
 * NONE: nothing is output
 * TRACE: basic trace information
 * WARNING: small non-critical errors (code can continue to run without problems)
 * ERROR: large potentially problematic issues (code can probably run, but might crash)
 * CRITICALERROR: errors that break the program. It should close
 * */
enum class Level {
    NONE,
    TRACE,
    WARNING,
    ERROR,
    CRITICAL_ERROR,
    LEVEL_COUNT
};

enum class DebugVarType {
    CHAR,
    INTEGER32,
    INTEGER64,
    FLOAT32,
    FLOAT64,
    STRING,
    DEBUGVAR_TYPE_COUNT
};

/**
 * Class to interface with the logger
 * CFG doc:
 * 
 * Capitalize any output by prefixing with ^
 * */
class DebugLogger {
    public:
        DebugLogger(const std::string& loggerName = "Debug:", Level level = Level::WARNING) 
            :level(level)
        {
            //add default variables
            addVariable("th", timeVars + 0, DebugVarType::FLOAT32);
            addVariable("tm", timeVars + 1, DebugVarType::FLOAT32);
            addVariable("ts", timeVars + 2, DebugVarType::FLOAT32);
            addVariable("tl", timeVars + 3, DebugVarType::FLOAT32);
            addVariable("ti", timeVars + 4, DebugVarType::FLOAT32);
            addVariable("tn", timeVars + 5, DebugVarType::FLOAT32);

            addVariable("eth", elapsedTimeVars + 0, DebugVarType::FLOAT32);
            addVariable("etm", elapsedTimeVars + 1, DebugVarType::FLOAT32);
            addVariable("ets", elapsedTimeVars + 2, DebugVarType::FLOAT32);
            addVariable("etl", elapsedTimeVars + 3, DebugVarType::FLOAT32);
            addVariable("eti", elapsedTimeVars + 4, DebugVarType::FLOAT32);
            addVariable("etn", elapsedTimeVars + 5, DebugVarType::FLOAT32);

            addVariable("name", &this->loggerName, DebugVarType::STRING);

            //char pnemonics
            reserves["char"] = Token::TokenType::SIGNED_CHAR;

            //short pnemoincs
            reserves["short"] = Token::TokenType::SIGNED_SHORT;
            reserves["ushort"] = Token::TokenType::SIGNED_SHORT;

            //int pnemonics
            reserves["int"] = Token::TokenType::SIGNED_INT;
            reserves["uint"] = Token::TokenType::SIGNED_INT;

            //long pneumonics
            reserves["long"] = Token::TokenType::SIGNED_LONG;
            reserves["ulong"] = Token::TokenType::SIGNED_LONG;

            //float pnemoinics
            reserves["float"] = Token::TokenType::FLOAT;

        }

        ~DebugLogger() {
        }

        /**
         * Returns the level of the debugger
         * */
        Level getLevel() const {
            return level;
        }

        /**
         * Updates level
         * @param newLevel: the new level
         * */
        void setLevel(Level newLevel) {
            this->level = newLevel;
        }
        
        /**
         * There exist various variables which the prefix uses
         * To use a variable, use two braces and write the name of the variable inside; constrast from the {} used for printing parameters passed into the function
         * [ var_name ] -> with or without the spaces, it doesn't matter
         * Basic formatting options within the braces apply
         * Variables:
         * th, tm, ts, tl, ti, tn -> the time suffixes (also their own variables)
         * th -> time hours
         * tm -> time minutes
         * ts -> time seconds
         * tl -> time milliseconds
         * ti -> time microseconds
         * tn -> time nanoseconds
         * These time suffixees can be used after any time variable
         * no prefix means use the raw values for the time, so (th) will give the time in hours including the decimal
         * eth, etm, ets, .. -> elapsed time since last message (doesnt care about the debug level)
         * 
         * other variables
         * dl -> debug level
         * name -> the name of the logger
         * whatever custom variables you add
         * */
        void setPrefix(const std::string& prefix) {
            this->prefixFormat = prefix;
        }

        std::string getPrefix() const {
            return this->prefixFormat;
        }

        void trace(std::ostream& output, const char* format, ...) {
            //set color (trace color)
            va_list args;
            va_start(args, format);

            int formatIndex = 0;

            //process and print arguments
            while(printNextArgument(format, formatIndex, args));

            va_end(args);
        }

        void warning(std::ostream& output, const char* format, ...) {
            //set color (warning color)
        }

        void error(std::ostream& output, const char* format, ...) {
            //set color (error color)
        }

        void critical(std::ostream& output, const char* format, ...) {
            //set color (critical color)
        }

        /**
         * adds a variable to the debugger
         * @param name the name to which the variable will be referred
         * @param variable the variable value as a void*
         * @param type the variable type, one of the section from the enum
         * @return true if the variable was added successfully, false if the variable is conflicting with other variables
         * */
        bool addVariable(const std::string& name, void* variable, DebugVarType type) {
            if(variables.find(name) == variables.end()) {
                variables.emplace(std::pair<std::string, DebugVar>({name, DebugVar(type, variable)}));
                return true;
            }

            return false;
        } 

        /**
         * Removes a variable from the list
         * @param name the name of the variable to remove
         * @return true if the variable existed and is now removed, false if the variable never existed
         * */
        bool removeVariable(const std::string& name) {
            std::map<std::string, DebugVar>::iterator v = variables.find(name);

            if(v != variables.end()) {
                variables.erase(v);
                return true;
            }

            return false;
        }

    private:
        /**
         * contains information representing a token
         * */
        struct Token {
            enum class TokenType {
                CAPITAL,
                LOWER,
                RIGHT,
                FILL_ZERO,
                VARIABLE_NAME,
                FLOAT,
                UNSIGNED_MARK,
                SIGNED_CHAR,
                SIGNED_SHORT,
                SIGNED_INT,
                SIGNED_LONG,
                STRING,
                NUMBER,
                DECIMAL,
                HEX_MODIFIER,
                CAPITAL_HEX_MODIFIER,
                BINARY_MODIFIER
            };

            const char* lexemeStart, *lexemeEnd;
            TokenType type;
        };

        Token currentToken;

        bool getIdentifier(const char* format, int& index) {
            if(isAlpha(format, index) || format[index] == '_') {
            }
            else {
                return false;
            }

            currentToken.type = Token::TokenType::VARIABLE_NAME;
            currentToken.lexemeStart = format + index;

            while(format[index] && (isAlpha(format, index) || format[index] == '_' || isNum(format, index))) {
                index++;
            }

            currentToken.lexemeEnd = format + index;
            return true;
        }

        bool getNextToken(const char* format, int& index) {
            skipWhitespace(format, index);

            if(format[index] == '^') {
                currentToken.type = Token::TokenType::CAPITAL;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            if(format[index] == '0') {
                currentToken.type = Token::TokenType::FILL_ZERO;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            if(format[index] == '$') {
                currentToken.type = Token::TokenType::LOWER;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            if(format[index] == '>') {
                currentToken.type = Token::TokenType::RIGHT;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(format[index] == 'u') {
                currentToken.type = Token::TokenType::UNSIGNED_MARK;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(format[index] == '.') {
                currentToken.type = Token::TokenType::DECIMAL;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(format[index] == 'x') {
                currentToken.type = Token::TokenType::HEX_MODIFIER;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(format[index] == 'X') {
                currentToken.type = Token::TokenType::CAPITAL_HEX_MODIFIER;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(format[index] == 'b') {
                currentToken.type = Token::TokenType::BINARY_MODIFIER;
                currentToken.lexemeStart = format + index;
                currentToken.lexemeEnd = format + index + 1;
                index++;
                return true;
            }
            else if(isNum(format, index)) {
                getNumber(format, index);
                return true;
            }
            else if(getIdentifier(format, index)) {
                //check to see if the identifier was actually a reserve
                std::string v(currentToken.lexemeStart, currentToken.lexemeEnd);

                std::map<std::string, Token::TokenType>::iterator t = reserves.find(v);

                if(t != reserves.end()) {
                    //its actually a reserve word
                    currentToken.type = t->second;
                }

                return true;
            }
            else {
                //error of some kind
                return false;
            }
        }

        void getNumber(const char* format, int& index) {
            currentToken.type = Token::TokenType::NUMBER;
            currentToken.lexemeStart = format + index;

            while(isNum(format, index)) {
                index++;
            }

            currentToken.lexemeEnd = format + index;
        }

        void printVariable(const char* format, int& index, va_list args) {
            //0 for no change, 1 for upper, 2 for lower
            int capitalized = CAPITALIZEDFORMAT_NONE;
            bool rightAligned = false;
            bool unsignedValue = false;
            std::string variableName;
            int setSpaceCount = -1;
            int setSpaceCount_dec = -1;
            bool fillZero = false;

            //0 = decimal, 1 = hex, 2 = upper hex, 3 = binary
            int outputFormat = OUTPUTFORMAT_DECIMAL;

            bool foundDecimal = false;

            //implement variable grammar here
            index++;
            while(format[index] != ']' && format[index]) {
                if(!getNextToken(format, index)) {
                    break;
                }

                if(currentToken.type == Token::TokenType::CAPITAL) {
                    capitalized = CAPITALIZEDFORMAT_CAPS;
                }
                else if(currentToken.type == Token::TokenType::LOWER) {
                    capitalized = CAPITALIZEDFORMAT_LOWER;
                }
                else if(currentToken.type == Token::TokenType::RIGHT) {
                    rightAligned = true;
                }
                else if(currentToken.type == Token::TokenType::FILL_ZERO) {
                    fillZero = true;
                }
                else if(currentToken.type == Token::TokenType::NUMBER) {
                    int value = 0;
                    std::string lexeme(currentToken.lexemeStart, currentToken.lexemeEnd);
                    value = std::stoi(lexeme);

                    if(foundDecimal) {
                        foundDecimal = false;
                        setSpaceCount_dec = value;
                    }
                    else {
                        setSpaceCount = value;
                    }
                }
                else if(currentToken.type == Token::TokenType::DECIMAL) {
                    foundDecimal = true;
                }
                else if(currentToken.type == Token::TokenType::UNSIGNED_MARK) {
                    unsignedValue = true;
                }
                else if(currentToken.type == Token::TokenType::HEX_MODIFIER) {
                    outputFormat = OUTPUTFORMAT_HEX;
                }
                else if(currentToken.type == Token::TokenType::CAPITAL_HEX_MODIFIER) {
                    outputFormat = OUTPUTFORMAT_UPPERHEX;
                }
                else if(currentToken.type == Token::TokenType::BINARY_MODIFIER) {
                    outputFormat = OUTPUTFORMAT_BIN;
                }
                else {
                    variableName = std::string(currentToken.lexemeStart, currentToken.lexemeEnd);
                }

                skipWhitespace(format, index);
            }

            //now that we have reached the end, we can go ahead and print the variable
            //lets check if it exists first
            std::map<std::string, DebugVar>::iterator var = variables.find(variableName);

            if(var != variables.end()) {
                switch(var->second.getType()) {
                    case DebugVarType::CHAR:
                        {
                            char value = var->second.getChar();
                            printFormattedChar(value, capitalized, rightAligned, setSpaceCount);
                        }
                        break;
                    case DebugVarType::INTEGER32:
                        {
                            uint32_t value = var->second.getInt32();
                            printFormattedInteger(value, rightAligned, setSpaceCount, outputFormat, unsignedValue, fillZero, false);
                        }
                        break;
                    case DebugVarType::INTEGER64:
                        {
                            uint64_t value = var->second.getInt64();
                            printFormattedInteger(value, rightAligned, setSpaceCount, outputFormat, unsignedValue, fillZero, true);
                        }
                        break;
                    case DebugVarType::FLOAT32:
                        {
                            float value = var->second.getFloat32();
                            printFormattedFloat(value, rightAligned, setSpaceCount, setSpaceCount_dec, fillZero);
                        }
                        break;
                    case DebugVarType::FLOAT64:
                        {
                            double value = var->second.getFloat64();
                            printFormattedFloat(value, rightAligned, setSpaceCount, setSpaceCount_dec, fillZero);
                        }
                        break;
                    case DebugVarType::STRING:
                        {
                            const char* value = var->second.getString();
                            printFormattedString(value, capitalized, rightAligned, setSpaceCount);
                        }
                        break;
                }
            }
            //otherwise skip printing and continue (for now, perhaps an error msg in the print string)
        }

        /**
         * Reverses a string
         * @param buffer the buffer to reverse
         * @param len the length of the contents of the string
         * */
        void reverseString(char* buffer, int len) {
            //reverse the string in place
            char* begin = buffer, *end = buffer + len - 1;

            while(begin < end) {
                char tmp = *begin;
                *begin = *end;
                *end = tmp;
                begin++;
                end--;
            }
        }

        /**
         * Prints hex to a buffer
         * @param spaces indicates the min number of digits
         * @param fill the character to fill the empty spaces with
         * @return the length of the string
         * */
        int printHexToBuffer(char* buffer, uint64_t value, bool upperFlag) {
            char hexChars[] = "0123456789abcdef";
            char capHexChars[] = "0123456789ABCDEF";
            char* charset = upperFlag? capHexChars : hexChars;
            int index = 0;

            do{
                int digit = value % 16;
                buffer[index++] = charset[digit];
                value /= 16;
            } while(value);

            buffer[index] = 0;
            reverseString(buffer, index);
            return index;
        }

        int printBinToBuffer(char* buffer, uint64_t value) {
            char charset[3] = "01";
            int index = 0;

            do {
                int digit = value % 2;
                buffer[index++] = charset[digit];
                value /= 2;
            } while(value);
            
            buffer[index] = 0;
            reverseString(buffer, index);
            return index;
        }

        void printFormattedFloat(double value, bool right, int spaces, int decSpaces, bool fillZero) {
            std::string format = "%";
            if(fillZero) {
                format += '0';
            }

            format += std::to_string(spaces);

            if(decSpaces > 0) {
                format += ".";
                format += std::to_string(decSpaces);
            }

            format += "f";

            char buffer[129];
            buffer[128] = 0;
            int len = sprintf_s(buffer, 128, format.c_str(), value);

            printf("%s", buffer);
        }

        void printFormattedStringRaw(const char* toPrint, int cap, int len) {
            for(int i = 0; i < len; ++i) {
                char next = toPrint[i];

                if(cap == CAPITALIZEDFORMAT_CAPS) {
                    next = std::toupper(next);
                }
                else if(cap == CAPITALIZEDFORMAT_LOWER) {
                    next = std::tolower(next);
                }

                printf("%c", next);
            }
        }

        void printFormattedString(const char* toPrint, int cap, bool right, int space) {
            int len = (int)strlen(toPrint);

            if(right) {
                for(int i = 0; i < space - len; ++i) {
                    printf(" ");
                }

                printFormattedStringRaw(toPrint, cap, len);
            }
            else {
                printFormattedStringRaw(toPrint, cap, len);

                for(int i = 0; i < space - len; ++i) {
                    printf(" ");
                }
            }
        }

        void printFormattedInteger(uint64_t value, bool right, int space, int outputFormat, bool unsignedMark, bool fillZero, bool longlong) {
            char buffer[129];
            buffer[128] = 0;
            int len = 0;

            //print 32 bit integer
            if(outputFormat == OUTPUTFORMAT_DECIMAL) {
                if(unsignedMark) {
                    if(longlong) {
                        len = sprintf_s(buffer, 128, "%llu", value);
                    }
                    else {
                        len = sprintf_s(buffer, 128, "%u", (int)value);
                    }
                }
                else {
                    if(longlong) {
                        len = sprintf_s(buffer, 128, "%lld", value);
                    }
                    else {
                        len = sprintf_s(buffer, 128, "%d", (int)value);
                    }
                }
            }
            else if(outputFormat == OUTPUTFORMAT_HEX) {
                len = printHexToBuffer(buffer, value, false);
            }
            else if(outputFormat == OUTPUTFORMAT_UPPERHEX) {
                len = printHexToBuffer(buffer, value, true);
            }
            else if(outputFormat == OUTPUTFORMAT_BIN) {
                len = printBinToBuffer(buffer, value);
            }

            if(right) {
                for(int i = 0; i < space - len; ++i) {
                    if(fillZero) {
                        printf("0");
                    }
                    else {
                        printf(" ");
                    }
                }

                printf("%s", buffer);
            }
            else {
                printf("%s", buffer);
            }
        }

        void printFormattedChar(char value, int cap, bool right, int space) {
            if(cap == 1) {
                value = std::toupper(value);
            }
            else if(cap == 2) {
                value = std::tolower(value);
            }

            if(right) {
                for(int i = 0; i < space - 1; ++i) {
                    printf(" ");
                }

                printf("%c", value);
            }
            else {
                printf("%c", value);

                for(int i = 0; i < space - 1; ++i) {
                    printf(" ");
                }
            }
        }

        void printArgument(const char* format, int& index, va_list args) {
            //implement variable grammar here
            index++;
            while(format[index] != '}' && format[index]) {
                if(!getNextToken(format, index)) {
                    break;
                }

                skipWhitespace(format, index);
            }
        }

        /**
         * Prints an argument and returns whether the entire string has been finished or not
         * Prints each character until a special one is found:
         * [] indicates a variable
         * {} indicates an ordered parameter in the va_list
         * @param format the format for printing
         * @param index the current index in the string
         * @param args the argument list
         * @return true if there is more to the string
         * */
        bool printNextArgument(const char* format, int& index, va_list args) {
            while(format[index]) {
                switch(format[index]) {
                    case '[':
                        printVariable(format, index, args);
                        break;
                    case '{':
                        printArgument(format, index, args);
                        return format[index];
                    case '\\':
                        index++;
                    default:
                        std::cout << format[index];
                        break;
                }

                index++;
            }

            return format[index];
        }

        //raw values for total time
        float timeVars[6] = { 0 };

        //raw values for total time
        float elapsedTimeVars[6] = { 0 };

        /*
        * prints to the output stream the debug format
        */
        void printPrefix() {

        }

        /**
         * Struct containing information for a debug var
         * @author Bryce Young
         * */
        struct DebugVar {
            public:

                DebugVar(DebugVarType type, void* value) 
                    :type(type),
                    value(value)
                {
                }

                DebugVar(const DebugVar& var)
                    :type(var.type),
                    value(var.value)
                {
                }

                DebugVar& operator=(const DebugVar& var) {
                    this->type = var.type;
                    this->value = var.value;
                    return *this;
                }

                char getChar() {
                    return *(char*)value;
                }

                float getFloat32() {
                    return *(float*)value;
                }

                double getFloat64() {
                    return *(double*)value;
                }

                int32_t getInt32() {
                    return *(uint32_t*)value;
                }

                int64_t getInt64() {
                    return *(uint64_t*)value;
                }

                const char* getString() {
                    return &(*(std::string*)value)[0];
                }

                DebugVarType getType() const {
                    return type;
                }

            private:
                DebugVarType type;
                void* value;
        };

        bool isNum(const char* format, int& index) {
            return (format[index] >= '0' && format[index] <= '9');
        }

        bool isAlpha(const char* format, int& index) {
            return ((format[index] >= 'a' && format[index] <= 'z') || (format[index] >= 'A' && format[index] <= 'Z'));
        }

        bool isWhiteSpace(const char* format, int& index) {
            return format[index] == ' ' || format[index] == '\t';
        }

        void skipWhitespace(const char* format, int& index) {
            while(isWhiteSpace(format, index)) {
                index++;
            }
        }

        //current output level
        Level level;

        //the logger's name
        std::string loggerName;

        //list of every usable variable
        std::map<std::string, DebugVar> variables;

        //list of reserves
        std::map<std::string, Token::TokenType> reserves;

        /**
         * A string representing the prefix of each debug
         * Can access internal variables and is updated on each print
         * */
        std::string prefixFormat;

};

#endif