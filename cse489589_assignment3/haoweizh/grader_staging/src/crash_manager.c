#include <string.h>

#include "../include/global.h"
#include "../include/crash_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

void crash_response(int sock_index){
    uint8_t control_code = 4;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *crash_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *crash_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(crash_response,crash_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, crash_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(crash_response);
    exit(0);
}