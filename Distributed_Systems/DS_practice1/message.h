#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#define MAXSIZE 256
/* Operation codes of the messages*/
#define COP_DESTROY 0
#define COP_INIT 1
#define COP_GET 2
#define COP_SET 3


struct request_t {
	int cop; /*Operation code. 1 to init, 0 to destroy, 2 to get, 3 to set*/
	char vector_name[MAXSIZE]; /*vector name */
	int pos; /*position to operate in the queue*/
	uint64_t val; /*Value to operate with*/
	char p_name[MAXSIZE]; /*name of the queue sending the request*/
};

struct reply_t {
	int errno;
	int value; //Value to return. Only used by "get" request; 
};

#endif