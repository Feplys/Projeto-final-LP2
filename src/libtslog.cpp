#include "libtslog.h"
#include <iostream>
#include <iomanip>
#include <thread>

namespace tslog {

// Inicialização dos membros estáticos
std::unique_ptr<Logger> Logger::instance_ = nullptr;
std::mutex Logger::instance_mutex_;

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<Logger>(new Logger());
        // Configuração padrão inicial
        instance_->configure("app.log", LogLevel::INFO, true, true);
    }
    return *instance_;
}

void Logger::configure(const std::string& filename, 
                       LogLevel min_level,
                       bool console, 
                       bool file) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    min_level_ = min_level;
    console_output_ = console;
    file_output_ = file;
    filename_ = filename;
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    if (file_output_ && !filename_.empty()) {
        log_file_.open(filename_, std::ios::app);
        if (!log_file_.is_open()) {
            std::cerr << "[TSLOG ERROR] Não foi possível abrir o arquivo de log: " << filename_ << std::endl;
            file_output_ = false;
        }
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) {
        return;
    }
    
    // Formata a mensagem fora do lock para otimizar
    std::stringstream formatted_msg;
    formatted_msg << "[" << get_timestamp() << "]"
                  << "[" << level_to_string(level) << "]"
                  << "[" << get_thread_id() << "] "
                  << message;
    
    std::string final_msg = formatted_msg.str();
    
    // Bloqueia apenas para a escrita
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (console_output_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << final_msg << std::endl;
        } else {
            std::cout << final_msg << std::endl;
        }
    }
    
    if (file_output_ && log_file_.is_open()) {
        log_file_ << final_msg << std::endl;
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    if (log_file_.is_open()) {
        log_file_.flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

std::string Logger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG   ";
        case LogLevel::INFO:     return "INFO    ";
        case LogLevel::WARNING:  return "WARNING ";
        case LogLevel::ERROR:    return "ERROR   ";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN ";
    }
}

std::string Logger::get_thread_id() const {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

} // namespace tslog
