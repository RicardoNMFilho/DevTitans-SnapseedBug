#include "read_logs_daemon_cpp.h"

namespace devtitans::logs {


// Função responsável por ler os logs do buffer Main, que contém os logs das aplicações e é onde é possível verificar quando ocorre a exploração da vulnerabilidade por um app
void ReadLogsDaemonCpp::readMainLogs() {

    struct logger_list *logger_list_main;
    struct log_msg log_msg_main;

    logger_list_main = android_logger_list_alloc(ANDROID_LOG_WRAP, 0, 0);
    if (!logger_list_main) {
        ALOG(LOG_INFO, "ReadLogs", "Não foi possível alocar o logger");
        return;
    }else{
        ALOG(LOG_INFO, "ReadLogs", "O logger foi alocado");
    }

    struct logger *main_logger = android_logger_open(logger_list_main, LOG_ID_MAIN);
    if (!main_logger) {
        android_logger_list_free(logger_list_main);
        return;
    }

    // Mudar a parte de java.lang.String para que seja possível reconhecer qualquer objeto que não um Placelable. (Talvez simplificar o regex para a parte incial do log apenas)
    std::regex regex_pattern_main("Bundle Key android\\.intent\\.extra\\.STREAM expected Parcelable");

    while (1) {
    int ret = android_logger_list_read(logger_list_main, &log_msg_main);

        if (ret > 0) {

            // Aqui você pode acessar a mensagem do log usando a função msg()
            char* mensagem_do_log = log_msg_main.msg();
            uint16_t tamanho_real = log_msg_main.entry.len; 
            int32_t pid_app_malicioso = log_msg_main.entry.pid;
            if (mensagem_do_log != nullptr) {
                for (unsigned int i = 0; i < log_msg_main.entry.len; ++i) {
                    mensagem_do_log[i] = (mensagem_do_log[i] == '\0') ? ' ' : mensagem_do_log[i];
                }

                // Imprima a mensagem completa, garantindo que você use o tamanho real
                // ALOG(LOG_INFO, "ReadLogs", "Mensagem do log (%u bytes): %s", tamanho_real, mensagem_do_log);

                if (std::regex_search(mensagem_do_log, regex_pattern_main)) {
                    ALOG(LOG_INFO, "ReadLogs", "Ocorreu uma tentativa de enviar um objeto inválido para o Parcelable feita pelo processo %u", pid_app_malicioso);

                    if (kill(pid_app_malicioso, SIGTERM) == 0) {
                        ALOG(LOG_INFO, "ReadLogs", "Processo %u morto com sucesso", pid_app_malicioso);
                    } else {
                        ALOG(LOG_INFO, "ReadLogs", "Erro ao matar o processo %u: %s", pid_app_malicioso, strerror(errno));
                    }
                }


            } else {
                ALOG(LOG_INFO, "ReadLogs", "Erro ao acessar a mensagem do log");
            }
        } 
    else if (ret == 0) {
        ALOG(LOG_INFO, "ReadLogs", "Nada para ler");
    } 
    else {
        ALOG(LOG_INFO, "ReadLogs", "Erro ao ler os logs: %s", strerror(-ret));
    }
    }

    android_logger_list_free(logger_list_main);
}

// Função responsável por ler os logs do Kernel, onde é possível verificar problemas de auditoria como o do instagram.
void ReadLogsDaemonCpp::readKernelLogs() {

    struct logger_list *logger_list_kernel;
    struct log_msg log_msg_kernel;


    // ANDROID_LOG_WRAP espera o buffer estar quase que completamente cheio e então pega o log, não afeta a efetividade do serviço
    logger_list_kernel = android_logger_list_alloc(ANDROID_LOG_WRAP, 0, 0);
    if (!logger_list_kernel) {
        ALOG(LOG_INFO, "ReadLogs", "Não foi possível alocar o logger");
        return;
    }else{
        ALOG(LOG_INFO, "ReadLogs", "O logger foi alocado");
    }

    struct logger *kernel_logger = android_logger_open(logger_list_kernel, LOG_ID_KERNEL);
    if (!kernel_logger) {
        android_logger_list_free(logger_list_kernel);
        return;
    }

    // Genérico para ler as mensagens que contém audit, mudar na quarta
    std::regex regex_pattern_kernel("audit");

    while (1) {
    int ret = android_logger_list_read(logger_list_kernel, &log_msg_kernel);

        if (ret > 0) {

            // Basta implementar a função para ler aqui
            
        } 
    else if (ret == 0) {
        ALOG(LOG_INFO, "ReadLogs", "Nada para ler");
    } 
    else {
        ALOG(LOG_INFO, "ReadLogs", "Erro ao ler os logs: %s", strerror(-ret));
    }
    }

    android_logger_list_free(logger_list_kernel);
}

// Faz com que as funções sejam chamadas em threads, permitindo que o serviço leia tanto logs do Main quanto do Kernel
void ReadLogsDaemonCpp::startDaemon() {
    std::thread mainLogsThread(&ReadLogsDaemonCpp::readMainLogs, this);
    std::thread kernelLogsThread(&ReadLogsDaemonCpp::readKernelLogs, this);

    mainLogsThread.join();
    kernelLogsThread.join();
}

}

int main() {
    devtitans::logs::ReadLogsDaemonCpp daemon;
    daemon.startDaemon();
    return 0;
}
