#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/select.h>

#include "../include/init_manager.h"
#include "../include/router_manager.h"
#include "../include/data_manager.h"
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/connection_manager.h"
#include "../include/network_util.h"

/* Not finished! */
void init_next_hop(){
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(ntohs(r->cost) == UINT16_MAX)
            r->next_hop = htons(UINT16_MAX);
        else if(ntohs(r->cost) == 0)
            r->next_hop = htons(0);
        else{
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
    uint16_t updates_periodic_interval;
    uint16_t offset;
    memcpy(&number_of_routers, cntrl_payload+offset, sizeof(number_of_routers));
    offset += sizeof(number_of_routers);
    memcpy(&updates_periodic_interval, cntrl_payload+offset, sizeof(updates_periodic_interval));
    offset += sizeof(updates_periodic_interval);

    init_router_list(cntrl_payload, offset, payload_len);

    router_socket = create_router_sock();
    data_socket = create_data_sock();
    FD_SET(router_socket, &master_list);
    FD_SET(data_socket, &master_list);
    head_fd = router_socket > data_socket ? router_socket : data_socket;

    timeout.tv_sec = updates_periodic_interval;
    timeout.tv_usec = 0;

    uint8_t control_code = 1;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *cntrl_header = create_response_header(sock_index, control_code, response_code, payload_length);
    char *init_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    memcpy(init_response,cntrl_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, init_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(init_response);
}


