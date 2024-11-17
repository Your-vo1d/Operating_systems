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

// Поток-потребитель
void *consumer(void *arg) {
    while (1) {
        // Блокировка мьютекса
        pthread_mutex_lock(&lock);

        // Ожидание наступления события
        while (ready == 0) {
            // Ожидание с временным освобождением мьютекса
            pthread_cond_wait(&cond1, &lock);
            printf("Потребитель: пробуждение\n");
        }

        // Вычисляем задержку
        struct timeval current_time;
        gettimeofday(&current_time, NULL); // Текущее время
        long delay_microseconds = (current_time.tv_sec - event_time.tv_sec) * 1000000L + (current_time.tv_usec - event_time.tv_usec);
        double delay_seconds = delay_microseconds / 1000000.0; // Перевод в секунды

        // Сбрасываем флаг события и выводим сообщение
        ready = 0;
        printf("Потребитель: событие обработано (задержка %.6f секунд)\n", delay_seconds);

        // Освобождаем мьютекс
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    // Создание потоков
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Ожидание завершения потоков (в данном случае они бесконечны)
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}
