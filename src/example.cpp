#include <iostream>
#include "DebugLogger.h"
#include <sstream>

//example of using debug logger

/**
 * This is a header only library, so all you have to do is copy the headers into your project and you can use it!
 * The cmake code here is for compiling this demo
 * */
int main() {
    DebugLogger logger;
    logger.trace("output");

    //set prefix (prefix can only use internal variables and cannot deal with parameters)
    //prefixes can be set for each individual level, but this example won't deal with that as it's probably not an important feature
    //prefix follows the same formatting options as the normal trace prints
    logger.setPrefix("");
    logger.trace("notice how this only prints the text shown here, if we change the prefix it will change");

    //if we add a prefix, it appears before every log message (one can be set for each individual level, but again, we wont use that in this demo)
    //to use that, simply add a parameter indicating the level for the prefix
    logger.setPrefix("This is a prefix: ");
    logger.trace("This text should appear after the prefix");
    logger.setPrefix("");

    //now let's talk about internal variables
    //there are several internal variables indicated in the documentation.
    //to use these internal variables, use a double bracket -> [] and write the name of the variable inside [variableName]
    logger.trace("[lmc]: internal variable lmc(level message count) indicates the number of messages that have been sent to the current level");
    //there are many default internal variables (please read the documentation for more details)

    //these variables can be used in PREFIXES!
    //NOTE the variable indicator can be escaped with a backslash which must be written in c and c++ as '\\'
    //NOTE you can escape a backslash with a backslash. So to output a backslash, use four '\\\\'
    logger.setPrefix("[ln]-\\[[lmc]\\]: ");
    logger.trace("This prints the current level's name followed by a dash and the current number of messages in this level - \\\\\\]");

    //you can create your own internal variables!!
    //you just need a logger object, a pointer to the variable, as well as a type and size (int32 and int64 are DIFFERENT)
    int var1 = 100;
    long long var2 = 1000;
    //add the vars
    logger.addVariable("var1", &var1, DebugVarType::INTEGER32); //int32 because its a 32 bit integer
    logger.addVariable("var2", &var2, DebugVarType::INTEGER64); //this time its a 64 bit integer
    logger.trace("32 bit var: [var1], 64 bit var: [var2]");

    //PRINTING arguments
    //this works a lot like printf where you write the format, and pass each variable in separately
    //there are various types of varaibles that can be printed indicated in the documentation. Each one has its printf alias and a more readable version
    //use a variable with curly braces. These can be escaped in the same was as the brackets
    logger.trace("char: {char}, int: {d}, another int: {int}", 'B', 16, -1);

    //FORMATING options
    //each argument and variable is formatted the exact same way, but the available formatting options vary between types
    //for details on this, read the docs. Invalid formatting options will be ignored. Repeated options will be overridden
    logger.trace("Takes 10 spaces ({10d}), 10 spaces right justified ({>10d})", 10, 10);
    //notice how the arguments took up the same space, but the '>' told the system to move the text to the right
    //another example
    logger.trace("Lots of options on string: ({10>^str}), unsigned int: ({+d}), another unsigned int: ({u})", "text", -1, -1);
    //'^' capitalizes the string or char, '+' makes an integer unsigned. You can also use the unsigned types like: uint, u, ulong, ul, etc

    //you can see the same formatting options can be applied to variables
    //let me show you how to lowercase the text with '$'
    logger.trace("normal: [ln], formatted: [$ln]");

    //formatting options may be applied in any order as long as they don't conflict. The main one that could conflict is the fill zero formatter.
    //that one is marked with a 0, so what could happen is if you put that right after the space option, it will treat it as part of the number
    //heres how to use it properly and how to print in hex
    //NOTE PLEASE PUT A SPACE BETWEEN THE BINARY AND HEX SYMBOLS because they can conflict with numbers, types, and identifiers
    logger.trace("{>08X int}{0x long 16}", 0xDEAD, 0xBEEF);
    //by putting the zero specified before the space count, it treated it properly. As well as that, putting it after the x also works
    //notice how it doesn't matter where you put the space parameter as long as it cannot be confused with the type specifier or variable name. That's why there must be a space if you put it after
    //NOTE: the capitalization doesn't work on hex numbers. You have to use lowercase x or X to change the case.

    //FORMATTING floats

    //lets combine what we learned 
    //this is a simple prefix with basic internal information
    //prints the name with 3 characters, prints the elapsed time in milliseconds to 2 decimal places, then prints the current level's message count with five digits filling in zeros
    logger.setPrefix("[3ln]~[.2etl] \\[[>05lmc]\\]: ");
    double testVar = 100.156;
    logger.addVariable("testVar", &testVar, DebugVarType::FLOAT64);

    //there is one more spacing option for floats. If you add a number after a decimal point, then it indicates the number of spaces taken by the decimals
    //.2 would indicate 2 decimals of precision.
    //the first number in the spacing represents the total amount of space used by the float. For example 10.2 will use 10 spaces min, but will use at most 2 decimal places of precision
    logger.trace("{str}: ([10.2testVar])", "Current testVar value");
    testVar = -0.12338;
    logger.trace("{str}: [.4testVar], param 2 {{{>+3uint}}}, capitalized char: {^char}", "Now it's something different", 10, 'a');

    //RECURSIVE formats
    //here it gets a little more complicated, but much more powerful!
    //recursive formatting allows you to apply formatting to "sub-format strings" within each formatted string
    //this allows you to easily create effective alignment properties within strings
    
    //to use, create a new argument specifier with [], but don't include a type or argument as parameters (the argument will not be outputtint or skipped)
    //when a sub-format is used, arguments are ignored
    //use a single quote to indicate that it's a sub string, but do not terminate it with a single quote. The string ends when it reaches the closing bracket 
    //basically: ['subformat]
    logger.trace("(basic formatting) - [^'(recursive formatting)]");
    
    //because the quotes don't need to be terminated, you don't need to escape the single quotes within the string
    logger.trace("['other single quotes don't have to be escaped: 'this']");

    //the formatting options applied to the outer format will be applied to the inner text output
    logger.trace("['this is a [^'sub {$str}]]", "FORMAT");
    logger.trace("[$'THIS TEXT IS LOWERCASE [^'this text is also lowercase]]");

    //of course when you modify spaces, the spaces on the outer format will be applied to the result of the inner formats
    //this allows you to do alignment much better, here's an example
    //if using printf, you cannot easily align inner items
    //it leads to things like colons appearing out of place - 
    logger.trace("{25str}: {str}", "Weird colon", "ALIGNED spaces");
    logger.trace("{25str}: {str}", "Colon", "ALIGNED spaces");
    //notice how the colon doesn't respond to the space the first string took up. In printf, this can be solved, but not in a quick way
    //to fix that in this logger program, use recursive formatting! Like so
    logger.trace("[25'{str}:] {str}", "Normal colon", "ALIGNED spaces");
    logger.trace("[25'{str}:] {str}", "Colon", "ALIGNED spaces");
    //notice how the text is still properly aligned, but the colon is attached to the first string instead of being in the middle

    //notice how the quotes are added on to the end 
    logger.trace("['this is a [^'sub format']'] -> The quotes were added to the end of the formatting because you don't need a closing single quote");

    //you can use the escape back slash to print raw braces the same way as before
    logger.trace("This are some braces in a sub format: ['\\{\\}\\[\\]\\}\\{\\]\\[]\\\\");

    //of course you can access all parameters and variables as previously shown. The arguments will still be consumed in the order they appear within the string.
    logger.trace("[10'{str}:] [>12.th]", "Hours");
    logger.trace("[10'{str}:] [>12.tm]", "Minutes");
    logger.trace("[10'{str}:] [>12.ts]", "Seconds");
    logger.trace("[10'{str}:] [>12.tl]", "Millis");
    logger.trace("[10'{str}:] [>12.ti]", "Micros");

    logger.trace("[lbc][rbc][bks]");
    logger.trace("{char}{char}{char}", '[', ']', '\\');

    logger.trace("Total program runtime millis: [tl]");

    return 0;
}