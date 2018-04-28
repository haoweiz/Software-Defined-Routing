#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


void connect_destinate(uint32_t des_ip){
    struct router *r;
    uint16_t next_hop;
    uint16_t des_port;
    LIST_FOREACH(r,&router_list,next){
        if(des_ip == r->ip){
            next_hop = r->next_hop;
        }
    }
    LIST_FOREACH(r,&router_list,next){
        if(r->id == next_hop){
            des_port = r->data_port;
        }
    }

    int connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in des_addr; 
    bzero(&des_addr,sizeof(des_addr));
    des_addr.sin_family = AF_INET;
    des_addr.sin_port = des_port;
    des_addr.sin_addr.s_addr = des_ip;
    if ((connect(connect_socket, (struct sockaddr *)&des_addr, sizeof(struct sockaddr))) < 0){
        close(connect_socket);
        ERROR("Connect error.");
    }
}


void sendfile_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint32_t des_ip;
    uint8_t ttl;
    uint8_t transfer_id;
    uint16_t seqnum;
    uint8_t offset = 0;
    char *file_name;

    memcpy(&des_ip, cntrl_payload + offset, sizeof(des_ip));
    offset += sizeof(des_ip);

    memcpy(&ttl, cntrl_payload + offset, sizeof(ttl));
    offset += sizeof(ttl);

    memcpy(&transfer_id, cntrl_payload + offset, sizeof(transfer_id));
    offset += sizeof(transfer_id);

    memcpy(&seqnum, cntrl_payload + offset, sizeof(seqnum));
    offset += sizeof(seqnum);

    memcpy(file_name, cntrl_payload + offset, payload_len - offset + 1);

    connect_destinate(des_ip);

    uint8_t control_code = 5;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;  //unfinished!
    char *sendfile_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *sendfile_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_response,sendfile_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, sendfile_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(sendfile_response);
}
