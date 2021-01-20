/*Предок и потомки обмениваются сообщениями через 
неименованный программный канал. С помощью сигнала 
меняется ход выполнения программы. Предок ждет завершения 
своих потомков. Вывод соответствующих сообщений на экран.
*/

#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

void catch_sig(int signum)
{
    printf("\nProccess catched signal #%d\n", signum);
}

int main()
{
	int fd[2]; // дескриптор _одного_ программного канала
			   // fd[0] - указывает на конец канала для чтения, fd[1] - указывает на конец канала для записи
			   // потомок унаследует открытый программный канал предка

	signal(SIGINT, catch_sig); // SIGINT — сигнал для остановки процесса пользователем с терминала. 

	if (pipe(fd) == -1) 	   // создаем неименованный канал функцией pipe()
	{
        perror("can't pipe.");

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
		close(fd[1]);

		char msg[64];
		read(fd[0], msg, 64);

		printf("\nfirst_child: reading..\n\n");
		printf("first_child: read <%s>\n", msg);

		return 0;
	}

	int second_child = fork();

	if (second_child == -1)
	{
        perror("can't fork.");
		return 1;
	}

	if (second_child == 0) 
	{
		close(fd[1]);

		char msg[64];
		read(fd[0], msg, 64);

		printf("\nsecond_child: reading..\n\n");
		printf("second_child: read <%s>\n", msg);

		return 0;
	}

	if (first_child != 0 && second_child != 0)
	{	
		close(fd[0]); 

		printf("Parent: waiting for CTRL+C signal\n");
		sleep(3600);

	    char msg1[64] = "It`s my last message, children.";
	    write(fd[1], msg1, 64); //передаём сообщение в канал

	    char msg2[64] = "It`s my last message, children.";
	    write(fd[1], msg2, 64); //передаём сообщение в канал

	    printf("\nParent: pid=%d; group=%d; first_child=%d; second_child=%d\n", getpid(), getpgrp(), first_child, second_child);

		int first_finished_status;
		int second_finished_status;
		
		pid_t first_finished_child_pid = wait(&first_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("\nchild has finished: PID= %d\n", first_finished_child_pid);

		if (WIFEXITED(first_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n\n", WEXITSTATUS(first_finished_status));

		else
			printf("child terminated abnormally");

		pid_t second_finished_child_pid = wait(&second_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("\nchild has finished: PID= %d\n", second_finished_child_pid);

		if (WIFEXITED(second_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n\n", WEXITSTATUS(second_finished_status));

		else
			printf("child terminated abnormally");
		 
		return 0;
	}
}