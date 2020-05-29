#ifndef CAMERA_H
#define CAMERA_H

#define MAX_MS_SIZE 1024
#define ID_LENGTH 10
#define MAX_SERVER_MS_SIZE
//op codes
#define SET_FREQ 1
#define SET_ANGLE 2
#define SEND_IMG 6
#define UPDATE_STATUS 7
//time elapsed (seconds) between each time the camera sends its status to central server
#define STATUS_UPDATE_FREQ 3600

/*Message exanged between server and other devices only include 
a number (degrees [-90, 90], frequency in minutes or an ip address). Thus, 
the value is stored in a char array of at most 15 bytes (longest ip addr in dotted format)
*/
struct server_req_t {
    char op;
    char ID[ID_LENGTH];
    char value[15];
};

struct status_req_t {
    char op = UPDATE_STATUS;
    char ID[ID_LENGTH];
    char status[MAX_MS_SIZE];
};

struct image_req_t {
    char op = SEND_IMG;
    char ID[ID_LENGTH];
    char img[MAX_MS_SIZE*MAX_MS_SIZE];//max 1MB
};

/*Initializes the thread in charge of listening for server requests*/
int init_server_side();
/*Process the requests arriving from the central server*/
int processReq(int *sc);
/*Collects and sends images and status to the central server*/
int gatherData();


#endif