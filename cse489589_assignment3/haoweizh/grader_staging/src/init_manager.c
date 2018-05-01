#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/select.h>

#include "../include/init_manager.h"
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/connection_manager.h"
#include "../include/network_util.h"
#include "../include/time_manager.h"

int create_router_sock(){
    int router_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(router_sock < 0)
        ERROR("router_socket() failed");

    struct sockaddr_in router_addr;
    socklen_t addrlen = sizeof(router_addr);
    bzero(&router_addr, sizeof(router_addr));
    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(htons(r->cost) == 0)
            router_addr.sin_port = htons(r->router_port);
    }

    int yes = 1;
    if(setsockopt(router_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        ERROR("setsockopt() failed");

    if(bind(router_sock, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0)
        ERROR("bind() failed");
    return router_sock;
}

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

    if(listen(data_sock, number_of_routers) < 0)
        ERROR("listen() failed");

    LIST_INIT(&data_connection_list);
    return data_sock;
}

/* Not finished! */
void init_next_hop(){
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(ntohs(r->cost) == UINT16_MAX)
            r->next_hop = htons(UINT16_MAX);
        else{
            r->next_hop = r->id;
        }
    }
}



void init_router_list(char *cntrl_payload, uint16_t offset, uint16_t payload_len){
    LIST_INIT(&router_list);
    while(offset < payload_len){
        struct router *r = (struct router*)malloc(sizeof(struct router));

        memcpy(&r->id, cntrl_payload + offset, sizeof(r->id));
        offset += sizeof(r->id);

        memcpy(&r->router_port, cntrl_payload + offset, sizeof(r->router_port));
        offset += sizeof(r->router_port);

        memcpy(&r->data_port, cntrl_payload + offset, sizeof(r->data_port));
        offset += sizeof(r->data_port);

        memcpy(&r->cost, cntrl_payload + offset, sizeof(r->cost));
        offset += sizeof(r->cost);

        memcpy(&r->ip, cntrl_payload + offset, sizeof(r->ip));
        offset += sizeof(r->ip);

        LIST_INSERT_HEAD(&router_list, r, next);
    }
    init_next_hop();
}



void init_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    isinit = TRUE;
    uint16_t offset;
    memcpy(&number_of_routers, cntrl_payload+offset, sizeof(number_of_routers));
    number_of_routers = ntohs(number_of_routers);
    offset += sizeof(number_of_routers);
    memcpy(&updates_periodic_interval, cntrl_payload+offset, sizeof(updates_periodic_interval));
    offset += sizeof(updates_periodic_interval);
    updates_periodic_interval = ntohs(updates_periodic_interval);
    timeout.tv_sec = updates_periodic_interval;

    init_router_list(cntrl_payload, offset, payload_len);
    init_time();
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(htons(r->cost) == 0)
            myid = r->id;
    }

    router_socket = create_router_sock();
    data_socket = create_data_sock();
    FD_SET(router_socket, &master_list);
    FD_SET(data_socket, &master_list);
    int hf = router_socket > data_socket ? router_socket : data_socket;
    head_fd = head_fd > hf ? head_fd : hf;

    uint8_t control_code = 1;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *cntrl_header = create_response_header(sock_index, control_code, response_code, payload_length);
    char *init_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    memcpy(init_response,cntrl_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, init_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(init_response);
}



