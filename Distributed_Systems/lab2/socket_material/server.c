#include "read_line.h"
#include <unistd.h>
#include <errno.h>

int main(){
    socket()// Open connection socket
    bind()
    listen()
    accept(); //Client wants to establish a connection
    create new data socket
    while(connection not closed){
        read()  // request
            if(req == EXIT)
                close() // the data socket, NOT THE LISTENING PORT of the server
                break;
            else
                print req.string
        write() // reply
    }
    close();// Close connection socket
    exit(0);
}