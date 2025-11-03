/*
 * File: Basic.cpp
 * ---------------
 * BASIC interpreter main and command processing.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"
#include "statement.hpp"

/* Function prototypes */
static Statement* parseStatementForProgram(TokenScanner &scanner);
static Statement* parseStatementImmediate(const std::string &cmd, TokenScanner &scanner);
void processLine(std::string line, Program &program, EvalState &state);

/* Main program */
int main() {
    EvalState state;
    Program program;
    while (true) {
        try {
            std::string input;
            if (!std::getline(std::cin, input)) break;
            if (input.empty()) continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/* Helpers */
static Statement* parseStatementForProgram(TokenScanner &scanner) {
    std::string cmd = scanner.nextToken();
    if (cmd == "REM") return new RemStatement(scanner);
    if (cmd == "LET") return new LetStatement(scanner);
    if (cmd == "PRINT") return new PrintStatement(scanner);
    if (cmd == "INPUT") return new InputStatement(scanner);
    if (cmd == "END") return new EndStatement(scanner);
    if (cmd == "GOTO") return new GotoStatement(scanner);
    if (cmd == "IF") return new IfStatement(scanner);
    error("SYNTAX ERROR");
    return nullptr;
}

static Statement* parseStatementImmediate(const std::string &cmd, TokenScanner &scanner) {
    if (cmd == "LET") return new LetStatement(scanner);
    if (cmd == "PRINT") return new PrintStatement(scanner);
    if (cmd == "INPUT") return new InputStatement(scanner);
    // Others are not allowed in immediate mode per reference implementation
    error("SYNTAX ERROR");
    return nullptr;
}

/* Process a single input line */
void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    std::string first = scanner.nextToken();
    if (first.empty()) return;

    TokenType t = scanner.getTokenType(first);
    if (t == NUMBER) {
        int lineNumber = stringToInteger(first);
        if (!scanner.hasMoreTokens()) {
            // delete this line
            program.removeSourceLine(lineNumber);
            return;
        }
        // Parse statement portion first; only add if parsing succeeds
        TokenScanner stmtScanner;
        stmtScanner.ignoreWhitespace();
        stmtScanner.scanNumbers();
        // The current scanner is already positioned after the line number; we can
        // reconstruct the remaining substring by taking the input line and removing
        // the prefix of the line number and a following space if present.
        // Simpler approach: use the same scanner (which is already after the number)
        // to parse statement directly.
        Statement *stmt = parseStatementForProgram(scanner);
        // Store full original line (including number) and parsed stmt
        program.addSourceLine(lineNumber, line);
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate commands
    if (first == "LIST") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        int ln = program.getFirstLineNumber();
        while (ln != -1) {
            std::cout << program.getSourceLine(ln) << std::endl;
            ln = program.getNextLineNumber(ln);
        }
        return;
    } else if (first == "RUN") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        int current = program.getFirstLineNumber();
        while (current != -1) {
            Statement *stmt = program.getParsedStatement(current);
            if (!stmt) {
                // Should not happen if we parsed on insert, but be safe
                std::string src = program.getSourceLine(current);
                TokenScanner sc2; sc2.ignoreWhitespace(); sc2.scanNumbers(); sc2.setInput(src);
                std::string numTok = sc2.nextToken();
                (void) numTok; // ignore
                stmt = parseStatementForProgram(sc2);
                program.setParsedStatement(current, stmt);
            }
            program.clearNextLineOverride();
            int nextDefault = program.getNextLineNumber(current);
            stmt->execute(state, program);
            int next = program.resolveNextLine(nextDefault);
            current = next;
        }
        return;
    } else if (first == "CLEAR") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        program.clear();
        state.Clear();
        return;
    } else if (first == "QUIT") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        std::exit(0);
    }

    // Immediate executable statements: LET / PRINT / INPUT
    Statement *stmt = parseStatementImmediate(first, scanner);
    stmt->execute(state, program);
    delete stmt;
}
