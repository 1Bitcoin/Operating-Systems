#include <stdio.h>
#include <unistd.h>

int main()
{
  int childpid = fork();
  
  if (childpid == -1)
  {
       perror("Can’t fork.\n");
       return 1;
  }
  else if (childpid == 0)
  {	
  	while (1) 
        {
       	printf("child_pid: %d  \n", getpid());
        }
        return 0;
  }
  else
  {	
        while(1)  
        {
       	printf("parent_pid: %d  \n", getpid());
        }
        return 0;
  }
} 
