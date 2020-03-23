#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_READERS	10
#define NUM_THREADS	11    // 10 readers and 1 writer
#define ITER 		40

pthread_mutex_t	mutex_writers;
pthread_mutex_t	mutex_readers;
int		num_readers;
int		resource = 0;

void reader(void) {
	int j;
	int s;
	double k;

	for(j=0 ; j < ITER; j++) {
		k = (double) rand_r((unsigned int *) &s) / RAND_MAX;	
		usleep((int) 1+(k * 6000000));
        
		if (pthread_mutex_lock(&mutex_readers)!=0)
			printf(".......  Error in mutex lock 1 \n");
		num_readers++;

		if (num_readers == 1) {
			printf("First %lu \n", (unsigned long int) pthread_self());
			pthread_mutex_lock(&mutex_writers);
		}

		if (pthread_mutex_unlock(&mutex_readers)!=0)
			printf(".......  Error in mutex unlock 1 \n");

		printf("       Executing reader %lu reads  %d \n", (unsigned long int) pthread_self(), resource );

		if (pthread_mutex_lock(&mutex_writers)!=0)
			printf(".......  Error in mutex lock 2 \n");
		num_readers--;

		if (num_readers == 0){
			printf("Last %lu \n", (unsigned long int) pthread_self());
			if( pthread_mutex_unlock(&mutex_writers) !=0)
				printf("________________ Error in unlock\n");
		}

		if (pthread_mutex_unlock(&mutex_readers)!=0)
			printf(".......  Error in mutex unlock 2 \n");
	}
	
	pthread_exit(NULL);

}

void writer(void) {
	int j;
	int s;
	double k;

	for(j=0 ; j < ITER; j++) {
		k = (double) rand_r((unsigned int *) &s) / RAND_MAX;	
		usleep((int) 2 + (k * 5000000)); 

		pthread_mutex_lock(&mutex_writers);

		resource = resource + 1;
		printf("                                                          Writer: New value %d\n", resource);

		pthread_mutex_unlock(&mutex_writers);
	
	}
	
	pthread_exit(NULL);

}

int main(int argc, char *argv[])
{
	int j;
	pthread_attr_t attr;
	pthread_t thid[NUM_THREADS];
	struct timeval t;
	pthread_mutexattr_t   mattr;

	gettimeofday(&t, NULL);
	srand(t.tv_sec);

	pthread_attr_init(&attr);

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);


	pthread_mutex_init(&mutex_writers, &mattr);
	pthread_mutex_init(&mutex_readers, &mattr);


	for (j = 0; j < NUM_READERS; j++)
		if (pthread_create(&thid[j], NULL, (void *) reader, NULL) == -1){
			printf("Error creating a reader\n");
			exit(0);
		}
	if (pthread_create(&thid[j], NULL, (void *) writer, NULL) == -1){
		printf("Error creating a writer\n");
		exit(0);
	}

	for (j = 0; j < NUM_THREADS; j++)
		pthread_join(thid[j], NULL);

	exit(0);

}
	

	

