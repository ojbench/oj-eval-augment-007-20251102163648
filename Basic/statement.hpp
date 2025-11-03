/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type and subclasses
 * for each BASIC statement.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "program.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

class Statement {
public:
    Statement();
    virtual ~Statement();
    virtual void execute(EvalState &state, Program &program) = 0;
};

// REM: comment, does nothing when executed
class RemStatement : public Statement {
public:
    explicit RemStatement(TokenScanner &scanner); // consumes the rest of the line
    void execute(EvalState &state, Program &program) override;
};

// LET: assignment via expression with '=' operator
class LetStatement : public Statement {
public:
    explicit LetStatement(TokenScanner &scanner);
    ~LetStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp_ = nullptr; // must be a COMPOUND with op '='
};

// PRINT: evaluate expression and print
class PrintStatement : public Statement {
public:
    explicit PrintStatement(TokenScanner &scanner);
    ~PrintStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp_ = nullptr;
};

// INPUT: read integer into variable
class InputStatement : public Statement {
public:
    explicit InputStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
private:
    std::string var_;
};

// END: terminate program
class EndStatement : public Statement {
public:
    explicit EndStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
};

// GOTO: jump to line
class GotoStatement : public Statement {
public:
    explicit GotoStatement(TokenScanner &scanner);
    void execute(EvalState &state, Program &program) override;
private:
    int target_ = -1;
};

// IF <exp1> <op> <exp2> THEN <line>
class IfStatement : public Statement {
public:
    explicit IfStatement(TokenScanner &scanner);
    ~IfStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *lhs_ = nullptr;
    Expression *rhs_ = nullptr;
    std::string op_;
    int target_ = -1;
};

#endif
