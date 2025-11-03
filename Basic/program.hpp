/*
 * File: program.h
 * ---------------
 * This interface exports a Program class for storing a BASIC
 * program.
 */

#ifndef _program_h
#define _program_h

#include <string>
#include <map>
#include <unordered_map>
#include <climits>
#include "statement.hpp"

class Statement;

/*
 * This class stores the lines in a BASIC program.  Each line
 * in the program is stored in order according to its line number.
 * Moreover, each line in the program is associated with two
 * components:
 *
 * 1. The source line, which is the complete line (including the
 *    line number) that was entered by the user.
 *
 * 2. The parsed representation of that statement, which is a
 *    pointer to a Statement.
 */

class Program {

public:

    // Constructs an empty BASIC program.
    Program();

    // Frees any heap storage associated with the program.
    ~Program();

    // Removes all lines from the program.
    void clear();

    // Adds/replaces a source line with the specified line number.
    void addSourceLine(int lineNumber, const std::string& line);

    // Removes the line with the specified number, if it exists.
    void removeSourceLine(int lineNumber);

    // Returns the full source line (including line number) or empty string if not found.
    std::string getSourceLine(int lineNumber);

    // Sets the parsed statement for a line. Line must exist.
    void setParsedStatement(int lineNumber, Statement *stmt);

    // Retrieves the parsed statement pointer (nullptr if none).
    Statement *getParsedStatement(int lineNumber);

    // Returns the first line number in the program, or -1 if none.
    int getFirstLineNumber();

    // Returns the next line number after the given one, or -1 if none.
    int getNextLineNumber(int lineNumber);

    // Utilities used during RUN/flow control
    bool hasLine(int lineNumber) const;

    // Control-flow override for next line set by statements like GOTO/END/IF
    void setNextLineOverride(int lineNumber);   // use -1 to terminate
    void clearNextLineOverride();               // clears any override
    int resolveNextLine(int defaultNext) const; // returns override if set, else defaultNext

private:
    // map from line number to original source line (including number)
    std::map<int, std::string> sourceLines_;
    // parsed statements for lines (owned pointers)
    std::unordered_map<int, Statement*> parsed_;

    // next-line override during RUN loop
    int nextLineOverride_ = INT_MIN; // sentinel meaning "not set"
};

#endif
