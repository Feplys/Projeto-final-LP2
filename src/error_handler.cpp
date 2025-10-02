#include "error_handler.h"
#include "libtslog.h"
#include <iostream>

namespace chat {

void ErrorHandler::handle_exception(const std::exception& e) {
    LOG_CRITICAL("Exceção capturada: " + std::string(e.what()));
    
    // Tentar converter para nossa exceção customizada
    const ChatException* chat_exc = dynamic_cast<const ChatException*>(&e);
    
    std::cerr << "❌ ERRO INESPERADO: " << e.what() << std::endl;
    
    if (chat_exc) {
       
        std::cerr << "💡 SUGESTÃO: " << chat_exc->get_suggestion() << std::endl;
    }
}

} 


