#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <string.h>

#include "../include/data_manager.h"
#include "../include/global.h"

int create_data_sock(){
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(data_sock < 0)
        ERROR("socket() failed");

    struct sockaddr_in data_addr;
    socklen_t addrlen = sizeof(data_addr);

    /* Make socket re-usable */
    if(setsockopt(data_sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    bzero(&data_addr, sizeof(data_addr));

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(htons(r->cost) == 0)
            data_addr.sin_port = htons(r->data_port);
    }

    if(bind(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
        ERROR("bind() failed");

    if(listen(data_sock, 5) < 0)
        ERROR("listen() failed");

    return data_sock;
}
