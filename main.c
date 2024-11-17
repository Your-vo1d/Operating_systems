#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

// Глобальные переменные для синхронизации
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
int ready = 0; // Флаг состояния события
struct timeval event_time; // Время отправки события

// Поток-поставщик
void *producer(void *arg) {
    while (1) {
        // Блокировка мьютекса
        pthread_mutex_lock(&lock);

        // Проверка флага, чтобы избежать двойного предоставления события
        if (ready == 1) {
            pthread_mutex_unlock(&lock);
            continue;
        }

        // Устанавливаем флаг события и фиксируем время
        ready = 1;
        gettimeofday(&event_time, NULL); // Запоминаем текущее время
        printf("Поставщик: событие отправлено\n");

        // Сигнализируем потоку-потребителю
        pthread_cond_signal(&cond1);

        // Освобождаем мьютекс
        pthread_mutex_unlock(&lock);
        sleep(1); // Задержка перед следующим событием
    }
    return NULL;
}

