#include "libtslog.h"
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>

void worker_thread(int thread_id, int num_messages) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay_dist(10, 200); // 10-200ms
    std::uniform_int_distribution<> level_dist(0, 4);    // 0-4 para LogLevel
    
    for (int i = 0; i < num_messages; ++i) {
        // Simula algum trabalho
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(gen)));
        
        int level_idx = level_dist(gen);
        std::string message = "Thread " + std::to_string(thread_id) + 
                              " - Mensagem " + std::to_string(i + 1);
        
        tslog::LogLevel level = static_cast<tslog::LogLevel>(level_idx);
        tslog::Logger::getInstance().log(level, message);
    }
    
    LOG_INFO("Thread " + std::to_string(thread_id) + " finalizada.");
}

void stress_test() {
    LOG_INFO("=== INICIANDO TESTE DE STRESS ===");
    
    const int NUM_THREADS = 10;
    const int MESSAGES_PER_THREAD = 20;
    
    std::vector<std::thread> threads;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker_thread, i + 1, MESSAGES_PER_THREAD);
    }
    
    LOG_INFO("Log da thread principal durante o teste.");
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    LOG_INFO("=== TESTE CONCLUÍDO ===");
    LOG_INFO("Tempo total: " + std::to_string(duration.count()) + "ms");
}

int main() {
    std::cout << "=== TESTE DA BIBLIOTECA LIBTSLOG ===" << std::endl;
    std::cout << "Logs serão salvos em 'test_libtslog.log'" << std::endl;
    
    tslog::Logger::getInstance().configure(
        "test_libtslog.log",      // arquivo
        tslog::LogLevel::DEBUG,   // nível mínimo
        true,                     // console
        true                      // arquivo
    );
    
    LOG_INFO("=== TESTE BÁSICO ===");
    LOG_DEBUG("Mensagem de debug.");
    LOG_INFO("Mensagem informativa.");
    LOG_WARNING("Mensagem de aviso.");
    LOG_ERROR("Mensagem de erro.");
    LOG_CRITICAL("Mensagem crítica.");
    
    std::cout << "\nPressione ENTER para iniciar o teste de concorrência...";
    std::cin.get();
    
    stress_test();
    
    std::cout << "\nTeste concluído! Verifique o arquivo 'test_libtslog.log'" << std::endl;
    
    return 0;
}
