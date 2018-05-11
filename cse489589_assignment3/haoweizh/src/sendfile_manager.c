#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


/* After receiving sendfile_response, connect to destination. */
int connect_destinate(uint32_t des_ip){
    struct router *r;
    uint16_t des_port;
    LIST_FOREACH(r,&router_list,next){
        if(r->ip == des_ip){
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
    return connect_socket;
}

char *create_packet_header(uint32_t des_ip,uint8_t transfer_id,uint8_t ttl,uint16_t seqnum,uint8_t fin){
    char *buffer = (char*)malloc(DATA_PACKET_HEADER_OFFSET);
    uint16_t offset = 0;
    memcpy(buffer+offset,&des_ip,sizeof(uint32_t));
    offset = offset + sizeof(uint32_t);
    memcpy(buffer+offset,&transfer_id,sizeof(uint8_t));
    offset = offset + sizeof(uint8_t);
    memcpy(buffer+offset,&ttl,sizeof(uint8_t));
    offset = offset + sizeof(uint8_t);
    memcpy(buffer+offset,&seqnum,sizeof(uint16_t));
    offset = offset + sizeof(uint16_t);
    memcpy(buffer+offset,&fin,sizeof(uint8_t));
    offset = offset + sizeof(uint8_t);
    uint8_t padding8 = 0;
    uint16_t padding16 = 0;
    memcpy(buffer+offset,&padding8,sizeof(uint8_t));
    memcpy(buffer+offset,&padding16,sizeof(uint16_t));
    return buffer;

}


void sendfile_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint32_t des_ip;
    uint8_t ttl;
    uint8_t transfer_id;
    uint16_t seqnum;
    uint8_t offset = 0;
    char file_name[16];
    memset(file_name,'\0',16);

    memcpy(&des_ip, cntrl_payload + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(&ttl, cntrl_payload + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&transfer_id, cntrl_payload + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(&seqnum, cntrl_payload + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(file_name, cntrl_payload + offset, payload_len-8);

    struct sendfile_stats *ss = (struct sendfile_stats*)malloc(sizeof(struct sendfile_stats));
    ss->transfer_id = transfer_id;
    ss->ttl = ttl;
    ss->number = 0;

    /* Find next hop ip address */
    uint16_t next_hop;
    uint32_t next_hop_ip;
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(r->ip == des_ip){
            next_hop = r->next_hop;
        }
    }
    LIST_FOREACH(r,&router_list,next){
        if(r->id == next_hop){
            next_hop_ip = r->ip;
        }
    }
    int connect_socket = connect_destinate(next_hop_ip);

    /* Send file here */
    FILE *stream = fopen(file_name,"r");
    fseek(stream, 0L, SEEK_END);
    int sz = ftell(stream);
    int count = 0;
    fseek(stream, 0L, SEEK_SET);
    char *total_payload = (char*)malloc(sz);
    memset(total_payload,0,sz);
    fread(total_payload,sz,1,stream);
    fclose(stream);
    char *header_one = create_packet_header(des_ip,transfer_id,ttl,seqnum,FIN_ONE);
    char *header_zero = create_packet_header(des_ip,transfer_id,ttl,seqnum,FIN_ZERO);

    while(count * PAYLOAD_LEN < sz){
        char *header;
        if((count+1) * PAYLOAD_LEN >= sz){
            header = header_one;
        }
        else{
            header = header_zero;
        }

        sendALL(connect_socket,header,DATA_PACKET_HEADER_OFFSET);
        sendALL(connect_socket,total_payload + count*PAYLOAD_LEN,PAYLOAD_LEN);

        ss->seqnum[ss->number] = seqnum;
        ss->number++;
        count++;
        seqnum = htons(ntohs(seqnum)+1);
    }
    LIST_INSERT_HEAD(&sendfile_stats_list, ss, next);
    close(connect_socket);

    /* Send control response */
    uint8_t control_code = 5;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;  
    char *sendfile_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *sendfile_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_response,sendfile_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, sendfile_response, CNTRL_RESP_HEADER_SIZE + payload_length);

    free(ss);
    free(total_payload);
    free(sendfile_response);
}
