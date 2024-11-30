#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>

sem_t resourceSemaphore;
sem_t whiteSemaphore;
sem_t blackSemaphore;

int whiteCount = 0;
int blackCount = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void enterWhite() {
    pthread_mutex_lock(&lock);

    if (blackCount > 0) {
        pthread_mutex_unlock(&lock);
        sem_wait(&whiteSemaphore);
    } else {
        whiteCount++;
        pthread_mutex_unlock(&lock);
    }

    sem_post(&resourceSemaphore);
    printf("Process %d (white) is now using the resource.\n", getpid());
}

void exitWhite() {
    pthread_mutex_lock(&lock);
    whiteCount--;
    if (whiteCount == 0) {
        while (sem_trywait(&blackSemaphore) == 0) {
            sem_post(&resourceSemaphore);
        }
    }
    pthread_mutex_unlock(&lock);
    printf("Process %d (white) has finished using the resource.\n", getpid());
}

void enterBlack() {
    pthread_mutex_lock(&lock);

    if (whiteCount > 0) {
        pthread_mutex_unlock(&lock);
        sem_wait(&blackSemaphore);
    } else {
        blackCount++;
        pthread_mutex_unlock(&lock);
    }

    sem_post(&resourceSemaphore);
    printf("Process %d (black) is now using the resource.\n", getpid());
}

void exitBlack() {
    pthread_mutex_lock(&lock);
    blackCount--;
    if (blackCount == 0) {
        while (sem_trywait(&whiteSemaphore) == 0) {
            sem_post(&resourceSemaphore);
        }
    }
    pthread_mutex_unlock(&lock);
    printf("Process %d (black) has finished using the resource.\n", getpid());
}

void whiteProcess() {
    enterWhite();
    sleep(1);
    exitWhite();
}

void blackProcess() {
    enterBlack();
    sleep(1);
    exitBlack();
}

int main() {
    sem_init(&resourceSemaphore, 0, 1);
    sem_init(&whiteSemaphore, 0, 0);
    sem_init(&blackSemaphore, 0, 0);

    pid_t pids[10];
    int i;

    for (i = 0; i < 5; i++) {
        if ((pids[i] = fork()) == 0) {
            whiteProcess();
            exit(0);
        }

        if ((pids[i + 5] = fork()) == 0) {
            blackProcess();
            exit(0);
        }
    }

    for (i = 0; i < 10; i++) {
        waitpid(pids[i], NULL, 0);
    }

    sem_destroy(&resourceSemaphore);
    sem_destroy(&whiteSemaphore);
    sem_destroy(&blackSemaphore);
    pthread_mutex_destroy(&lock);

    printf("All processes have finished.\n");
    return 0;
}

