#include <string.h>
#include <sys/queue.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/sendfile_stats_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


void sendfile_stats_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint8_t transfer_id;
    uint16_t padding = 0;
    uint8_t control_code = 6;
    uint8_t response_code = 0;
    uint16_t payload_length = last_seqnum - init_seqnum + 1 + SENDFILE_STATS_HEAD;  
    memcpy(&transfer_id,cntrl_payload,sizeof(uint8_t));
    
    char *sendfile_stats_payload;
    if(transfer_id == global_transfer_id){
        sendfile_stats_payload = (char*)malloc(payload_length);
        memset(sendfile_stats_payload,'\0',payload_length);
   
        uint16_t offset = 0;
        memcpy(sendfile_stats_payload + offset,&global_transfer_id,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);
        memcpy(sendfile_stats_payload + offset,&global_ttl,sizeof(uint8_t));
        offset = offset + sizeof(uint8_t);
        memcpy(sendfile_stats_payload + offset,&padding,sizeof(uint16_t));
        offset = offset + sizeof(uint16_t);

        uint16_t begin = init_seqnum;
        uint16_t end = last_seqnum;
        printf("ttl = %d\n",global_ttl); 
        printf("init_seqnum = %d\n",init_seqnum);
        printf("last_seqnum = %d\n",last_seqnum);
        while(begin <= end){
            uint16_t seqnum = htons(begin);
            memcpy(sendfile_stats_payload + offset,&seqnum,sizeof(uint16_t));
            begin++;
            offset = offset + sizeof(uint16_t);
        }

        char *sendfile_stats_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
        char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, payload_length);
        memcpy(sendfile_stats_response,sendfile_stats_header,CNTRL_RESP_HEADER_SIZE);
        memcpy(sendfile_stats_response + CNTRL_RESP_HEADER_SIZE,sendfile_stats_payload,payload_length);
        sendALL(sock_index, sendfile_stats_response, CNTRL_RESP_HEADER_SIZE + payload_length);
        free(sendfile_stats_response);
        free(sendfile_stats_payload);
    }
    else{
        char *sendfile_stats_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE);
        char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, 0);
        memcpy(sendfile_stats_response,sendfile_stats_header,CNTRL_RESP_HEADER_SIZE);
        sendALL(sock_index, sendfile_stats_response, CNTRL_RESP_HEADER_SIZE);
    }
}
