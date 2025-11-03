/*
 * File: statement.cpp
 * -------------------
 * Implements the Statement class and its subclasses.
 */

#include "statement.hpp"

#include <unordered_set>

#include <vector>

/* Base class */
Statement::Statement() = default;
Statement::~Statement() = default;

/* REM */
RemStatement::RemStatement(TokenScanner &/*scanner*/) {
    // consume the rest of the line silently
}
void RemStatement::execute(EvalState &/*state*/, Program &/*program*/) {
    // no-op
}

/* LET */
LetStatement::LetStatement(TokenScanner &scanner) {
    // Parse expression and ensure it is an assignment
    Expression *e = parseExp(scanner);
    if (e->getType() != COMPOUND || ((CompoundExp*) e)->getOp() != "=") {
        delete e;
        error("SYNTAX ERROR");
    }
    exp_ = e;
}
LetStatement::~LetStatement() {
    delete exp_;
}
void LetStatement::execute(EvalState &state, Program &/*program*/) {
    (void) exp_->eval(state);
}

/* PRINT */
PrintStatement::PrintStatement(TokenScanner &scanner) {
    exp_ = parseExp(scanner);
}
PrintStatement::~PrintStatement() {
    delete exp_;
}
void PrintStatement::execute(EvalState &state, Program &/*program*/) {
    int v = exp_->eval(state);
    std::cout << v << std::endl;
}

/* INPUT */
static bool isKeyword(const std::string &s) {
    static const std::unordered_set<std::string> keys = {
            "REM","LET","PRINT","INPUT","END","GOTO","IF","THEN","RUN","LIST","CLEAR","QUIT","HELP"
    };
    return keys.count(s) > 0;
}

InputStatement::InputStatement(TokenScanner &scanner) {
    std::string t = scanner.nextToken();
    if (t.empty()) error("SYNTAX ERROR");
    TokenType tp = scanner.getTokenType(t);
    if (!(tp == WORD || tp == NUMBER)) error("SYNTAX ERROR");
    if (isKeyword(t)) error("SYNTAX ERROR");
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
    var_ = t;
}
void InputStatement::execute(EvalState &state, Program &/*program*/) {
    while (true) {
        std::cout << " ? ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            // Treat EOF as invalid and continue prompting (or break)
            return;
        }
        line = trim(line);
        try {
            int v = stringToInteger(line);
            state.setValue(var_, v);
            break;
        } catch (ErrorException &/*ex*/) {
            std::cout << "INVALID NUMBER" << std::endl;
        }
    }
}

/* END */
EndStatement::EndStatement(TokenScanner &scanner) {
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
void EndStatement::execute(EvalState &/*state*/, Program &program) {
    program.setNextLineOverride(-1);
}

/* GOTO */
GotoStatement::GotoStatement(TokenScanner &scanner) {
    std::string t = scanner.nextToken();
    if (t.empty() || scanner.getTokenType(t) != NUMBER) error("SYNTAX ERROR");
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
    target_ = stringToInteger(t);
}
void GotoStatement::execute(EvalState &/*state*/, Program &program) {
    if (!program.hasLine(target_)) error("LINE NUMBER ERROR");
    program.setNextLineOverride(target_);
}

/* IF */
static Expression* parseExpFromTokens(const std::vector<std::string> &toks) {
    std::string s;
    for (size_t i = 0; i < toks.size(); ++i) {
        if (i) s += ' ';
        s += toks[i];
    }
    TokenScanner sc;
    sc.ignoreWhitespace();
    sc.scanNumbers();
    sc.setInput(s);
    return parseExp(sc);
}

IfStatement::IfStatement(TokenScanner &scanner) {
    std::vector<std::string> lhsT, rhsT;
    int depth = 0;
    std::string tok;
    // read lhs until relational op at depth 0
    while (true) {
        tok = scanner.nextToken();
        if (tok.empty()) error("SYNTAX ERROR");
        if (tok == "(") { depth++; lhsT.push_back(tok); continue; }
        if (tok == ")") { depth--; if (depth < 0) error("SYNTAX ERROR"); lhsT.push_back(tok); continue; }
        if (depth == 0 && (tok == "=" || tok == "<" || tok == ">")) { op_ = tok; break; }
        lhsT.push_back(tok);
    }
    // read rhs until THEN
    depth = 0;
    while (true) {
        tok = scanner.nextToken();
        if (tok.empty()) error("SYNTAX ERROR");
        if (tok == "(") { depth++; rhsT.push_back(tok); continue; }
        if (tok == ")") { depth--; if (depth < 0) error("SYNTAX ERROR"); rhsT.push_back(tok); continue; }
        if (depth == 0 && tok == "THEN") break;
        rhsT.push_back(tok);
    }

    // Parse expressions with exception safety
    Expression *L = nullptr;
    Expression *R = nullptr;
    try {
        L = parseExpFromTokens(lhsT);
        R = parseExpFromTokens(rhsT);
        std::string tline = scanner.nextToken();
        if (tline.empty() || scanner.getTokenType(tline) != NUMBER) {
            delete L; delete R; error("SYNTAX ERROR");
        }
        if (scanner.hasMoreTokens()) { delete L; delete R; error("SYNTAX ERROR"); }
        lhs_ = L;
        rhs_ = R;
        target_ = stringToInteger(tline);
    } catch (...) {
        delete L; delete R; throw;
    }
}
IfStatement::~IfStatement() {
    delete lhs_;
    delete rhs_;
}
void IfStatement::execute(EvalState &state, Program &program) {
    int lv = lhs_->eval(state);
    int rv = rhs_->eval(state);
    bool cond = false;
    if (op_ == "=") cond = (lv == rv);
    else if (op_ == "<") cond = (lv < rv);
    else if (op_ == ">") cond = (lv > rv);
    else error("SYNTAX ERROR");
    if (cond) {
        if (!program.hasLine(target_)) error("LINE NUMBER ERROR");
        program.setNextLineOverride(target_);
    }
}
