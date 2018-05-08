#include <string.h>

#include "../include/global.h"
#include "../include/sendfile_stats_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"


void sendfile_stats_response(int sock_index, char *cntrl_payload, uint16_t payload_len){
    uint8_t transfer_id;
    memcpy(&transfer_id,cntrl_payload,sizeof(uint8_t));


    uint8_t control_code = 6;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;  
    char *sendfile_stats_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *sendfile_stats_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_stats_response,sendfile_stats_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, sendfile_stats_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(sendfile_stats_response);
}
