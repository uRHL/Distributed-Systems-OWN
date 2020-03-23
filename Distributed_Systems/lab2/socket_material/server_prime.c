#include "read_line.h"
#include <unistd.h>
#include <errno.h>

int main(){
    open receiving sockect
    bind

    listen
    accept

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while(1){
        sd = accept(s, (struct sockaddr *) &client, &len);
        pthread_create(&thid, &attr, manage_request, &sd);

        /* wait for the child to copy the descriptor */
        pthread_mutex_lock(&mtx);
        while(busy == TRUE)
            pthread_cond_wait(&mtx, &cond);
        pthread_mutex_unlock(&mtx);
        busy = TRUE

    }

}

int manage_request(int * s){
    int s_local;
    pthread_mutex_lock(&mtx);
    s_local = *s;
    open new socket
    
    busy = FALSE;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    read
    write
    close new_socket

    /* manage_request using descriptor s_local */
    close(s_local);
    pthread_exit(NULL);


}