#include "headers/ErrorEngine.h"
#include <iostream> // Используем cerr для ошибок
#include <iomanip>  // Для форматирования вывода (например, setw)

int ErrorEngine::getErrorCount() const {
    return errorCount;
}

int ErrorEngine::getWarningCount() const {
    return warningCount;
}

void ErrorEngine::report(int line, int column, const std::string& module, const std::string& message, const std::string& errorType) {
    printError(line, column, errorType, message);

    if (errorType != "Unknown Error") { // Или более строгая проверка типа
        errorCount++;
    }
}

void ErrorEngine::reportWithHint(int line, int column, const std::string& module, const std::string& message, const std::string& hint, const std::string& errorType) {
    printError(line, column, errorType, message, hint);
     if (errorType != "Unknown Error") {
        errorCount++;
    }
}

void ErrorEngine::warn(int line, int column, const std::string& message) {
    printError(line, column, "Warn", message);
    warningCount++;
}

// --- Приватные методы ---

void ErrorEngine::printError(int line, int column, const std::string& errorType, const std::string& message, const std::string& hint) {
    // TODO: Добавить цвета
    std::cerr << errorType << " [line " << line + 1 << ", column " << column + 1 << "]: " << message << std::endl;

    if (sourceLines && sourceLines->size() > 0) {
        printSourceLine(line, column);
    }

    if (!hint.empty()) {
        // TODO: Добавить цвета для подсказки
        std::cerr << "    Hint: " << hint << std::endl;
    }
    throw std::runtime_error("");
}

void ErrorEngine::printSourceLine(int line, int column) {
    // Сначала проверяем, что указатель не NULL
    if (!sourceLines) {
        std::cerr << "    (Can't reach source code)" << std::endl;
        return;
    }

    int lineCount = static_cast<int>(sourceLines->size());

    std::cerr << "lineCount: " << lineCount << std::endl;  

    // Затем проверяем границы
    if (line >= 0 && line < lineCount) {
        const std::string& codeLine = (*sourceLines)[line];
        // Вывод номера строки и самой строки
        std::cerr << std::setw(5) << line + 1 << " | " << codeLine << std::endl;
        // Вывод указателя
        std::cerr << std::setw(5) << "" << " | " << generatePointer(column) << std::endl;
    } else {
        std::cerr << "    (Can't reach source code at line " << line + 1 << ")" << std::endl;
    }
}

std::string ErrorEngine::generatePointer(int column) {
    if (column >= 0) {
        // Создаем строку с пробелами до нужной колонки и ставим '^'
        return std::string(column, ' ') + '^';
    }
    return ""; // Если колонка некорректна
}