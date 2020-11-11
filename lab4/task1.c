#include <stdio.h>
#include <unistd.h>


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
		//потомственный код1
		printf("first_child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid());
		sleep(2);
		printf( "first_child: pid=%d; group=%d;	parent=%d\n", getpid(), getpgrp(), getppid() );

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
		//потомственный код2
		printf("second_child: pid=%d; group=%d; parent=%d\n", getpid(), getpgrp(), getppid());
		sleep(2);
		printf( "second_child: pid=%d;	group=%d; parent=%d\n", getpid(), getpgrp(), getppid() );

		return 0;
	}

	if (first_child != 0 && second_child != 0) // можно опустить?
	{
		//родительский код
        printf("Parent: pid=%d;	group=%d; first_child=%d; second_child=%d\n", getpid(), getpgrp(), first_child, second_child);
		return 0;
	}
}