#include <string.h>
#include <sys/queue.h>

#include "../include/global.h"
#include "../include/sendfile_stats_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


void sendfile_stats_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint8_t transfer_id;
    uint16_t padding = 0;
    uint8_t control_code = 6;
    uint8_t response_code = 0;
    uint16_t payload_length;  
    memcpy(&transfer_id,cntrl_payload,sizeof(uint8_t));

    char *sendfile_stats_payload;
    struct sendfile_stats *ss;
    LIST_FOREACH(ss,&sendfile_stats_list,next){
        if(ss->transfer_id == transfer_id){
            payload_length = SENDFILE_STATS_HEAD + ss->number * sizeof(uint16_t);
            sendfile_stats_payload = (char*)malloc(payload_length);
            memset(sendfile_stats_payload,0,payload_length);
        }
    }

    LIST_FOREACH(ss,&sendfile_stats_list,next){
        if(ss->transfer_id == transfer_id){
            memcpy(sendfile_stats_payload,&ss->transfer_id,sizeof(uint8_t));
            memcpy(sendfile_stats_payload + 1,&ss->ttl,sizeof(uint8_t));
            memcpy(sendfile_stats_payload + 2,&padding,sizeof(uint16_t));
            for(int i = 1;i <= ss->number;++i){
                memcpy(sendfile_stats_payload + SENDFILE_STATS_HEAD + (i-1)*sizeof(uint16_t),&ss->seqnum[i-1],sizeof(uint16_t));
            }
        }
    }

    char *sendfile_stats_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_stats_response,sendfile_stats_header,CNTRL_RESP_HEADER_SIZE);
    memcpy(sendfile_stats_response + CNTRL_RESP_HEADER_SIZE,sendfile_stats_payload,payload_length);
    sendALL(sock_index, sendfile_stats_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(sendfile_stats_response);
    free(sendfile_stats_payload);
}
