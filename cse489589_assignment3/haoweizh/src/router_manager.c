#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <string.h>

#include "../include/router_manager.h"
#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

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


    if(bind(router_sock, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0)
        ERROR("bind() failed");
    return router_sock;
}

void routing_table_response(int sock_index){
    uint8_t control_code = 2;
    uint8_t response_code = 0;
    uint16_t payload_length = 4 * sizeof(uint16_t) * number_of_routers;
    char *routing_table_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *cntrl_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(routing_table_response,cntrl_header,CNTRL_RESP_HEADER_SIZE);
    struct router *r;
    uint16_t offset = 0;
    uint16_t padding = 0;
    LIST_FOREACH(r,&router_list,next){
        memcpy(routing_table_response + CNTRL_RESP_HEADER_SIZE + offset, &r->id, sizeof(uint16_t));
        offset = offset + 2;
        memcpy(routing_table_response + CNTRL_RESP_HEADER_SIZE + offset, &padding, sizeof(uint16_t));
        offset = offset + 2;
        memcpy(routing_table_response + CNTRL_RESP_HEADER_SIZE + offset, &r->next_hop, sizeof(uint16_t));
        offset = offset + 2;
        memcpy(routing_table_response + CNTRL_RESP_HEADER_SIZE + offset, &r->cost, sizeof(uint16_t));
        offset = offset + 2;
    }
    sendALL(sock_index, routing_table_response, CNTRL_RESP_HEADER_SIZE + payload_length);
}

void update_response(int sock_index, char *cntrl_payload){
    uint16_t id;
    uint16_t cost;
    uint16_t offset = 2;
    memcpy(&id, cntrl_payload, sizeof(uint16_t));
    memcpy(&cost, cntrl_payload + offset, sizeof(uint16_t));
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(r->id == id)
            r->cost = cost;
    }

    uint8_t control_code = 3;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *update_response = create_response_header(sock_index, control_code, response_code, payload_length);
    sendALL(sock_index, update_response, CNTRL_RESP_HEADER_SIZE + payload_length);
}

void crash_response(int sock_index){
    uint8_t control_code = 4;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *crash_response = create_response_header(sock_index, control_code, response_code, payload_length);
    sendALL(sock_index, crash_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    exit(0);
}
