#include "read_line.h"
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

int main(){

    while(1){
        socket() // Create a new data socket to receive the information
        connect() // Try to establish connection with the server
        while(string = read_line() != "EXIT"){
            read_line()
            send_msg()
            recv_msg()
        }
        close() // close the data socket
        exit(0);
    }
}