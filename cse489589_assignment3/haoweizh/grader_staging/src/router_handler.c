#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/router_handler.h"
#include "../include/time_manager.h"

#define MAXBUFLEN 1024

void recv_update(int router_socket){
    printf("recv_update.\n");
    struct sockaddr_in from_addr;    
    int numbytes = 0;
    char buffer[MAXBUFLEN];
    bzero(&buffer,sizeof(buffer));
    int addr_len = sizeof(struct sockaddr);
    if ((numbytes = recvfrom(router_socket,buffer,MAXBUFLEN-1,0,(struct sockaddr *)&from_addr,&addr_len)) < 0){
        ERROR("Recvfrom error.");
        exit(1);
    }
    /* Parsing buffer. */
    uint16_t *id = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *cost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t myid;
    id[0] = ntohs(atoi(strtok(buffer," ")));
    cost[0] = ntohs(atoi(strtok(NULL," ")));
    for(int i = 1;i != number_of_routers;++i){
        id[i] = ntohs(atoi(strtok(NULL," ")));
        cost[i] = ntohs(atoi(strtok(NULL," ")));
        if(cost[i] == 0)
            myid = id[i];
    }
    struct time *t;
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect && t->router_id == myid){
            gettimeofday(&t->begin_time,NULL);
            t->left_time.tv_sec = updates_periodic_interval;
            t->left_time.tv_usec = 0;
            t->miss = 0;
        }
    }
    timeout = gettimeout();
    
    uint16_t *old_id = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *old_cost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    struct router *r;
    int b = 0;
    int index = 0;
    LIST_FOREACH(r,&router_list,next){
        old_id[b] = ntohs(r->id);
        old_cost[b] = ntohs(r->cost);
        if(ntohs(r->next_hop) == myid) index = b;
        b++;
    }
    for(int i = 0;i != number_of_routers;++i){
        int k;
        for(int j = 0;j != number_of_routers;++j){
            if(old_id[j] == id[i]) k = j;
        }
        if(old_cost[index] + cost[i] < old_cost[k]){
            old_cost[k] = old_cost[index] + cost[i];
            struct router *temp;
            LIST_FOREACH(temp,&router_list,next){
                if(temp->id == htons(old_id[k])){
                    temp->next_hop = htons(index);
                    temp->cost = old_cost[k];
                }
            }
        }
    }
}

void send_vector(int router_socket, struct router *r){
    struct sockaddr_in des_addr;
    des_addr.sin_family = AF_INET;
    des_addr.sin_port = r->router_port;
    des_addr.sin_addr.s_addr = r->ip;
    bzero(&(des_addr.sin_zero), 8);

    int numbytes = 0;
    int length = sizeof(MAXBUFLEN);
    char buffer[MAXBUFLEN];
    bzero(&buffer,sizeof(buffer));

    /* Write information about routers to be sent. */
    LIST_FOREACH(r,&router_list,next){
        char id[16];
        bzero(&id,sizeof(id));
        sprintf(id,"%d",r->id);
        char cost[16];
        bzero(&cost,sizeof(cost));
        sprintf(cost,"%d",r->cost);
        strcat(buffer,id);
        strcat(buffer," ");
        strcat(buffer,cost);
        strcat(buffer," ");
    }

    if ((numbytes = sendto(router_socket,buffer,length,0,(struct sockaddr *)&des_addr,sizeof(struct sockaddr_in))) < 0) {
        ERROR("Sendto error.");
    }
}
