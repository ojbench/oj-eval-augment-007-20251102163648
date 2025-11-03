/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
#include <climits>



Program::Program() = default;

Program::~Program() { clear(); }

void Program::clear() {
    for (auto &kv : parsed_) {
        delete kv.second;
    }
    parsed_.clear();
    sourceLines_.clear();
    clearNextLineOverride();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    sourceLines_[lineNumber] = line;
    auto it = parsed_.find(lineNumber);
    if (it != parsed_.end()) {
        delete it->second;
        parsed_.erase(it);
    }
}

void Program::removeSourceLine(int lineNumber) {
    auto itSrc = sourceLines_.find(lineNumber);
    if (itSrc != sourceLines_.end()) {
        sourceLines_.erase(itSrc);
    }
    auto it = parsed_.find(lineNumber);
    if (it != parsed_.end()) {
        delete it->second;
        parsed_.erase(it);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines_.find(lineNumber);
    if (it == sourceLines_.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLines_.find(lineNumber) == sourceLines_.end()) {
        delete stmt;
        return;
    }
    auto it = parsed_.find(lineNumber);
    if (it != parsed_.end()) {
        delete it->second;
    }
    parsed_[lineNumber] = stmt;
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
    auto it = parsed_.find(lineNumber);
    if (it == parsed_.end()) return nullptr;
    return it->second;
}

int Program::getFirstLineNumber() {
    if (sourceLines_.empty()) return -1;
    return sourceLines_.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLines_.upper_bound(lineNumber);
    if (it == sourceLines_.end()) return -1;
    return it->first;
}

bool Program::hasLine(int lineNumber) const {
    return sourceLines_.find(lineNumber) != sourceLines_.end();
}

void Program::setNextLineOverride(int lineNumber) {
    nextLineOverride_ = lineNumber;
}

void Program::clearNextLineOverride() {
    nextLineOverride_ = INT_MIN;
}

int Program::resolveNextLine(int defaultNext) const {
    if (nextLineOverride_ != INT_MIN) return nextLineOverride_;
    return defaultNext;
}


//more func to add
//todo


