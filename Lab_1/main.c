#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Глобальные переменные для синхронизации
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
int ready = 0; // Флаг состояния события

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

        // Устанавливаем флаг события
        ready = 1;
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

        // Сбрасываем флаг события и выводим сообщение
        ready = 0;
        printf("Потребитель: событие обработано\n");

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
