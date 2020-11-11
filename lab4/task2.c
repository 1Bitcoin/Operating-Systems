#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>


int main()
{
	int first_child = fork();

	if (first_child == -1)
	{
		perror("can't fork.");
		return 1;
	}

	if (first_child == 0)
	{
		printf("first_child: pid=%d;		group=%d;		parent=%d\n", getpid(), getpgrp(), getppid());

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

		printf("second_child: pid=%d;		group=%d;		parent=%d\n", getpid(), getpgrp(), getppid());

		return 0;
	}
	
	if (first_child != 0 && second_child != 0)
	{
		printf("Parent: pid=%d;		group=%d;	first_child=%d; 	second_child=%d\n", getpid(), getpgrp(), first_child, second_child);

		int first_finished_status;
		int second_finished_status;
		
		pid_t first_finished_child_pid = wait(&first_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("child has finished: PID= %d\n", first_finished_child_pid);

		if (WIFEXITED(first_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n", WEXITSTATUS(first_finished_status));

		else
			printf("child terminated abnormally");

		pid_t second_finished_child_pid = wait(&second_finished_status); // Ждет завершения любого процесса и возвращает его PID.

		printf("second_finished_child_pid has finished: PID= %d\n", second_finished_child_pid);

		if (WIFEXITED(second_finished_status)) // Ненулевой, если дочерний процесс завершен нормально.
			printf("child exited with code %d\n", WEXITSTATUS(second_finished_status));

		else
			printf("child terminated abnormally");


		return 0;
	}
}