#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>

#define COUNT_PRODUCERS 4
#define COUNT_CONSUMERS 4

#define BIN_SEM 0
#define BUFFER_EMPTY 1
#define BUFFER_FULL 2

#define BUF_SIZE 5

struct sembuf producer_P[] = 
{
    {BUFFER_EMPTY, -1, 0},
    {BIN_SEM, -1, 0}
};

struct sembuf producer_V[] = 
{
    {BIN_SEM, 1, 0},
    {BUFFER_FULL, 1, 0}
};

struct sembuf consumer_P[] = 
{
    {BUFFER_FULL, -1, 0},
    {BIN_SEM, -1, 0}
};
struct sembuf consumer_V[] = 
{
    {BIN_SEM, 1, 0},
    {BUFFER_EMPTY, 1, 0}
};

void Producer(int nomer, int sem_id, char* buf, int* pos, int* character)
{
    while (1) 
    {
        // производит единичный объект

        // ждет, когда освободится хотя бы одна ячейка буфера 
        // и когда или другой производитель, или потребитель выйдет из критической секции
        semop(sem_id, producer_P, 2);
        
        // положить в буфер 
        buf[*pos] = 'a' + *character;

        printf("Producer #%d -> ", nomer);
        printf("put buffer[%d] = %c\n", *pos, buf[*pos]);

        *pos = *pos == BUF_SIZE - 1 ? 0 : *pos + 1;
        *character = *character == 25 ? 0 : *character + 1;

        // освобождение критической секции и инкремент количества заполненных ячеек
        semop(sem_id, producer_V, 2);

        sleep(rand() % 2);
    }
}

void Consumer(int nomer, int sem_id, char* buf, int* pos)
{
    while (1) 
    {
        // ждет, когда будет заполнена хотя бы одна ячейка буфера
        // и когда или потребитель, или другой производитель выйдет из критической секции
        semop(sem_id, consumer_P, 2);
        
        // взять из буфера 
        printf("Consumer #%d <- ", nomer);
        printf("take buffer[%d] = %c\n", *pos, buf[*pos]);

        *pos = *pos == BUF_SIZE - 1 ? 0 : *pos + 1;
        
        // освобождение критической секции и инкремент количества пустых ячеек
        semop(sem_id, consumer_V, 2);

        sleep(rand() % 4);
    }
}

int main()
{
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int shm_id;
    int sem_id;
    char *mem_ptr = -1;

    // возвращает идентификатор разделяемому сегменту памяти
    if ((shm_id = shmget(IPC_PRIVATE, BUF_SIZE * sizeof(char) + 3 * sizeof(int), IPC_CREAT | perms)) == -1)
    {
        perror("Unable to create a shared area.\n"); 
        return 1;
    }

    // возвращает указатель на сегмент разделяемой памяти
    if ((mem_ptr = shmat(shm_id, NULL, 0)) == -1) 
    {
        perror("Can't attach memory.\n");
        return 1;
    }

    // создание набора семафоров
    if ((sem_id = semget(IPC_PRIVATE, 3, perms)) == -1) 
    {
        perror("Can’t semget.\n");
        return 1;
    }

    // изменение управляющих параметров набора семафоров
    semctl(sem_id, BIN_SEM, SETVAL, 1); 
    semctl(sem_id, BUFFER_EMPTY, SETVAL, BUF_SIZE); 
    semctl(sem_id, BUFFER_FULL, SETVAL, 0); 

    // создание процессов
    int count_processes = 0;
    pid_t pid;
    
    int* producer_pos = mem_ptr + BUF_SIZE;
    int* character = producer_pos + 1;
    int* consumer_pos = producer_pos + 2;

    *producer_pos = 0;
    *consumer_pos = 0;
    *character = 0;

    for (int i = 0; i < COUNT_PRODUCERS; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }

        if (!pid)
            Producer(i + 1, sem_id, mem_ptr, producer_pos, character);
        else
            count_processes++;
    }

    for (int i = 0; i < COUNT_CONSUMERS; i++) 
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }

        if (!pid)
            Consumer(i + 1, sem_id, mem_ptr, consumer_pos);
        else
            count_processes++;
    }

    // ожидание завершения процессов
    int status;
    for (int i = 0; i < count_processes; i++)
    {
        wait(&status);
        if (!WIFEXITED(status))
            printf("exit-error, code = %d\n", status);
    }
    
    // освобождение ресурсов
    shmdt(mem_ptr);
    semctl(sem_id, 0, IPC_RMID);
    shmctl(shm_id, IPC_RMID, 0);

    return 0;
}