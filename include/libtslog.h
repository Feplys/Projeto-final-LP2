#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <sstream>

namespace tslog {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
private:
    static std::unique_ptr<Logger> instance_;
    static std::mutex instance_mutex_;
    
    std::ofstream log_file_;
    std::mutex log_mutex_;
    LogLevel min_level_;
    bool console_output_;
    bool file_output_;
    std::string filename_;

    Logger() = default;
    
    std::string get_timestamp() const;
    std::string level_to_string(LogLevel level) const;
    std::string get_thread_id() const;
    
public:
    // Padrão Singleton
    static Logger& getInstance();
    
    // Configuração
    void configure(const std::string& filename = "", 
                   LogLevel min_level = LogLevel::INFO,
                   bool console = true, 
                   bool file = true);
    
    // Métodos de logging
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    // Flush manual
    void flush();
    
    // Destrutor
    ~Logger();
    
    // Desabilitar cópia e atribuição
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Macros para facilitar o uso
#define LOG_DEBUG(msg) tslog::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) tslog::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) tslog::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) tslog::Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) tslog::Logger::getInstance().critical(msg)

} // namespace tslog

#endif // LIBTSLOG_H
