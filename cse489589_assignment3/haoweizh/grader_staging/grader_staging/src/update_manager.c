#include <string.h>

#include "../include/global.h"
#include "../include/update_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

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
    char *update_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *update_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(update_response,update_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, update_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(update_response);
}
