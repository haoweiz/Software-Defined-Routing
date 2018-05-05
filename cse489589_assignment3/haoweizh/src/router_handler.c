#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/router_handler.h"
#include "../include/time_manager.h"


uint16_t min(int a,int b){
    int result = a > b ? b : a;
    return result >= UINT16_MAX ? UINT16_MAX : result;
}


void recv_update(int router_socket){
    struct sockaddr_in from_addr;    
    int numbytes = 0;
    int length = UPDATE_HEADER + number_of_routers * UPDATE_ENTRY_LENGTH;
    uint16_t *buffer = (uint16_t*)malloc(length*sizeof(uint16_t));
    int addr_len = sizeof(struct sockaddr);
    if ((numbytes = recvfrom(router_socket,buffer,length*sizeof(uint16_t),0,(struct sockaddr *)&from_addr,&addr_len)) < 0){
        ERROR("Recvfrom error.");
        exit(1);
    }

    /* Parsing buffer. */  
    uint16_t number_of_update_fields;  //N
    uint16_t source_router_port;       //N
    uint32_t source_ip_addr;           //N
    uint16_t offset = 0;
    uint16_t source_id;                //H

    memcpy(&number_of_update_fields, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&source_router_port, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(&source_ip_addr, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint16_t num = ntohs(number_of_update_fields);
    uint32_t *receive_ip = (uint32_t*)malloc(num * sizeof(uint32_t));
    uint16_t *receive_port = (uint16_t*)malloc(num * sizeof(uint16_t));
    uint16_t *receive_id = (uint16_t*)malloc(num * sizeof(uint16_t));
    uint16_t *receive_cost = (uint16_t*)malloc(num * sizeof(uint16_t));

    for(int i = 0;i != num;++i){
        memcpy(&receive_ip[i],buffer + offset,sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(&receive_port[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
        offset += sizeof(uint16_t);    //padding
        memcpy(&receive_id[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
        memcpy(&receive_cost[i],buffer + offset,sizeof(uint16_t));
        offset += sizeof(uint16_t);
        if(source_ip_addr == receive_ip[i])
            source_id = ntohs(receive_id[i]);
    }
    printf("receive buffer from %d\n",source_id);
    
    /* Update receive distance vector. */
    for(int i = 0;i != num;++i){
        for(int j = 0;j != num;++j){
            if(ntohs(receive_id[j]) == i+1){
                distance_vector[source_id-1][i] = receive_cost[j];
                break;
            }
        }
    }

    /* Update my distance vector */
    for(int i = 0;i != num;++i){
        if(i+1 == this_router_id)
            continue;
        uint16_t newcost;
        struct router *r;
        LIST_FOREACH(r,&router_list,next){
            if(ntohs(r->id) == i+1)
                newcost = ntohs(r->cost);
        }
        for(int j = 0;j != num;++j){
            if(j+1 == this_router_id){
                continue;
            }
            uint16_t origcost = newcost;
            newcost = min((int)newcost,(int)distance_vector[this_router_id-1][j]+(int)distance_vector[j][i]);
            if(origcost != newcost){
                struct router *r;
                LIST_FOREACH(r,&router_list,next){
                    if(ntohs(r->id) == i+1)
                        r->next_hop = htons(j+1);
                }
            }
        }
        distance_vector[this_router_id-1][i] = newcost;
    }

    /* When receive router table, update expire time. */
    struct time *t;
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect == TRUE && t->router_id == source_id){
            printf("Update expire time of router %d\n",source_id);
            gettimeofday(&t->begin_expire_time,NULL);
            t->expire_time.tv_sec = 3 * updates_periodic_interval;
            t->expire_time.tv_usec = 0;
        }
    }

    printrt();
}

/* r is destinate router */
void send_vector(int router_socket, struct router *r){   
    struct sockaddr_in des_addr;
    des_addr.sin_family = AF_INET;
    des_addr.sin_port = r->router_port;
    des_addr.sin_addr.s_addr = r->ip;
    bzero(&(des_addr.sin_zero), 8);

    int numbytes = 0;
    uint16_t offset = 0;
    int length = UPDATE_HEADER + number_of_routers * UPDATE_ENTRY_LENGTH;
    uint16_t *buffer = (uint16_t*)malloc(length*sizeof(uint16_t));

    uint16_t number_of_update_fields = htons(number_of_routers);
    uint16_t source_router_port = this_router_port;
    uint32_t source_ip_addr = this_router_ip;
    uint16_t padding = 0;

    /* Write information about routers to be sent. */
    memcpy(buffer+offset,&number_of_update_fields,sizeof(uint16_t));
    offset += UPDATE_FIELD;
    memcpy(buffer+offset,&source_router_port,sizeof(uint16_t));
    offset += SOURCE_ROUTER_PORT;
    memcpy(buffer+offset,&source_ip_addr,sizeof(uint32_t));
    offset += SOURCE_IP_ADDR;

    struct router *myrouter;
    LIST_FOREACH(myrouter,&router_list,next){
        memcpy(buffer+offset,&myrouter->ip,sizeof(uint32_t));
        offset += sizeof(uint32_t);

        memcpy(buffer+offset,&myrouter->router_port,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        memcpy(buffer+offset,&padding,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        memcpy(buffer+offset,&myrouter->id,sizeof(uint16_t));
        offset += sizeof(uint16_t);

        uint16_t cost;
        for(int i = 0;i != number_of_routers;++i){
            if(ntohs(myrouter->id) == i+1)
                cost = distance_vector[this_router_id-1][i];
        }
        memcpy(buffer+offset,&cost,sizeof(uint16_t));
        offset += sizeof(uint16_t);
    }

    printf("send %d buffer\n",ntohs(r->id));
    if ((numbytes = sendto(router_socket,buffer,length*sizeof(uint16_t),0,(struct sockaddr *)&des_addr,sizeof(struct sockaddr_in))) < 0) {
        ERROR("Sendto error.");
    }
}
