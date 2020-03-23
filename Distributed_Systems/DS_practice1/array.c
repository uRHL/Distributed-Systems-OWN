#include "message.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>           /* For O_* flags */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

char server_name[256] ="/mq_server_383351";

void get_p_name(char * str);

/*Sends a request to initialize a vector called "name" of N positions*/
int init(char * name, int N){
	mqd_t mq_server; /* server message queue*/
	mqd_t mq_client; /* client message queue*/

	struct mq_attr attr;
	attr.mq_maxmsg = 1; /*max number of messages*/ 
	attr.mq_msgsize = sizeof(struct reply_t);

	/*Creating the request*/
	struct reply_t reply;
	struct request_t req;
	req.pos = N;
	req.val = 0;
	req.cop = COP_INIT;
	/*Coping name to request*/
	sprintf(req.vector_name, "%s", name);	
	/*Obtain the name of the queue of the calling process*/
	get_p_name(req.p_name);

	

	/*Creating message queue for the client*/
	mq_client = mq_open(req.p_name, O_CREAT|O_RDONLY, 0700, &attr);
	/*Opening the server queue to send the request*/
	mq_server = mq_open(server_name, O_WRONLY);

	if (mq_server == -1){
		perror("Error connecting with the server");
		return -1;
	}else if (mq_send(mq_server, (const char *)&req, sizeof(struct request_t), 0) == -1){ 
		perror("Unable to send the request");
		return -1;
	}
	if (mq_receive(mq_client, (char *) &reply, sizeof(struct reply_t), 0) != sizeof(struct reply_t)){
		/* If mq_receive returns a number of bytes readed different from the size of the message then
		the message is corrupted. In this clause is also included the case of mq_receive returning -1, 
		which indicates that an error ocurred */
		perror("Error retrieving data from the server");
		return -1;
	}
	/* Closing the client side of the server queue. Closing the client queue. */
	mq_close(mq_server);
	mq_close(mq_client);
	/* If no error ocurred the value received from the server is returned */
	return reply.errno;
}

/*Writes value in position i of vector "name"*/
int set(char * name, int i, int value){
	mqd_t mq_server; /* server message queue*/
	mqd_t mq_client; /* client message queue*/

	struct mq_attr attr;
	attr.mq_maxmsg = 1; /*max number of messages*/ 
	attr.mq_msgsize = sizeof(struct reply_t);

	/*Creating the request*/
	struct reply_t reply;
	struct request_t req;
	req.pos = i;
	req.val = value;
	req.cop = COP_SET;
	/*Coping name to request*/
	sprintf(req.vector_name, "%s", name);	
	/*Obtain the name of the queue of the calling process*/
	get_p_name(req.p_name);

	
	/*Creating message queue for the client*/
	mq_client = mq_open(req.p_name, O_CREAT|O_RDONLY, 0700, &attr);
	/*Opening the server queue to send the request*/
	mq_server = mq_open(server_name, O_WRONLY);

	if (mq_server == -1){
		perror("Error connecting with the server");
		return -1;
	}else if (mq_send(mq_server, (const char *)&req, sizeof(struct request_t), 0) == -1){ 
		perror("Unable to send the request");
		return -1;
	}
	if (mq_receive(mq_client, (char *) &reply, sizeof(struct reply_t), 0) != sizeof(struct reply_t)){
		/* If mq_receive returns a number of bytes readed different from the size of the message then
		the message is corrupted. In this clause is also included the case of mq_receive returning -1, 
		which indicates that an error ocurred */
		perror("Error retrieving data from the server");
		return -1;
	}
	/* Closing the client side of the server queue. Closing the client queue. */
	mq_close(mq_server);
	mq_close(mq_client);
	/* If no error ocurred the value received from the server is returned */
	return reply.errno;
}

/*Gets the value from position i of vector "name"*/
int get(char * name, int i, int* value){
	mqd_t mq_server; /* server message queue*/
	mqd_t mq_client; /* client message queue*/

	struct mq_attr attr;
	attr.mq_maxmsg = 1; /*max number of messages*/ 
	attr.mq_msgsize = sizeof(struct reply_t);

	/*Creating the request*/
	struct reply_t reply;
	struct request_t req;
	req.pos = i;
	req.val = 0;
	req.cop = COP_GET;
	/*Coping name to request*/
	sprintf(req.vector_name, "%s", name);	
	/*Obtain the name of the queue of the calling process*/
	get_p_name(req.p_name);

	
	/*Creating message queue for the client*/
	mq_client = mq_open(req.p_name, O_CREAT|O_RDONLY, 0700, &attr);
	/*Opening the server queue to send the request*/
	mq_server = mq_open(server_name, O_WRONLY);

	if (mq_server == -1){
		perror("Error connecting with the server");
		return -1;
	}else if (mq_send(mq_server, (const char *)&req, sizeof(struct request_t), 0) == -1){ 
		perror("Unable to send the request");
		return -1;
	}
	if (mq_receive(mq_client, (char *) &reply, sizeof(struct reply_t), 0) != sizeof(struct reply_t)){
		/* If mq_receive returns a number of bytes readed different from the size of the message then
		the message is corrupted. In this clause is also included the case of mq_receive returning -1, 
		which indicates that an error ocurred */
		perror("Error retrieving data from the server");
		return -1;
	}
	*value = reply.value; //Coping the obtained value to the variable given by the client
	/* Closing the client side of the server queue. Closing the client queue. */
	mq_close(mq_server);
	mq_close(mq_client);

	/* If no error ocurred the value received from the server is returned */
	return reply.errno;
}


/*Destroys an existing vector*/
int destroy(char * name){
	mqd_t mq_server; /* server message queue*/
	mqd_t mq_client; /* client message queue*/

	struct mq_attr attr;
	attr.mq_maxmsg = 1; /*max number of messages*/ 
	attr.mq_msgsize = sizeof(struct reply_t);

	/*Creating the request*/
	struct reply_t reply;
	struct request_t req;
	req.pos = 0;
	req.val = 0;
	req.cop = COP_DESTROY;
	/*Coping name to request*/
	sprintf(req.vector_name, "%s", name);	
	/*Obtain the name of the queue of the calling process*/
	get_p_name(req.p_name);

	
	/*Creating message queue for the client*/
	mq_client = mq_open(req.p_name, O_CREAT|O_RDONLY, 0700, &attr);
	/*Opening the server queue to send the request*/
	mq_server = mq_open(server_name, O_WRONLY);

	if (mq_server == -1){
		perror("Error connecting with the server");
		return -1;
	}else if (mq_send(mq_server, (const char *)&req, sizeof(struct request_t), 0) == -1){ 
		perror("Unable to send the request");
		return -1;
	}
	if (mq_receive(mq_client, (char *) &reply, sizeof(struct reply_t), 0) != sizeof(struct reply_t)){
		/* If mq_receive returns a number of bytes readed different from the size of the message then
		the message is corrupted. In this clause is also included the case of mq_receive returning -1, 
		which indicates that an error ocurred */
		perror("Error retrieving data from the server");
		return -1;
	}
	/* Closing the client side 
of the server queue. Destroying the client queue. */
	mq_close(mq_server);
	mq_close(mq_client);
	mq_unlink(req.p_name);
	return reply.errno;

}

/*creates a unique name for a client message queue using its pid*/
void get_p_name(char * str){
	sprintf(str, "/client_");
	char pid[6];
	sprintf(pid, "%d", getpid());
	strcat(str, pid);
}
