#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define N 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t  full  = PTHREAD_COND_INITIALIZER;

char buffer[N];
int head  = 0;
int tail  = 0;
int count = 0;

char *message;
int msg_len = 0;

void insert_item(char item)
{
    buffer[tail] = item;
    tail = (tail + 1) % N;
    count++;
}

char remove_item(void)
{
    char item = buffer[head];
    head = (head + 1) % N;
    count--;
    return item;
}

void *producer(void *arg)
{
    int i;
    char item;

    for (i = 0; i < msg_len; i++) {
        item = message[i];

        pthread_mutex_lock(&mutex);
        if (count == N)
            pthread_cond_wait(&empty, &mutex);

        insert_item(item);

        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *consumer(void *arg)
{
    int i;
    char item;

    for (i = 0; i < msg_len; i++) {
        pthread_mutex_lock(&mutex);
        if (count == 0)
            pthread_cond_wait(&full, &mutex);

        item = remove_item();

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);

        printf("%c", item);
    }
    printf("\n");

    pthread_exit(NULL);
}

int main(void)
{
    pthread_t prod_thread, cons_thread;
    FILE *fp;

    if ((fp = fopen("message.txt", "r")) == NULL) {
        printf("ERROR: can't open message.txt!\n");
        return -1;
    }

    message = (char *)malloc(sizeof(char) * 1024);
    if (message == NULL) {
        printf("ERROR: Out of memory!\n");
        return -1;
    }

    message = fgets(message, 1024, fp);
    msg_len = strlen(message) - 1;  /*remove /n*/
    fclose(fp);

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    return 0;
}
