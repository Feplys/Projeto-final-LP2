#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "chat_exceptions.h"
#include <exception>

namespace chat {

// Códigos de erro para a aplicação
enum ErrorCode {
    SUCCESS = 0,
    GENERAL_ERROR = 1,
    CONFIG_ERROR = 2,
    NETWORK_ERROR = 3,
    LOGGER_ERROR = 4
};

class ErrorHandler {
public:
    
    static void handle_exception(const std::exception& e);
};

} 

#endif 


