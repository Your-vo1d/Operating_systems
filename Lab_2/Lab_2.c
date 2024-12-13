#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

// Максимальное количество клиентов
const int CLIENT_LIMIT = 2;

// Переменная для отслеживания сигнала
volatile sig_atomic_t signalFlag = 0;

// Структура для хранения информации о клиенте
typedef struct {
    int clientSocket;             // Дескриптор сокета клиента
    struct sockaddr_in clientAddr; // Адрес клиента
} Client;

// Обработчик сигнала SIGHUP
void handleSigHup(int signal) {
    signalFlag = 1;
}

// Настройка обработчика сигнала
void configureSignalHandler(sigset_t *prevMask) {
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = handleSigHup; // Указываем обработчик
    sa.sa_flags |= SA_RESTART; 
    sigaction(SIGHUP, &sa, NULL);

    // Блокировка сигнала SIGHUP
    sigset_t blockedMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, prevMask);
}

// Функция для создания TCP-сервера
int initializeServer(int port) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        perror("Ошибка привязки сокета");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, CLIENT_LIMIT) != 0) {
        perror("Ошибка при прослушивании сокета");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

// Основная логика обработки подключений
int handleServerConnections(int serverSocket, sigset_t prevSignalMask) {
    Client clients[CLIENT_LIMIT];
    int clientCount = 0;
    char messageBuffer[1024] = {0};

    while (1) {
        // Обработка сигнала SIGHUP
        if (signalFlag) {
            puts("Clients: ");
            for (int i = 0; i < clientCount; i++) {
                puts(" ");
            }
            puts("\n");
            signalFlag = 0;
        }

        // Подготовка дескрипторов для select
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        int maxFd = serverSocket;

        // Добавляем дескрипторы клиентов в набор
        for (int i = 0; i < clientCount; i++) {
            FD_SET(clients[i].clientSocket, &readfds);
            if (clients[i].clientSocket > maxFd) {
                maxFd = clients[i].clientSocket;
            }
        }

        // Ожидание событий с использованием pselect
        if (pselect(maxFd + 1, &readfds, NULL, NULL, NULL, &prevSignalMask) == -1) {
            if (errno == EINTR) continue;  // Игнорируем прерывания от сигналов
            return -1;
        }

        // Обработка нового подключения клиента
        if (FD_ISSET(serverSocket, &readfds) && clientCount < CLIENT_LIMIT) {
            Client *newClient = &clients[clientCount];
            int addrLen = sizeof(newClient->clientAddr);
            int newSocket = accept(serverSocket, (struct sockaddr*)&newClient->clientAddr, &addrLen);
            if (newSocket >= 0) {
                newClient->clientSocket = newSocket;
                clientCount++;
            } else {
                perror("Ошибка accept");
            }
        }

        // Чтение данных от клиентов
        for (int i = 0; i < clientCount; i++) {
            Client *client = &clients[i];
            if (FD_ISSET(client->clientSocket, &readfds)) {
                int bytesRead = read(client->clientSocket, messageBuffer, sizeof(messageBuffer) - 1);
                if (bytesRead > 0) {
                    messageBuffer[bytesRead] = '\0'; // Завершаем строку
                    printf("%s\n", messageBuffer);
                } else {
                    close(client->clientSocket);
                    puts("Соединение закрыто.");
                    clients[i] = clients[clientCount - 1];
                    clientCount--;
                    i--;
                }
            }
        }
    }
}

int main() {
    int serverSocket = initializeServer(2523);
    puts("Сервер запущен, ожидание подключений...");

    sigset_t prevSignalMask;
    configureSignalHandler(&prevSignalMask);

    int result = handleServerConnections(serverSocket, prevSignalMask);
    if (result == -1) {
        perror("Ошибка работы с pselect");
    }

    return 0;
}
