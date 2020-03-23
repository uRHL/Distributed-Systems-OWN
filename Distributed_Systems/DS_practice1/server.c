#include "message.h"
#include <mqueue.h> /* POSIX message queue library */
#include <pthread.h> /* POSIX threads library */
#include <stdio.h> 
#include <unistd.h>
#include <string.h> 
#include <stdlib.h> /* Memory allocation library */

#define MAX_NUM_VECTORS 100
void  process_message(struct request_t*);

struct vector_t {
	char name[MAXSIZE]; // Name of the vector
	int size; // Number of elements
	int used; // Boolean value indicating if the slot in the database is free
	int * ptr; // Pointer to the first element of the vector
};

/* mutex and condition variables for the message copy*/
pthread_mutex_t mutex_msg;
int msg_not_copied = 1; /* 1 = TRUE */  
pthread_cond_t cond_msg;
char server_name[MAXSIZE] = "/mq_server_383351";
int NUM_VECTORS = 0; // Current number of vectors stored in db
int first_free_slot; /* Index of the first free position in the Database to store a new vector */

// Pointer to the "database" (memory location) of the vectors. 
struct vector_t * vectors_db; 

int main(void){
	mqd_t mq_server; 	/* server queue */
	struct request_t msg;	/* message to receive */
	struct mq_attr q_attr;	/* queue atributes */
	pthread_attr_t t_attr;	/* thread atributes */
	pthread_t thid; /* thread id */

	q_attr.mq_maxmsg = 10;
	q_attr.mq_msgsize = sizeof(struct request_t);

	vectors_db = calloc(MAX_NUM_VECTORS, sizeof(struct vector_t)); 
	
	// The server CREATES its own queue
	// Afterwards the lib_array (the client)  OPENS it
	mq_server = mq_open(server_name, O_CREAT|O_RDONLY, 0700, &q_attr);
	if (mq_server == -1) {
		perror("Can't create server queue");
		return 1;
	}

	pthread_mutex_init(&mutex_msg, NULL);
	pthread_cond_init(&cond_msg, NULL);
	
	pthread_attr_init(&t_attr);

	/* Thread attributes*/
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
	
	while (1) {
		mq_receive(mq_server, (char *) &msg, sizeof(struct request_t), 0);
		pthread_create(&thid, &t_attr, (void*) process_message, &msg);

		/* Wait for threads that are writing */
		pthread_mutex_lock(&mutex_msg);
		while (msg_not_copied)
			pthread_cond_wait(&cond_msg, &mutex_msg);
		msg_not_copied = 1;
		pthread_mutex_unlock(&mutex_msg);
	} //end while
} //end main



void  process_message(struct request_t* msg){
	struct request_t msg_local;
	mqd_t mq_client;
	struct reply_t reply;
	int it = 0; // iterator used in for loops
	
	/* threads copy message to its own memory space*/
	pthread_mutex_lock(&mutex_msg);
	memcpy((struct request_t*) &msg_local, (struct request_t*) msg, sizeof(struct request_t));
	/* Waking up the server */
	msg_not_copied = 0;

    pthread_cond_signal(&cond_msg);
	pthread_mutex_unlock(&mutex_msg);

	/* execute client request and prepare reply */
	
	switch(msg_local.cop){
		case COP_DESTROY: // Destroy
			for (it = 0; it < NUM_VECTORS; it++){
				if (!(strcmp(msg_local.vector_name, vectors_db[it].name))){// The requested vector has been found. Stop searching
					free(vectors_db[it].ptr); // Free memory occupied by vector elements 
					//Delete vector data from the "database"
					char * null_str = "\0";			
					sprintf(vectors_db[it].name, null_str); // Deleting vector name (Null string)
					vectors_db[it].size = 0; // Setting the number of elements to 0
					vectors_db[it].used = 0;
					reply.errno = 0; // Deletion successful

					NUM_VECTORS--; //Decreasing counter of the number of vectors in the dababase
					goto RET; //Return message to the client
				}
			}
			// Vector not found
			perror("The specified vector does not exist");
			reply.errno = -1;
			break;

		case COP_INIT: // Init
			/* Check that there is free space in the database */
			if (NUM_VECTORS >= MAX_NUM_VECTORS) {
				perror("The database is full");
				reply.errno = -1;
				goto RET; //Returning message to the client
			}
			if (msg_local.pos <= 0){
				perror("Invalid vector size (must be at least 1)");
				reply.errno = -1;
				goto RET; //Returning message to the client
			}
			int first_free_slot = NUM_VECTORS;
			for (it = 0; it < NUM_VECTORS; it++){
				if (!(strcmp(msg_local.vector_name, vectors_db[it].name))) {
					if (msg_local.pos == vectors_db[it].size) {
						perror("That vector already exists");
						reply.errno = 0;

					} else { // Vector exists but with different size
						perror("That name is already in use");					
						reply.errno = -1;
					}
					goto RET; //Return message to client
				}
				if (first_free_slot == NUM_VECTORS && !(vectors_db[it].used)){
					/* Updating the default free position to the smaller one. It only happens once*/
					/* This is done to reduce empty positions between vectors*/
					first_free_slot = it;
				}
			}
			// The requested vector does not exist. Creating it
			//printf("Creating vector in position %d : %d\n", it, first_free_slot);
			sprintf(vectors_db[first_free_slot].name, msg_local.vector_name);
			vectors_db[first_free_slot].size = msg_local.pos; // Number of elements in the vector
			vectors_db[first_free_slot].ptr = calloc(vectors_db[first_free_slot].size, sizeof(int)); // Allocate memory for the vector
			vectors_db[first_free_slot].used = 1;
			NUM_VECTORS++;
			// Vector created successfully
			reply.errno = 0;
			perror("Vector created successfully\n");
			break;

		case COP_GET: // Get
			for (it = 0; it < NUM_VECTORS; it++){
				if (!(strcmp(msg_local.vector_name, vectors_db[it].name))){// The requested vector has been found. Stop searching
				/* Take the starting memory location of the specified vector 
				Go to the i-th element
				Set its value to the value specified in the request */
				if (msg_local.pos < 0 || msg_local.pos >= vectors_db[it].size){ //must be met that 0 <= index < vector size
					printf("Index %d out of boudns [%d]", msg_local.pos, vectors_db[it].size);
					reply.errno = -1;
				} else {
					reply.value = vectors_db[it].ptr[msg_local.pos];
					reply.errno = 0;
				}				
				goto RET;// Return message to client
				}
			}
			// Vector not found
			perror("The specified vector does not exist");
			reply.errno = -1;
			break;
		case COP_SET:	// Set	
			for (it = 0; it < NUM_VECTORS; it++){
				if (!(strcmp(msg_local.vector_name, vectors_db[it].name))){// The requested vector has been found. Stop searching
				/* Take the starting memory location of the specified vector 
				Go to the i-th element
				Set its value to the value specified in the request */
				vectors_db[it].ptr[msg_local.pos] = msg_local.val;
				reply.errno = 0;
				goto RET; //Return message to client
				}
			}
			// Vector not found
			perror("The specified vector does not exist");
			reply.errno = -1;
			break;
		default:
			perror("Operation not found");
			reply.errno = -1;

	}
	/* return result to client queue */
	//  RET: Block of code to Prepare message for answer 
RET:mq_client = mq_open(msg_local.p_name, O_WRONLY);
	mq_send(mq_client, (char *) &reply, sizeof(struct reply_t), 0);
	mq_close(mq_client);
	pthread_exit(0);
}