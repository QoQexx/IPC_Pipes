#ifndef _PRIMES_H
#define _PRIMES_H

#define	NUM_THREADS		3

#define	CHILD_CREAT(proc_id)	"Child process with process ID %d has been created succesfully\n", proc_id


typedef struct
{
	pid_t pid;
	unsigned long numb;
	int res;	
} child_d;

/* Function prototyping */

int is_prime(unsigned long number);
void worker(int read_f, int write_f);



#endif
