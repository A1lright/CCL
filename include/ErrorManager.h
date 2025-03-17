#pragma once

#include <vector>
#include <string>
#include <iostream>

enum class ErrorLevel
{
    INFO,    // 信息
    WARNING, // 警告
    ERROR    // 错误
};

enum class ErrorType
{
    LexicalError,  // 词法错误
    SyntaxError,   // 语法错误
    SemanticError, // 语义错误
    CodeGenError   // 代码生成错误
};

struct ErrorInfo
{
    ErrorLevel level;    // 错误级别（INFO, WARNING, ERROR）
    char errorCode;      // 规范中的a-m错误码
    int lineNumber;      // 错误行号
    std::string message; // 错误描述
    ErrorType type;      // 错误类型

    ErrorInfo(ErrorLevel l, char code, int line, const std::string &msg, ErrorType t)
        : level(l), errorCode(code), lineNumber(line), message(msg), type(t) {}
};

class ErrorManager
{
private:
    std::vector<ErrorInfo> errors_;
    bool hasFatalError_ = false;

public:
    // 单例模式获取实例
    static ErrorManager &getInstance()
    {
        static ErrorManager instance;
        return instance;
    }

    // 添加错误信息
    void addError(ErrorLevel level, char code, int line, const std::string &msg, ErrorType type)
    {
        if (level == ErrorLevel::ERROR)
        {
            hasFatalError_ = true; // 如果是错误，设置为致命错误
        }
        errors_.emplace_back(level, code, line, msg, type);
    }

    // 打印错误信息
    void reportErrors() const
    {
        for (const auto &error : errors_)
        {
            std::string levelStr;
            switch (error.level)
            {
            case ErrorLevel::INFO:
                levelStr = "[INFO]";
                break;
            case ErrorLevel::WARNING:
                levelStr = "[WARNING]";
                break;
            case ErrorLevel::ERROR:
                levelStr = "[ERROR]";
                break;
            }

            std::string typeStr;
            switch (error.type)
            {
            case ErrorType::LexicalError:
                typeStr = "Lexical Error";
                break;
            case ErrorType::SyntaxError:
                typeStr = "Syntax Error";
                break;
            case ErrorType::SemanticError:
                typeStr = "Semantic Error";
                break;
            case ErrorType::CodeGenError:
                typeStr = "Code Generation Error";
                break;
            }

            // 输出错误信息
            std::cerr << levelStr << " (" << typeStr << ") "
                      << "Error Code: " << error.errorCode
                      << " at Line " << error.lineNumber
                      << ": " << error.message << std::endl;
        }
    }

    // 检查是否存在错误
    bool hasErrors() const { return !errors_.empty(); }

    // 检查是否存在致命错误
    bool hasFatal() const { return hasFatalError_; }

    // 清空错误信息
    void clear()
    {
        errors_.clear();
        hasFatalError_ = false;
    }
};
