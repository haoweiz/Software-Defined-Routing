#include <string.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/update_manager.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/time_manager.h"

void update_response(int sock_index, char *cntrl_payload){
    uint16_t id;
    uint16_t cost;
    uint16_t offset = 2;
    int index = -1;
    memcpy(&id, cntrl_payload, sizeof(uint16_t));
    memcpy(&cost, cntrl_payload + offset, sizeof(uint16_t));
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        if(id == r->id){
            r->cost = cost;
            r->connect = TRUE; 
            index = r->index;
        }
    }
    struct time *t;
    LIST_FOREACH(t,&time_list,next){
        if(t->index == index){
            gettimeofday(&t->begin_send_time,NULL);
            gettimeofday(&t->begin_expire_time,NULL);
            t->send_time.tv_sec = updates_periodic_interval;
            t->send_time.tv_usec = 0;
            t->expire_time.tv_sec = 3 * updates_periodic_interval;
            t->expire_time.tv_usec = 0;
            t->isconnect = TRUE;
        }
    }
    printf("---------------------------update-----------------------------\n");
    printtimecon();
    printroutercon();
    printf("--------------------------------------------------------------\n");
    uint8_t control_code = 3;
    uint8_t response_code = 0;
    uint16_t payload_length = 0;
    char *update_response = (char*) malloc(CNTRL_RESP_HEADER_SIZE + payload_length);
    char *update_header = create_response_header(sock_index, control_code, response_code, payload_length);
    memcpy(update_response,update_header,CNTRL_RESP_HEADER_SIZE);
    sendALL(sock_index, update_response, CNTRL_RESP_HEADER_SIZE + payload_length);
    free(update_response);
}
