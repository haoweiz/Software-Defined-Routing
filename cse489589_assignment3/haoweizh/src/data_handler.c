#include <netinet/in.h>
#include <sys/socket.h>

#include "../include/global.h"
#include "../include/data_handler.h"

int new_data_conn(int data_socket){
    int newfd;
    struct sockaddr_in remoteaddr;
    int addrlen = sizeof(remoteaddr);
    if((newfd = accept(data_socket, (struct sockaddr*)&remoteaddr, (socklen_t*)&addrlen)) < 0){
        ERROR("Accept error!");
        exit(1);
    }
    return newfd;
}

