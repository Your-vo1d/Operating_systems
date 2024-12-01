#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

//Максимальное количество пользователей
const int MAXC_CLIENT = 1;
// Переменная-флаг, которая отслеживает обаботку сигнала
volatile sig_atomic_t wasSigHup = 0;

// Функция-обработчик сигнала SIGHUP
void sigHupHandler(int r)
{
    wasSigHup = 1;
}

// Структура для хранения информации о клинете

typedef struct Client
{
    int socket_fd; // 
    struct sockaddr_in address;  // Адрес клиента
};

// Создание TCP-сервера 
int createServerSocket(int port)
{
    struct sockaddr_in server_address; // Адрес сервера
    memset(&server_address, 0, sizeof(server_address)); // Обнуление памяти
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Создание сокета

    if (serverSocket == -1)
    {
        puts("Ошибка создания сокета\n");
        exit(-1);
    }

    //Определение сервера
    server_address.sin_family = AF_INET; //IP4
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr*) &server_address, sizeof(server_address)) != 0)
    {
        puts("Ошибка привязки сокета\n");
        close(serverSocket);
        exit(-1);
    }

    if (listen(serverSocket, MAXC_CLIENT) != 0)
    {
        puts("Ошибка при прослушивании сокета\n");
        close(serverSocket);
        exit(-1);
    }
    return serverSocket;
}


void setupSignalHandler()
{
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = sigHupHandler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);
}