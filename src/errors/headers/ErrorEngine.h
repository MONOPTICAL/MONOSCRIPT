#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>

class ErrorEngine {
public:
    // Singleton pattern implementation
    ErrorEngine(const ErrorEngine&) = delete;
    ErrorEngine& operator=(const ErrorEngine&) = delete;
    
    static ErrorEngine& getInstance() {
        static ErrorEngine instance;
        return instance;
    }
    
    // Initialize with source code
    void initialize(const std::vector<std::string>& sourceLinesParam) {
        sourceLines = &sourceLinesParam;
        errorCount = 0;
        warningCount = 0;
    }

    // Initialize without source code
    void initialize() {
        sourceLines = {};
        errorCount = 0;
        warningCount = 0;
    }

    // Error reporting methods
    void report(int line, int column, const std::string& module, const std::string& message, const std::string& errorType = "Unknown Error");
    void reportWithHint(int line, int column, const std::string& module, const std::string& message, const std::string& hint, const std::string& errorType = "Unknown Error");
    void warn(int line, int column, const std::string& message);

    // Statistics
    int getErrorCount() const;
    int getWarningCount() const;

private:
    // Private constructor for singleton
    ErrorEngine() : sourceLines(nullptr), errorCount(0), warningCount(0) {}
    
    const std::vector<std::string>* sourceLines = nullptr;
    int errorCount;
    int warningCount;

    // Error formatting and output
    void printError(int line, int column, const std::string& errorType, const std::string& message, const std::string& hint = "");
    void printSourceLine(int line, int column);
    std::string generatePointer(int column);
};