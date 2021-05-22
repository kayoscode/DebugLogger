#ifndef INCLUDE_DEBUG_LOGGER_H
#define INCLUDE_DEBUG_LOGGER_H

#include <iostream>
#include <ostream>
#include <map>
#include <string>
#include <stdarg.h>
#include "Timer.h"

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

/**
 * all the valid types for debug variables
 * INTEGER32: a 32 bit integer
 * INTEGER64: a 64 bit integer
 * FLOAT32: a 32 bit floating point value
 * FLOAT64: a 64 bit floating point value
 * STRING: an instance of std::string
 * @author Bryce Young 5/19/2021
 * */
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
 * 
 * @author Bryce Young 5/20/2021
 * */
class DebugLogger {
    public:
        DebugLogger(const std::string& loggerName = "Debug:", Level level = Level::WARNING) 
            :level(level)
        {
            totalNanoseconds = 0;

            for(int i = 0; i <= (int)Level::LEVEL_COUNT; ++i) {
                messageCount[i] = 0;
            }

            levelNames[(int)Level::TRACE] = "TCE";
            levelNames[(int)Level::WARNING] = "WNG";
            levelNames[(int)Level::ERROR] = "ERR";
            levelNames[(int)Level::CRITICAL_ERROR] = "CRT";
            levelNames[(int)Level::LEVEL_COUNT] = levelNames[(int)Level::TRACE];

            //add default variables
            //th = time hours
            //tm = time minutes
            //ts = time seconds
            //tl = time milliseconds
            //ti = time microseconds
            addInternalVariable("th", timeVars + 0, DebugVarType::FLOAT64);
            addInternalVariable("tm", timeVars + 1, DebugVarType::FLOAT64);
            addInternalVariable("ts", timeVars + 2, DebugVarType::FLOAT64);
            addInternalVariable("tl", timeVars + 3, DebugVarType::FLOAT64);
            addInternalVariable("ti", timeVars + 4, DebugVarType::FLOAT64);

            //eth = elapsed time hours
            //etm = elapsed time minutes
            //ets = elapsed time seconds
            //etl = elapsed time milliseconds
            //eti = elapsed time microseconds
            addInternalVariable("eth", elapsedTimeVars + 0, DebugVarType::FLOAT64);
            addInternalVariable("etm", elapsedTimeVars + 1, DebugVarType::FLOAT64);
            addInternalVariable("ets", elapsedTimeVars + 2, DebugVarType::FLOAT64);
            addInternalVariable("etl", elapsedTimeVars + 3, DebugVarType::FLOAT64);
            addInternalVariable("eti", elapsedTimeVars + 4, DebugVarType::FLOAT64);

            //the name of the logger program
            addInternalVariable("pn", &this->loggerName, DebugVarType::STRING);

            //the name of the levels
            //tn = trace name
            //wn = warning name
            //en = error name
            //cn = critical name
            //ln = current level name
            addInternalVariable("tn", &this->levelNames[(int)Level::TRACE], DebugVarType::STRING);
            addInternalVariable("wn", &this->levelNames[(int)Level::WARNING], DebugVarType::STRING);
            addInternalVariable("en", &this->levelNames[(int)Level::ERROR], DebugVarType::STRING);
            addInternalVariable("cn", &this->levelNames[(int)Level::CRITICAL_ERROR], DebugVarType::STRING);
            addInternalVariable("ln", &this->levelNames[(int)Level::LEVEL_COUNT], DebugVarType::STRING);

            //variables for message count
            //dmc stands for debug message count
            addInternalVariable("dmc", &messageCount[(int)Level::LEVEL_COUNT], DebugVarType::INTEGER64);

            //tmc stands for trace message count
            //wmc warning message count
            //emc error message count
            //cmc critical messageCount
            addInternalVariable("tmc", &messageCount[(int)Level::TRACE], DebugVarType::INTEGER64);
            addInternalVariable("wmc", &messageCount[(int)Level::WARNING], DebugVarType::INTEGER64);
            addInternalVariable("emc", &messageCount[(int)Level::ERROR], DebugVarType::INTEGER64);
            addInternalVariable("cmc", &messageCount[(int)Level::CRITICAL_ERROR], DebugVarType::INTEGER64);

            //level message count
            addInternalVariable("lmc", &currentMessageCount, DebugVarType::INTEGER64);

            //char pnemonics
            reserves["char"] = Token::TokenType::SIGNED_CHAR;
            reserves["c"] = Token::TokenType::SIGNED_CHAR;

            //int pnemonics
            reserves["int"] = Token::TokenType::SIGNED_INT;
            reserves["i"] = Token::TokenType::SIGNED_INT;
            reserves["d"] = Token::TokenType::SIGNED_INT;
            reserves["uint"] = Token::TokenType::SIGNED_INT;
            reserves["ui"] = Token::TokenType::SIGNED_INT;
            reserves["u"] = Token::TokenType::SIGNED_INT;

            //long pneumonics
            reserves["long"] = Token::TokenType::SIGNED_LONG;
            reserves["llu"] = Token::TokenType::SIGNED_LONG;
            reserves["ulong"] = Token::TokenType::SIGNED_LONG;
            reserves["ul"] = Token::TokenType::SIGNED_LONG;

            //float pnemoinics
            reserves["float"] = Token::TokenType::FLOAT;
            reserves["flt"] = Token::TokenType::FLOAT;
            reserves["f"] = Token::TokenType::FLOAT;

            //str pneumonics
            reserves["string"] = Token::TokenType::STRING;
            reserves["str"] = Token::TokenType::STRING;
            reserves["s"] = Token::TokenType::STRING;

            setPrefix("[3ln]~[.2etl] [[[>05lmc]]]: ");
            timer.reset();
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
         * Applies the prefix to all levels if @param targetLevel is omitted
         * */
        void setPrefix(const std::string& prefix, Level targetLevel = Level::LEVEL_COUNT) {
            if(targetLevel == Level::LEVEL_COUNT) {
                this->prefixFormat[(int)Level::TRACE] = prefix;
                this->prefixFormat[(int)Level::WARNING] = prefix;
                this->prefixFormat[(int)Level::ERROR] = prefix;
                this->prefixFormat[(int)Level::CRITICAL_ERROR] = prefix;
            }
            else if(targetLevel < Level::LEVEL_COUNT && targetLevel >= Level::TRACE){
                this->prefixFormat[(int)targetLevel] = prefix;
            }
        }

        void trace(const char* format, ...) {
            va_list args;
            va_start(args, format);

            //set trace vars
            updateLogger(Level::TRACE);
            setTrace();
            logInternal(std::cout, format, args);

            va_end(args);
        }

        void traceToStream(std::ostream& output, const char* format, ...) {
            va_list args;
            va_start(args, format);

            //set trace vars
            updateLogger(Level::TRACE);
            setTrace();
            logInternal(output, format, args);

            va_end(args);
        }

        void warning(const char* format, ...) {
            va_list args;
            va_start(args, format);

            updateLogger(Level::WARNING);
            setWarning();
            logInternal(std::cout , format, args);

            va_end(args);
        }

        void warningToStream(std::ostream& output, const char* format, ...) {
            va_list args;
            va_start(args, format);

            updateLogger(Level::WARNING);
            setWarning();
            logInternal(output, format, args);

            va_end(args);
        }

        void error(const char* format, ...) {
            va_list args;
            va_start(args, format);

            updateLogger(Level::ERROR);
            setError();
            logInternal(std::cout, format, args);

            va_end(args);
        }

        void errorToStream(std::ostream& output, const char* format, ...) {
            va_list args;
            va_start(args, format);

            updateLogger(Level::ERROR);
            setError();
            logInternal(output, format, args);

            va_end(args);
        }

        void critical(const char* format, ...) {
            va_list(args);
            va_start(args, format);

            updateLogger(Level::CRITICAL_ERROR);
            setCritical();
            logInternal(std::cout, format, args);

            va_end(args);
        }

        void criticalToStream(std::ostream& output, const char* format, ...) {
            va_list(args);
            va_start(args, format);

            updateLogger(Level::CRITICAL_ERROR);
            setCritical();
            logInternal(output, format, args);

            va_end(args);
        }

        /**
         * Updates times and message counts
         * */
        inline void updateLogger(Level level) {
            messageCount[(int)level]++;
            messageCount[(int)Level::LEVEL_COUNT]++;

            //update timers (hrs, mins, seconds, millis, microseconds)
            long long elapsedNanos = timer.nanoseconds();
            totalNanoseconds += elapsedNanos;

            timeVars[0] = (double)totalNanoseconds / 3.6e12;
            timeVars[1] = (double)totalNanoseconds / 6e10;
            timeVars[2] = (double)totalNanoseconds / 1e9;
            timeVars[3] = (double)totalNanoseconds / 1e6;
            timeVars[4] = (double)totalNanoseconds / 1000;

            elapsedTimeVars[0] = (double)elapsedNanos / 3.6e12;
            elapsedTimeVars[1] = (double)elapsedNanos / 6e10;
            elapsedTimeVars[2] = (double)elapsedNanos / 1e9;
            elapsedTimeVars[3] = (double)elapsedNanos / 1e6;
            elapsedTimeVars[4] = (double)elapsedNanos / 1000;

            timer.reset();
        }

        void warning() {

        }

        void error() {

        }

        void critical() {

        }

        inline void setTrace() {
            //set trace vars
            levelNames[(int)Level::LEVEL_COUNT] = levelNames[(int)Level::TRACE];
            currentMessageCount = messageCount[(int)Level::TRACE];
        }

        inline void setWarning() {
            levelNames[(int)Level::LEVEL_COUNT] = levelNames[(int)Level::WARNING];
            currentMessageCount = messageCount[(int)Level::WARNING];
        }

        inline void setError() {
            levelNames[(int)Level::LEVEL_COUNT] = levelNames[(int)Level::ERROR];
            currentMessageCount = messageCount[(int)Level::ERROR];
        }

        inline void setCritical() {
            levelNames[(int)Level::LEVEL_COUNT] = levelNames[(int)Level::CRITICAL_ERROR];
            currentMessageCount = messageCount[(int)Level::CRITICAL_ERROR];
        }

        /**
         * adds a variable to the debugger
         * @param name the name to which the variable will be referred
         * @param variable the variable value as a void*
         * @param type the variable type, one of the section from the enum
         * @return true if the variable was added successfully, false if the variable is conflicting with other variables
         * */
        bool addVariable(const std::string& name, void* variable, DebugVarType type) {
            //validate variable
            const char* varName = name.c_str();
            int index = 0;
            if(isAlpha(varName, index) || varName[0] == '_') {
                index++;

                while(varName[index] && (isAlpha(varName, index) || varName[index] == '_' || isNum(varName, index))) {
                    index++;
                }
                
                if(index == name.size()) {
                    if(variables.find(name) == variables.end()) {
                        variables.emplace(std::pair<std::string, DebugVar>({name, DebugVar(type, variable)}));
                        return true;
                    }
                }
            }

            //improper var name or something
            return false;
        } 

        /**
         * Removes a variable from the list
         * @param name the name of the variable to remove
         * @return true if the variable existed and is now removed, false if the variable never existed
         * */
        bool removeVariable(const std::string& name) {
            std::map<std::string, DebugVar>::iterator v = variables.find(name);

            if(v != variables.end() && !v->second.getReadonly()) {
                variables.erase(v);
                return true;
            }

            return false;
        }

    private:
        /**
         * Adds a variable that cannot be removed
         * */
        bool addInternalVariable(const std::string& name, void* variable, DebugVarType type) {
            if(variables.find(name) == variables.end()) {
                variables.emplace(std::pair<std::string, DebugVar>({name, DebugVar(type, variable, true)}));
                return true;
            }

            return false;
        }

        /**
         * Internal method to handle logging
         * @param output the output stream to write to
         * @param format the print format
         * @param args the va arguments as a reference
         * @param level the current log level
         * */
        inline void logInternal(std::ostream& output, const char* format, va_list& args) {
            //print prefix to message using only internal variables
            printPrefix(output, level);

            //set color (trace color)
            int len = (int)strlen(format);
            int formatIndex = 0;
            int previousFormatIndex = -1;

            //process and print arguments
            while(printNext(output, format, formatIndex, args) && formatIndex < len && formatIndex != previousFormatIndex) {
                previousFormatIndex = formatIndex;
            }

            output << "\n";
        }

        /**
         * contains information representing a token
         * */
        struct Token {
            enum class TokenType {
                CAPITAL,
                LOWER,
                RIGHT,
                FILL_ZERO,
                ZERO_DECIMALS,
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
        
        /**
         * Loads the next identifier
         * */
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

        /**
         * Returns the next one or two character token
         * */
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
            else if(format[index] == '+') {
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

                //if its .0, then it counts as a single token
                if(format[index] == '0') {
                    currentToken.type = Token::TokenType::ZERO_DECIMALS;
                    currentToken.lexemeEnd++;
                    index++;
                }

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

                return true;
            }
            else {
                //error of some kind
                return false;
            }
        }

        /**
         * Returns the next number
         * */
        void getNumber(const char* format, int& index) {
            currentToken.type = Token::TokenType::NUMBER;
            currentToken.lexemeStart = format + index;

            while(isNum(format, index)) {
                index++;
            }

            currentToken.lexemeEnd = format + index;
        }

        /**
         * Enumerates all formatting options supplied by the user
         * */
        void collectFormattingOptions(const char* format, int& index, int& capitalized, bool& rightAligned, bool& unsignedValue, std::string& v, int& spaceCount, int& spaceCount_dec, bool& fillZero, int& outputFormat, char end) { 
            bool foundDecimal = false;

            //implement variable grammar here
            index++;
            while(format[index] != end && format[index]) {
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
                        spaceCount_dec = value;
                    }
                    else {
                        spaceCount = value;
                    }
                }
                else if(currentToken.type == Token::TokenType::ZERO_DECIMALS) {
                    spaceCount_dec = 0;
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
                    v = std::string(currentToken.lexemeStart, currentToken.lexemeEnd);
                }

                skipWhitespace(format, index);
            }
        }

        /**
         * Prints the variable
         * */
        void printVariable(std::ostream& output, const char* format, int& index) {
            //0 for no change, 1 for upper, 2 for lower
            int capitalized = CAPITALIZEDFORMAT_NONE;
            bool rightAligned = false;
            bool unsignedValue = false;
            std::string variableName;
            int setSpaceCount = -1;
            int setSpaceCount_dec = -1;
            bool fillZero = false;
            int outputFormat = OUTPUTFORMAT_DECIMAL;

            collectFormattingOptions(format, index, capitalized, rightAligned, unsignedValue, variableName, setSpaceCount, setSpaceCount_dec, fillZero, outputFormat, ']');

            //now that we have reached the end, we can go ahead and print the variable
            //lets check if it exists first
            std::map<std::string, DebugVar>::iterator var = variables.find(variableName);

            if(var != variables.end()) {
                switch(var->second.getType()) {
                    case DebugVarType::CHAR:
                        {
                            char value = var->second.getChar();
                            printFormattedChar(output, value, capitalized, rightAligned, setSpaceCount);
                        }
                        break;
                    case DebugVarType::INTEGER32:
                        {
                            uint32_t value = var->second.getInt32();
                            printFormattedInteger(output, value, rightAligned, setSpaceCount, outputFormat, unsignedValue, fillZero, false);
                        }
                        break;
                    case DebugVarType::INTEGER64:
                        {
                            uint64_t value = var->second.getInt64();
                            printFormattedInteger(output, value, rightAligned, setSpaceCount, outputFormat, unsignedValue, fillZero, true);
                        }
                        break;
                    case DebugVarType::FLOAT32:
                        {
                            float value = var->second.getFloat32();
                            printFormattedFloat(output, value, rightAligned, setSpaceCount, setSpaceCount_dec, fillZero);
                        }
                        break;
                    case DebugVarType::FLOAT64:
                        {
                            double value = var->second.getFloat64();
                            printFormattedFloat(output, value, rightAligned, setSpaceCount, setSpaceCount_dec, fillZero);
                        }
                        break;
                    case DebugVarType::STRING:
                        {
                            const char* value = var->second.getString();
                            printFormattedString(output, value, capitalized, rightAligned, setSpaceCount);
                        }
                        break;
                }
            }
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

        double powers10[6] = { 10, 100, 1000, 10000, 100000, 1000000 };

        void printFormattedFloat(std::ostream& output, double value, bool right, int spaces, int decSpaces, bool fillZero) {
            decSpaces = (decSpaces == -1)? 6 : decSpaces;
            int tmpDecSpaces = decSpaces;
            decSpaces = std::max(0, decSpaces);
            decSpaces = std::min(5, decSpaces);

            if(decSpaces > 0) {
                double power = powers10[decSpaces - 1];

                //truncate the multiplication by the power
                value = (value * power);
                value = std::trunc((value + .5));
                value /= power;
            }
            else if(decSpaces == 0) {
                value = std::trunc((value + .5));
            }

            decSpaces = tmpDecSpaces;

            std::string toPrint(std::to_string(value));
            int decLoc = (int)toPrint.find('.');
            int len = 0;

            if(decLoc != -1) {
                len = decLoc + decSpaces + (decSpaces != 0);
            }
            else {
                //if the decimal point doesn't exist in the string, there are no decimal places (not relying on std::to_string to add trailing decimals)
                len = (int)toPrint.size();

                if(decSpaces != 0) {
                    len += decSpaces + 1;
                    toPrint += '.';
                }
            }

            //format it right if necessary
            if(right) {
                for(int i = 0; i < spaces - len; ++i) {
                    if(fillZero) {
                        output << '0';
                    }
                    else {
                        output << ' ';
                    }
                }

                for(int i = 0; i < len; ++i) {
                    if(i < toPrint.size()) {
                        output << toPrint[i];
                    }
                    else {
                        output << '0';
                    }
                }
            }
            else {
                for(int i = 0; i < len; ++i) {
                    if(i < toPrint.size()) {
                        output << toPrint[i];
                    }
                    else {
                        output << '0';
                    }
                }

                for(int i = 0; i < spaces - len; ++i) {
                    if(fillZero) {
                        output << '0';
                    }
                    else {
                        output << ' ';
                    }
                }
            }
        }

        void printFormattedStringRaw(std::ostream& output, const char* toPrint, int cap, int len) {
            std::string buffer(toPrint, toPrint + len);

            for(int i = 0; i < len; ++i) {
                char next = toPrint[i];

                if(cap == CAPITALIZEDFORMAT_CAPS) {
                    next = std::toupper(next);
                }
                else if(cap == CAPITALIZEDFORMAT_LOWER) {
                    next = std::tolower(next);
                }

                buffer[i] = next;
            }

            output << buffer;
        }

        void printFormattedString(std::ostream& output, const char* toPrint, int cap, bool right, int space) {
            int len = (int)strlen(toPrint);

            if(right) {
                for(int i = 0; i < space - len; ++i) {
                    output << ' ';
                }

                printFormattedStringRaw(output, toPrint, cap, len);
            }
            else {
                printFormattedStringRaw(output, toPrint, cap, len);

                for(int i = 0; i < space - len; ++i) {
                    output << ' ';
                }
            }
        }

        void printFormattedInteger(std::ostream& output, uint64_t value, bool right, int space, int outputFormat, bool unsignedMark, bool fillZero, bool longlong) {
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
                        output << "0";
                    }
                    else {
                        output << " ";
                    }
                }

                output << buffer;
            }
            else {
                output << buffer;

                for(int i = 0; i < space - len; ++i) {
                    if(fillZero) {
                        output << "0";
                    }
                    else {
                        output << " ";
                    }
                }
            }
        }

        void printFormattedChar(std::ostream& output, char value, int cap, bool right, int space) {
            if(cap == 1) {
                value = std::toupper(value);
            }
            else if(cap == 2) {
                value = std::tolower(value);
            }

            if(right) {
                for(int i = 0; i < space - 1; ++i) {
                    output << " ";
                }

                output << value;
            }
            else {
                output << value;

                for(int i = 0; i < space - 1; ++i) {
                    output << " ";
                }
            }
        }

        void printArgument(std::ostream& output, const char* format, int& index, va_list& args) {
            int capitalized = CAPITALIZEDFORMAT_NONE;
            bool rightAligned = false;
            bool unsignedValue = false;
            std::string type;
            int setSpaceCount = -1;
            int setSpaceCount_dec = -1;
            bool fillZero = false;
            int outputFormat = OUTPUTFORMAT_DECIMAL;

            collectFormattingOptions(format, index, capitalized, rightAligned, unsignedValue, type, setSpaceCount, setSpaceCount_dec, fillZero, outputFormat, '}');

            //lookup table to determine the variable type
            std::map<std::string, Token::TokenType>::iterator t = reserves.find(type);
            Token::TokenType argumentType;

            if(t != reserves.end()) {
                //its actually a reserve word
                argumentType = t->second;

                if(argumentType == Token::TokenType::SIGNED_CHAR) {
                    //collect char from VA args and print
                    char ch = va_arg(args, int);
                    printFormattedChar(output, ch, capitalized, rightAligned, setSpaceCount);
                }
                else if(argumentType == Token::TokenType::SIGNED_INT) {
                    uint32_t val = va_arg(args, uint32_t);
                    printFormattedInteger(output, val, rightAligned, setSpaceCount, outputFormat, unsignedValue || type[0] == 'u', fillZero, false);
                }
                else if(argumentType == Token::TokenType::SIGNED_LONG) {
                    uint64_t val = va_arg(args, uint64_t);
                    printFormattedInteger(output, val, rightAligned, setSpaceCount, outputFormat, unsignedValue || type[0] == 'u', fillZero, true);
                }
                else if(argumentType == Token::TokenType::FLOAT) {
                    double val = va_arg(args, double);
                    printFormattedFloat(output, val, rightAligned, setSpaceCount, setSpaceCount_dec, fillZero);
                }
                else if(argumentType == Token::TokenType::STRING) {
                    const char* strValue = (const char*)va_arg(args, void*);
                    printFormattedString(output, strValue, capitalized, rightAligned, setSpaceCount);
                }
                else {
                    //unrecognized type, ignore it
                }
            }
            //otherwise the type was not recognized, ignore it
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
        bool printNext(std::ostream& outputStream, const char* format, int& index, va_list& args) {
            int startIndex = index;

            switch(format[index]) {
                case '[':
                    //[ escapes itself. If there are two in a row, it means to put [ as a raw character
                    if(format[index + 1] == '[') {
                        index++;
                        outputStream << format[index];
                    }
                    else {
                        printVariable(outputStream, format, index);
                    }
                    break;
                case '{':
                    //this also escapes itself in the exact same way as the other one
                    if(format[index + 1] == '{') {
                        index++;
                        outputStream << format[index];
                    }
                    else {
                        printArgument(outputStream, format, index, args);
                    }
                    break;
                case ']':
                    if(format[index + 1] == ']') {
                        index++;
                        outputStream << format[index];
                    }
                    break;
                case '}':
                    if(format[index + 1] == '}') {
                        index++;
                        outputStream << format[index];
                    }
                    break;
                default:
                    while(format[index] != ']' && format[index] != '[' && format[index] != '{' && format[index] != '}' && format[index]) {
                        index++;
                    }
                    outputStream << std::string(format + startIndex, format + index);
                    index--;
                    break;
            }

            index++;

            return format[index];
        }
            
        /**
         * Prints the prefix
         * Pretty much the same thing as printNext, but it will only accept variables
         * */
        bool printNextPrefix(std::ostream& outputStream, const char* format, int& index) {
            int startIndex = index;

            switch (format[index]) {
                case '[':
                    //escapes itself
                    if(format[index + 1] == '[') {
                        index++;
                        outputStream << format[index];
                    }
                    else {
                        printVariable(outputStream, format, index);
                    }
                    break;
                case ']':
                    if(format[index + 1] == ']') {
                        index++;
                        outputStream << format[index];
                    }
                    break;
                default:
                    while(format[index] != ']' && format[index] != '[' && format[index] != '{' && format[index] != '}' && format[index]) {
                        index++;
                    }

                    outputStream << std::string(format + startIndex, format + index);
                    index--;
                    break;
            }

            index++;
            return format[index];
        }

        //raw values for total time
        double timeVars[5] = { 0 };
        long long totalNanoseconds = 0;

        //raw values for total time
        double elapsedTimeVars[5] = { 0 };

        /**
         * Stores the number of messages at each level
         * messageCount[LEVEL_COUNT] is the total number of messages sent to the debugger
         * */
        long long messageCount[(int)Level::LEVEL_COUNT + 1];
        long long currentMessageCount = 0;

        /*
        * prints to the output stream the debug format
        */
        void printPrefix(std::ostream& output, Level level) {
            const char* format = prefixFormat[(int)level].c_str();
            int len = (int)prefixFormat[(int)level].size();
            int formatIndex = 0;
            int previousFormatIndex = -1;

            //process and print arguments
            while(printNextPrefix(output, format, formatIndex) && formatIndex < len && formatIndex != previousFormatIndex) {
                previousFormatIndex = formatIndex;
            }
        }

        /**
         * Struct containing information for a debug var
         * @author Bryce Young
         * */
        struct DebugVar {
            public:

                DebugVar(DebugVarType type, void* value, bool readOnly = false) 
                    :type(type),
                    value(value),
                    readonly(readOnly)
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

                bool getReadonly() {
                    return readonly;
                }

            private:
                DebugVarType type;
                void* value;
                bool readonly = false;
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

        //an array of level names
        std::string levelNames[(int)Level::LEVEL_COUNT];

        /**
         * A string representing the prefix of each debug
         * Can access internal variables and is updated on each print
         * You can use a different format for each debug level if you want, but you have to specify it with specific function calls
         * Calling the funciton to set the prefix format globally will overwrite it for all level counts!
         * */
        std::string prefixFormat[(int)Level::LEVEL_COUNT];

        /**
         * Timer to keep track of time and changes in it
         * */
        Timer timer;
};

#endif