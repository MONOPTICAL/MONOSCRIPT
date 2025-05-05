#include "headers/ErrorEngine.h"
#include <iostream> // Используем cerr для ошибок
#include <iomanip>
#include <cctype> // Для isspace
#include <utility> // Добавлено для std::pair

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

    // Проверяем границы перед доступом к sourceLines
    if (line >= 0 && line < lineCount) {
        // Лямбда теперь возвращает пару: очищенную строку и количество удаленных символов
        auto removeLeadingPipesAndSpaces = [](const std::string& line) -> std::pair<std::string, size_t> {
            size_t startPos = 0;
            while (startPos < line.length() && (line[startPos] == '|' || std::isspace(static_cast<unsigned char>(line[startPos])))) {
                startPos++;
            }
            return {line.substr(startPos), startPos};
        };

        const std::string& originalLine = (*sourceLines)[line];
        auto [codeLine, removedCharsCount] = removeLeadingPipesAndSpaces(originalLine);

        // Вывод номера строки и самой строки
        std::cerr << std::setw(5) << line + 1 << " | " << codeLine << std::endl;
        // Вывод указателя (передаем очищенную строку, исходную колонку и кол-во удаленных символов)
        std::cerr << std::setw(5) << "" << " | " << generatePointer(codeLine, column, removedCharsCount) << std::endl;
    } else {
        std::cerr << "    (Can't reach source code at line " << line + 1 << ")" << std::endl;
    }
}

std::string ErrorEngine::generatePointer(const std::string& cleanedLine, int originalColumn, int removedCharsCount) {
    // Корректируем колонку для очищенной строки
    int adjustedColumn = originalColumn - removedCharsCount;

    // Проверка, что скорректированная колонка находится в пределах очищенной строки
    if (adjustedColumn < 0 || adjustedColumn >= static_cast<int>(cleanedLine.length())) {
         // Если колонка вне пределов, возвращаем старый указатель '^' на исходной позиции (или пустую строку)
         if (originalColumn >= 0) {
             return std::string(originalColumn, ' ') + '^'; // Используем исходную колонку для '^'
         }
         return "";
    }

    // Находим начало слова (двигаемся влево от adjustedColumn)
    int wordStart = adjustedColumn;
    while (wordStart > 0 && !std::isspace(static_cast<unsigned char>(cleanedLine[wordStart - 1]))) {
        wordStart--;
    }

    // Находим конец слова (двигаемся вправо от adjustedColumn)
    int wordEnd = adjustedColumn;
    while (wordEnd < static_cast<int>(cleanedLine.length()) - 1 && !std::isspace(static_cast<unsigned char>(cleanedLine[wordEnd + 1]))) {
        wordEnd++;
    }

    // Рассчитываем длину выделения
    int highlightLength = wordEnd - wordStart + 1;
    if (highlightLength <= 0) { // Защита
         if (originalColumn >= 0) {
             return std::string(originalColumn, ' ') + '^';
         }
         return "";
    }

    // Генерируем строку с пробелами и символами '~'
    return std::string(wordStart, ' ') + std::string(highlightLength, '~');
}