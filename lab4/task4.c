/*Предок и потомки обмениваются сообщениями через 
неименованный программный канал. Предок ждет 
завершения своих потомков. Вывод соответствующих 
сообщений на экран.*/

#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
	int fd[2]; // дескриптор _одного_ программного канала
			   // fd[0] - указывает на конец канала для чтения, fd[1] - указывает на конец канала для записи
			   // потомок унаследует открытый программный канал предка

	if (pipe(fd) == -1) // создаем неименованный канал функцией pipe()
	{
        perror("Couldn't pipe.");
		
		return 1;
	}

    int first_child = fork();

    if (first_child == -1)
	{
        perror("can't fork.");
		
		return 1;
	}

    if (first_child == 0)
	{
		printf("\nFirst_child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid());
        close(fd[0]);

        char message_first[] = "I'm first_child";
        write(fd[1], message_first, 64); // пишет до 64 байт из буфера, на который указывает message_first в файл
        								 // на который ссылается йаловый дескриптор fd.

        return 0;
	}

    int second_child = fork();

    if (second_child == -1 )
    {
        perror("can't fork.");
        
        return 1;
    }

    if (second_child == 0 )
    {
    	printf("\nSecond_child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid());
        close(fd[0]);

        char message_second[] = "I'm second_child";
        write(fd[1], message_second, 64);

        return 0;
	}

	if (first_child != 0 && second_child != 0)
	{
		close(fd[1]);

		char msg1[64];
		read(fd[0], msg1, 64); // пытается прочитать 64 байт из файлового дескриптора fd в 
							   // буфер, начинающийся по адресу msg1

		char msg2[64];
		read(fd[0], msg2, 64);

		printf("\nParent: pid=%d; group=%d; first_child=%d; second_child=%d\n", getpid(), getpgrp(), first_child, second_child);

		printf("\nParent reads: \nfirst msg: %s \nsecond_msg: %s\n\n", msg1, msg2);

		int first_finished_status;
		int second_finished_status;
		
		pid_t first_finished_child_pid = wait(&first_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("\nchild has finished: PID= %d\n", first_finished_child_pid);

		if (WIFEXITED(first_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n\n", WEXITSTATUS(first_finished_status));

		else
			printf("child terminated abnormally");

		pid_t second_finished_child_pid = wait(&second_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("\nchild exited with code has finished: PID= %d\n", second_finished_child_pid);

		if (WIFEXITED(second_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n\n", WEXITSTATUS(second_finished_status));

		else
			printf("child terminated abnormally");


		return 0;
	}

	return 0;
}
