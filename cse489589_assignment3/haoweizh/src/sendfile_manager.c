#include <string.h>

#include "../include/global.h"
#include "../include/sendfile_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void sendfile_response(int sock_index, char *cntrl_payload, uint16_t payload_len){


    uint8_t control_code = 5;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *sendfile_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *sendfile_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(sendfile_response,sendfile_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, sendfile_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(sendfile_response);
}
