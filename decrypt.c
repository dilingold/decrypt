/*
I completed up to the grande level for this assignement. I implemented
the 'user specified number of processes' requirement by prompting the user
to specify how many processed they would like to use. I used this number
in a for loop to fork that many children, and to divide by to give the
processes equal work.
My program first forks however many processes the user specified, and then
each process forks children to exec for each search value. This exec command
returns a value (statval) to the parent, which is the return value of fdecrypt.

My stopping mechanism is to ask the user to specify a number of digits in the
password. The program then searches all combinations of the specified number
of digits and stops when complete. I found a way to use this integer when
turning the search values into strings as 'padding' - with 0's before the
search value so all combinations are exhausted.

In order to stop all processes once the password has been found, the parent of
the child executing fdecrypt check the statval, which is the return value from
fdecrypt. If it is equal to 0, meaning the password tried is the correct password,
the current process is killed and sends a signal to the original parent. That
parent then checks, if it is signaled with a kill signal, it knows that it must 
be from a child that found the password, so it kills all remaining children.
*/

#define _POSIX_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int max_password;
	int digits;
	int statval;
	int processes;
	pid_t *pids;
	pid_t process_id;
	int i;
	int j;
	char str[10];
	int range;
	int lower_bound;
	char lower_bound_str[10];
	char upper_bound_str[10];
	
	/*determine the maximum password number by asking user to specify
	number of digits, then multiplying by 10 for each digit  */
	max_password = 1;
	printf("Please specify number of digits in password ...");
	scanf("%d", &digits);
	for (i = 0; i < digits; i++)
	{
		max_password = max_password * 10;
	}
	
	/*ask user to specify number of processes, then initialize array
	of that number of process ids, and determine a range so that processes
	will be given equal work load */
	printf("Please specify a number of processes ... ");
	scanf("%d", &processes);
	pids = malloc(sizeof(pid_t)*processes);
	printf("%d: I'm the parent\n", getpid());
	range = max_password / processes + (max_password % processes != 0);
	
	/*in a for loop, fork a child for specified number of processes, and have
	each child be given a range of password combinations, and fork children
	for each combination to execute fdecrypt with that combination. If the
	combination returns a 0, it is correct and all processes are killed */
	for (i = 0; i < processes; i++)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{
			lower_bound = range * i;
			j = lower_bound;
			sprintf(lower_bound_str, "%0*d", digits, lower_bound);
			if (lower_bound+range-1 >= max_password)
			{
				sprintf(upper_bound_str, "%0*d", digits, max_password-1);
			}
			else
			{
				sprintf(upper_bound_str, "%0*d", digits, lower_bound+range-1);
			}
			printf("Child %d: checking values %s - %s\n", getpid(), lower_bound_str, upper_bound_str);
			while (j <= lower_bound+range-1 && j < max_password)
			{
				sprintf(str, "%0*d", digits, j);
				process_id = fork();
				if (process_id == 0)
				{
					execlp("./fdecrypt", "./fdecrypt", "myfile.txt", str, NULL);
				}
				else
				{
					wait(&statval);
					if (!WIFEXITED(statval))
					{
						printf("Child did not terminate with exit\n");
					}
				
					if (statval == 0)
					{
						printf("password %s found by child %d\n", str, getpid());
						kill(getpid(), SIGTERM);
					}
				}
				j++;
			}
			printf("Child %d: search complete\n", getpid());
			sleep(1);
			exit(0);
		}
	}

	wait(&statval);
	if (WTERMSIG(statval))
	{
			printf("Search completed\n");
			kill(0, SIGTERM);
			exit(0);
			return 1;
	}
	if (!WIFEXITED(statval))
	{
		printf("Child did not terminate with exit\n");
	}
	exit(0);
	
	
	return 1;
}