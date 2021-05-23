# DEBUG LOGGER

A header only library to handle debugging in C++

## Prefix:
the prefix is just text that is printed out before each debug message. To set the prefix, call the setPrefix function and pass the input string.

```
DebugLogger logger;
logger.setPrefix("This is a prefix: ");
```
Every time you call a log function, it will print that prefix.
## Variables:
Variables can be accessed by using [varname]
```
logger.trace("[variableName]");
```
That will print out the variable name and remove the brackets.

A variable name must start with a letter or underscore. After that, it can include digits or letters/underscores

VARIABLES ARE CASE SENSITIVE

Please note that using formatters that start with a letter must have a space if placed before a number or identifier or they will not work properly
because they conflict.

When accessed, variables are case sensitive and must be lower case

### Variable types:
1. CHAR: a single 1 byte integer printed as a character
2. INTEGER32: a 32 bit integer 
3. INTEGER64: a 64 bit integer
4. FLOAT32: a 32 bit float (float)
5. FLOAT64: a 64 bit float (double)
6. STRING: internal strings are stored with std::string and not a char pointer

Variables can be both internal and external. Internal variables are set by the system and cannot be deleted, but they can be accessed from anywhere (including the prefix)

List of internal variables:
1. th: total program time in hours
2. tm: total program time in minutes
3. ts: total program time in seconds
4. tl: total program time in milliseconds
5. ti: total program tiem in microseconds
6. eth: time elapsed since last message hours
7. etm: time elapsed since last message minutes
8. ets: time elapsed since last message seconds
9. etl: time elapsed since last message milliseconds
10. eti: time elapsed since last message microseconds
11. pn: name of the program (can be set externally)
12. tn: name of the trace level
13. wn: name of the warning level
14. en: name of the error level
15. cn: name of the critical level
16. ln: name of the current level (useful when creating generic prefix)
17. dmc: count of debug messages (all levels included)
18. tmc: count of trace messages sent
19. wmc: count of warning messages sent
20. emc: count of error messages sent
21. cmc: count of critical messages sent
21. lmc: count of messages sent on the current level
22. bs: the character backslash: '\'
23. lbk: the character left bracket: '['
24. rbk: the character right bracket: ']'
25. lbc: the character left brace: '{'
26. rbc: the character right brace: '}'

External variables can be created by the programmer. To do so, you need the variable and a pointer. 
Undefined functionality if the variable goes out of scope and you try to use it in the debugger later!

```
int variable = 10;
logger.addVariable("variableName", &variable, DebugVarType::INTEGER32);

//the variables can be used the same exact way as internal variables
logger.trace("using internal variable [variableName]");
```

MAKE SURE THAT YOU USE THE RIGHT TYPE FOR THE VARIABLES OR ITS UNDEFINED FUNCTIONALITY

Variables can be used in the prefix, but parameters cannot

## Parameters:
Parameters are used in much the same way as variables, but they are given types instead of names. They are accessed with {} instead of []

Using parameters:
```
logger.trace("{int} {float}", 10, 10.12); //will print 10 to the screen
```

Each parameter type has multiple names that can be used interchangably with no change to the output.
Some are more explicit than others, but others are the same as printf to make it faster to write.

Types
1. char
2. 32 bit integer (signed / unsigned)
3. 64 bit integer (signed / unsigned)
4. float
5. string

* Char pneumonics:
    1. char
    2. ch
    3. c

* 32 bit signed integer pneumonics
    1. int
    2. i
    3. d

* 32 bit unsigned integer pneumonics
    1. uint
    2. ui
    3. u

* 64 bit signed integer pneumonics:
    1. long
    2. llu

* 64 bit unsigned integer pneumonics:
    1. ulong
    2. ul

* Float pneumonics:
    1. float
    2. flt
    3. f

* String pneumonics:
    1. string
    2. str
    3. s

For parameters, strings are passed as const char*, so if using std::string, using std::string.c_str()

## Formatting:
Each type has different formatting options

Each option can be written in any order as long as it doesn't combine seamlessly into another token type such as a number or identifier

Formatting options can be applied to variables and parameters in the exact same way

Apply formatting options in the following syntax

Formatting options for each type
* Char
  * '^' capitalizes the character
  * '$' lowercases the character
  * '>' aligns the character right
  * 'number' sets the min number of spaces it will take up. It will print spaces before or after the character is printed depending on the right alignment.
  ```
  logger.trace("{^>10char}", 'c');
  //prints 10 spaces right aligned and a capital C
  ```
* Int (works for both 32 and 64 bit ints)
    * '>' aligns the integer right
    * 'number' prints that number of spaces min
    * 'x' prints number in lower case hex
    * 'X' prints number in upper case hex
    * 'b' prints the number in binary
    * '+' indicates the number is unsigned (mainly for variables which don't have an unsigned type). It can ALSO be used for signed parameters, but it doesn't make any sense to do that
    * '0' fills zeros before or after the integer to fill the number of spaces based on right alignment. NOTE this one can interfere with the space, so put it before that formatter. It can also be placed after everything if you include a space.
    ```
    logger.trace("{08x+int}", -1); //zeroes are filled in after the hex unsigned integer taking up 8 spaces.

    //notice how the 0 specifier is before the number of spaces. The unsigned specifier can be anywhere. Because it's printing hex, the unsigned specifier doesn't do anything as hex is always positive.
    ```
* Float (works for both 32 bit and 64 bit floats) -> all float parameter are 64 bit
    * '>' align text right
    * 'number1.number2': number1 specifies the min number of total spaces the float can take up. number2 specifies the number of decimal places to round and print.
    * '0' works in the exact same way as integers
    ```
    //prints a float with 2 decimal spaces of precision
    logger.trace("{.2float}", 12.1225);

    //prints a float with 2 decimal spaces of precision, but with a couple of zeros after
    logger.trace("{010.2f}", 12.1235);
    ```
* String (works for both parameters and varaibles)
    * '^' specifies upper case
    * '$' species lower case
    * '>' specifies right alignment
    * 'number' the number of spaces before or after the text
    ```
    //prints the text right aligned with 100-strlen spaces preceding it
    logger.trace("{100>string}", "this is text to print");
    ```

## Levels
The logger has a could different levels.
1. Trace: for messages containing basic logging information
2. Warning: for messages containing information which indicates a potential problem
3. Error: for messages containing information about potentially program breaking problems
4. Critical: for messages containing information about a vital system having an error

Each level can be accessed with different functions
```
logger.trace("");
logger.warning("");
logger.error("");
logger.critical("");
```

Each one is named with a 3 letter name saved in their individual variables mentioned earlier.
1. Trace: TRE [tn]
2. Warning: WNG  [wn]
3. Error: ERR [en]
4. Critical: CRT [cn]

## Output buffers
The output of the debugger can be sent to cout by default, or it can be sent to a different ostream.

That ostream can be anything from a file, to an sstream.

Printing to alternative stream
```
std::ofstream file("fileName.txt", std::ios::out);
logger.traceToStream(file, "This will go to a file buffer");
```

## Sub-formats
Sub-formats allow you to apply formatting options to a formatting options to individual pieces of formatted text within a format. That is a simpler concept than it sounds. It just means that you can have a format inside of another format.

It essentially takes the output of the sub-format and applies formatting options to it before printing it out.

To use sub-formats, you have to use the variable access syntax with an open and close curly brace "[]". Instead of giving an variable name, you specify the formatting options, then add one single-quote. After that single quote, include the sub-format.
Any variable name put in where the formatting arguments are will not be printed, only the formatting string and its corresponding formats
```
['sub format]
```

The sub-format is terminated with the closing curly bracet, so you don't need a double quote.

writing sub formats is simple, use code like the following
```
//prints: normal formatting Sub FORMAT
logger.trace("Normal formatting ['Sub {^str}]", "format");
```

## Printing out [, ], {, }, and \
To print out the special characters, it must be escaped with a backslash. Given that C++ automatically escapes '\\' to '\', to write an escape, simply use the double backslash. To print out a backslash, use four backslashes: '\\\\'
```
//prints: This are some braces in a sub format: {}[]}{][\
logger.trace("This are some braces in a sub format: ['\\{\\}\\[\\]\\}\\{\\]\\[]\\\\");
```

Alternatively, each of these characters can be accessed with parameters, and there are default internal variables for each one.
```
//prints left and right braces
logger.trace("[lbc][rbc]");
logger.trace("{char}{char}", '[', ']');
```
For more information on these internal variables, see the variables section