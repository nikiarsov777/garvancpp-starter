#ifndef LOGGER_H
#define LOGGER_H

#include "../crow.h"

class Logger : public crow::ILogHandler {
public:
    void log(std::string message, crow::LogLevel level) override {
        // Проверяваме нивото и добавяме префикс ръчно,
        // тъй като Crow подава само "чистото" съобщение
        switch (level) {
        case crow::LogLevel::Debug:
            std::clog << "[DEBUG] " << message << std::endl;
            break;
        case crow::LogLevel::Info:
            std::clog << "[INFO]  " << message << std::endl;
            break;
        case crow::LogLevel::Warning:
            std::clog << "[WARN]  " << message << std::endl;
            break;
        case crow::LogLevel::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
        default:
            std::clog << "[LOG]   " << message << std::endl;
            break;
        }
    }
};


#endif // LOGGER_H
