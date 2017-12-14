#include <stdio.h>					/* Used for printf() */
#include <stdlib.h>					/* Used for malloc() 
							 * Used for strtoul() */
#include <unistd.h>					/* Used for pipe() 
							 * Used for read()
							 * Used for write() */			

#include <sys/wait.h>					/* Used for waitpid() */
#include <string.h>					/* Used for strlen() */
#include <errno.h>					/* Global variable errno */

#include "primes.h"					/* Declares is_prime() 
							 * Declares worker() */
						
int main(int argc, char* argv[])
{

	pid_t *pids;					/* Array that will hold all process IDs */
	int *fds_wr;					/* Array that hold all file descriptors for the parent and every child process */
	int *fds_rw;
	size_t childs;					/* Hold the amount of childs that are active */
	int i, status, index, element;			/* i is used as a loop counter variable. 
							 * status is used to check if there are problems with waitpid() 
							 * index is used      
							 * element is used for argv[] to child */	
	pid_t pid;					/* Temporarry hold the process ID of a child process that terminated */
					
	child_d *data;					/* Data structure sent by any child process */

	pids = malloc(sizeof(pid_t) * (NUM_THREADS));	/* Allocate enough memory to hold all child process IDs */
	fds_wr = malloc(sizeof(int) * (2 * NUM_THREADS));/* Allocate space for a dynamic int array (parent write, child read) */
	fds_rw = malloc(sizeof(int) * (2 * NUM_THREADS));/* Allocate space for a dynamic int array (parent read, child write) */

	data = malloc(sizeof(child_d)+4);		/* Allocate space for all the structure members */
		
	childs = NUM_THREADS;				/* The amount of child processes that need to be created */
	
	for(i = 0; i < childs; i++)			/* Loop untill the number of childs is met */
	{
		pipe(&fds_wr[2*i]);			/* Create 2 file descriptors (read and write) for every child process */
		pipe(&fds_rw[2*i]);			/* Create 2 file descriptors (read and write) for every child process */
	}
	

	for(i = 0; i < childs; i++)
	{
		if((pids[i] = fork()) < 0)		/* If a fork failed to create a child process */
		{
			printf("[ERROR] fork()\n");		
		} else if (!(pids[i]))			/* Only the child is would enter this statement */
		{
			printf(CHILD_CREAT(getpid()));	/* Print to stdout that the child created succesfully */

			if(close(fds_wr[((2*i)+1)]) < 0) printf("Closing file desc failed %s\n", strerror(errno));			
			if(close(fds_rw[(2*i)]) < 0) printf("Closing file desc failed %s\n", strerror(errno));

							/* Call the worker() subroutine with the read- and write file descriptor */
			worker(fds_wr[(2*i)], fds_rw[((2*i)+1)]);
			return 1;
		} else					/* Only the parent would enter this statement */
		{
			if(close(fds_wr[(2*i)]) < 0) printf("Closing file desc failed %s\n", strerror(errno));
			if(close(fds_rw[((2*i)+1)]) < 0) printf("Closing file desc failed %s\n", strerror(errno));
		}
	}

	for(index = 0; index < childs; index++)		
	{
							/* Give all child processes a task. If they return they get a new task */
		write(fds_wr[((2*index)+1)], argv[(index+1)], 4);
	}
	
	while(1)
	{
		for(i = 0; i < childs; i++)
		{
			if(read(fds_rw[(2*i)], data, sizeof(child_d)) > 0)	
			{
				printf("[RESULT] Number %ul is %d (returned from ID %d)\n", data->numb, data->res, data->pid);
				
				index = ((((int) data->pid) - ((int) pids[0])) * 2) + 1;		
				if(write(fds_wr[index], argv[(element++)+1], 4) < 0)  printf("%s\n",strerror(errno));
				
			}
		}
	}
		

	free(pids);					/* Release the allocated memory that holded all child process IDs */
	free(fds_wr);					/* Release the allocated memory that holded all file descriptors */
	free(fds_rw);

	return 0;					/* Return 0 to the OS (operation succesfull) */
}

int is_prime(unsigned long number)
{
	unsigned long ul;				

	if(number < 2) return 0;			/* All numbers less than 2 are not prime numbers */
	for(ul = 2; ul < number; ul++)			/* Loop from 2 to (number-1) and check if dividing is possible (no floating point) */
	{
		if(!(number % ul)) return 0;		/* Divide the target by the temporarry variable. Result is 0? 
						 	 * Yes	-> This is not a prime number (return 0)
			 				 * No	-> Continue the loop and check the next temporarry variable */
	}

	return 1;					/* If we arrived here, the number must be a prime number (return 1)*/
}

void worker(int read_f, int write_f)
{	
	char buffer[4];				
	unsigned long number;				 
	int ret;
	child_d data;
	
	while(1)
	{
		if(read(read_f, buffer, 4) > 0)	
		{			
			number = strtoul(buffer, NULL, 0);
			if(errno == EINVAL) printf("%s\n", strerror(errno));
			ret = is_prime(number);						
		
			data.pid = getpid();
			data.numb = number;
			data.res = ret;

			write(write_f, &data, sizeof(data));
		}	
	}
	
	 
}
