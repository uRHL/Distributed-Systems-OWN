#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <uistd.h>
#include <stdio.h>
#include <time.h>
#include "camera.h"

//Unique ID od the device, may include numbers and letters
#define CAM_ID "123456789"

//Camera attributes
uint64_t frequency = 1; //Default update frequency 1 minute
int angle = 0; //default camera angle 0 degrees
char status[MAX_MS_SIZE];
time_t lastStatusUpdate = 0;
time_t lastImgSent = 0;

//Additional variables
short listening_port = 7777;
short central_server_port = 3333;
//In this case the IPs are identical in order to test the program in this computer
char ipHost = "0.0.0.0";
char ipServer = "0.0.0.0";
//file descriptor of the socket used to listen for server requests
int listening_sc;
//File descriptor of the socket used to sending requests to the server
int client_sc;

pthread_mutex_t mux_camAttr;
//Connection information for both client and server side
struct sockaddr_in cam_server_addr, central_server_addr, centrals_client_side;


int main(){
    //create mutex, attr not used (null)
    pthread_mutex_init(&mux_camAttr, NULL);

    
    //initialize thread attr
    pthread_attr_t th_attr;
    pthread_t tid;
    pthread_attr_init(&th_attr);
    pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_DETACHED);

    if (init_server_side() != 0){
        //Error
        exit(0);
    }
    
    //new thread (gatherData) with no additional parameters
    pthread_create(&tid, &t_attr, (void *)gatherData, 0);

    //listening for server requests
    int sc;
    while(1){
        //accept(int sd, struct sockaddr *addr_client, int *addrlen)
        sc = accept(listening_sc, (struct sockaddr *) &centrals_client_side, sizeof(&centrals_client_side));
        if(sc != -1){
            //create a new thread for every request
            pthread_create(&tid, &t_attr, (void *)processReq, &sc);            
        } else {
            //Error
            printf("the camera with ID %s, could not stablish connection with the server\n", CAM_ID);
        }
    }
    close(sc);
    pthread_mutex_destroy(&mux_camAttr);
    return 0;
}

/*
Initializes the socket that will be used by the server side of the camera

returns: 0 on success, -1 if there is an error with the ip addresses, 
    -2 if bind failed, -3 if listening failed
*/
int init_server_side(){
    struct in_addr in_ip;
    struct hostent * host;

    //Initialize to 0
    bzero((char *)&cam_server_addr, sizeof(cam_server_addr));

    host = (struct hostent*)gethostbyname(ipHost);
    memcopy(&(cam_server_addr.sin_addr), hp->h_addr, hp->h_length);
    cam_server_addr.sin_family = AF_INET;
    cam_server_addr.sin_port = htons(listening_port);

    //create sockect: int socket(int domain, int type, int protocol)
    listening_sc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //Ooption to 
    int val = 1;
    if (setsockopt(listening_sc, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(int)) == -1){
        //ERROR
    }
    //bind(int sd, struct sockaddr *addr, int addrlen): assign an IP
    if ( bind(listening_sc, (struct sockaddr *) &cam_server_addr, sizeof(cam_server_addr) ) == -1){
        //Error
        retunr -2;
    }
    //listen, baklog = 5 (maximum queued requests)
    if(listen(listening_sc, 5) == -1){
        //Error
        return -3;
    }
    return 0;
}


/*
    param sc = sockect were the connection with the server was opened
 */
int processReq(int * sc){
    char ans = '0';
    struct server_req_t * rq_server = alloc(sizeof(rq_server));
    char buffer[MAX_SERVER_MS_SIZE];

    //read req from sc, no flags activated
    //read op code
    recv (*sc, (char *)&buffer, sizeof(char), 0);
    memcopy(&(rq_server->op), &buffer, sizeof(char));
    //read ID
    recv (*sc, (char *)&buffer, ID_LENGTH, 0) ;
    memcopy(&(rq_server->ID), &buffer, ID_LENGTH);
    //Read value associated to the operation (may be a time lapse in minutes or an angle [-90, 90])
    recv (*sc, (char *) &buffer, MAX_SERVER_MS_SIZE, 0);
    memcopy(&(rq_server->value), &buffer, MAX_SERVER_MS_SIZE);

    
    if(rq_server->ID != CAM_ID){
        //ERROR: the IP do not target the correct device 
        ans = '1';
    }else{
        //pasrse int from op code
        int op = rq_server->op - '0';
        switch(op){
            case SET_FREQ:
                pthread_mutex_lock(&mux_camAttr);
                //Assign new frequency like an int, not a string
                sscanf(&(rq_server->value), "%d", &frequency);
                pthread_mutex_unlock(&mux_camAttr);
                break;

            case SET_ANGLE:
                pthread_mutex_lock(&mux_camAttr);
                //Assign new angle like an int, not a string
                sscanf(&(rq_server->value), "%d", &angle);
                pthread_mutex_unlock(&mux_camAttr);
                break;

            default:
                //ERROR: op code not defined
                ans = '2';
        }
    }
    //send ans to server
    send(*sc, &ans sizeof(char));
    //close connection
    close(sc);
    exit(0);
    
}

int gatherData(){
    //Initialize structures to store the different messages for the server
    struct img_req_t * img_rq = alloc(sizeof(img_rq));
    struct status_req_t * status_rq = alloc(sizeof(status_rq));
    strcpy(&(img_rq->ID), CAM_ID, ID_LENGTH);
    strcpy(&(status_rq->ID), CAM_ID, ID_LENGTH);

    struct hostent * hp;
    
    bzero((char *) &central_server_addr, sizeof(central_server_addr));
    *hp = gethostbyname(ipServer);

    memcopy(&(central_server_addr.sin_addr), hp->h_addr, hp->h_lenght);
    central_server_addr.sin_family = AF_INET;
    central_server_addr.sin_port = htons(central_server_port);
    
    while(1){
        pthread_mutex_lock(&mux_camAttr);
        if (lastImgSent + freq*60 <= time(NULL)){
            pthread_mutex_unlock(&mux_camAttr);
            if ((client_sc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
                //Error
                printf("client socket could not be created\n");
                exit();
            }
            //open connection with server
            //int connect(int sd, struct sockaddr *addr_server, int addrlen)
            if (connect(client_sc, (struct sockaddr *) &central_server_addr, sizeof(central_server_addr)) == -1){
                printf("client could not connect to the server\n");
                exit(0);
                //ERROR
            }
            pthread_mutex_lock(&mux_camAttr);
            /*NOTE: actually, this image should be obtained directly from the 
                output stream of the camera. Nevertheless, for this practice it is assumed that
                the image is stored in a png file
            */

            FILE *img;
            //open the file with read-binary (rb) mode
            img = fopen(".//camIMG.png", "rb");
            /*Reading 1024 elements (byte arrays) of 1024 bytes each array.
            Thus the file size will be 1MB. (1024 = MAX_MS_SIZE)
            */
            fread(&(img_rq->img), MAX_MS_SIZE, MAX_MS_SIZE, img);
            
            fclose(img);
            //send the img request
            //send(int sd, void * buff, int len, int flags)
            send(client_sc, (void*)&img_rq->op, sizeof(char), 0);
            send(client_sc, (void*)&img_rq->ID, ID_LENGTH, 0);
            send(client_sc, (void *)&img_rq->img, MAX_MS_SIZE*MAX_MS_SIZE, 0);
            lastImgSent = time(NULL);

            close(client_sc);
        }
        pthread_mutex_unlock(&mux_camAttr);
        
        if(lastStatusUpdate + STATUS_UPDATE_FREQ <= time(NULL)){

            if ((client_sc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
                //Error
                printf("client socket could not be created\n");
                exit();
            }
            //open connection with server
            //int connect(int sd, struct sockaddr *addr_server, int addrlen)
            if (connect(client_sc, (struct sockaddr *) &central_server_addr, sizeof(central_server_addr)) == -1){
                printf("client could not connect to the server\n");
                exit(0);
                //ERROR
            }

            //send status request            
            send(client_sc, (void*)&status_rq->op, sizeof(char), 0);
            send(client_sc, (void*)&status_rq->ID, ID_LENGTH, 0);
            memcopy(&(status_rq->status), &status, MAX_MS_SIZE);
            send(client_sc, (void*)&status_rq->status, MAX_MS_SIZE, 0);
            lastStatusUpdate = time(NULL);

            close(client_sc);
        }        
    }
    return 0;

}