#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/data_handler.h"


bool isData(int sock_index){
    struct DataConn *data;
    LIST_FOREACH(data,&data_list,next){
        if(data->sockfd == sock_index)
            return TRUE;
    }
    return FALSE;
}

bool data_recv_file(int sock_index){
    return TRUE;
}

int new_data_conn(int data_socket){
    int newfd;
    struct sockaddr_in remoteaddr;
    int addrlen = sizeof(remoteaddr);
    if((newfd = accept(data_socket, (struct sockaddr*)&remoteaddr, (socklen_t*)&addrlen)) < 0){
        ERROR("Accept error!");
    }
    struct DataConn *newconn = (struct DataConn*)malloc(sizeof(struct DataConn));
    newconn->sockfd = newfd;
    LIST_INSERT_HEAD(&data_list, newconn, next);
    return newfd;
}

