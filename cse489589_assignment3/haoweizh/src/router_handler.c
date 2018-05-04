#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/router_handler.h"
#include "../include/time_manager.h"

#define MAXBUFLEN 1024


int min(int a,int b){
    int result = a > b ? b : a;
    return result >= UINT16_MAX ? UINT16_MAX : result;
}


void recv_update(int router_socket){
    struct sockaddr_in from_addr;    
    int numbytes = 0;
    char buffer[MAXBUFLEN];
    bzero(&buffer,sizeof(buffer));
    int addr_len = sizeof(struct sockaddr);
    if ((numbytes = recvfrom(router_socket,buffer,MAXBUFLEN,0,(struct sockaddr *)&from_addr,&addr_len)) < 0){
        ERROR("Recvfrom error.");
        exit(1);
    }
    printf("receive buffer = %s",buffer);

    /* Parsing buffer. */  
    uint16_t *receive_id = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *receive_cost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t receive_router_id;
    receive_id[0] = ntohs(atoi(strtok(buffer," ")));
    receive_cost[0] = ntohs(atoi(strtok(NULL," ")));
    for(int i = 1;i != number_of_routers;++i){
        receive_id[i] = ntohs(atoi(strtok(NULL," ")));
        receive_cost[i] = ntohs(atoi(strtok(NULL," ")));
        if(receive_cost[i] == 0)
            receive_router_id = receive_id[i];
    }
    printf("from %d\n",receive_router_id);

    struct time *t;
    LIST_FOREACH(t,&time_list,next){
        if(t->isconnect == TRUE && t->router_id == receive_router_id){
            printf("Update expire time of router %d\n",receive_router_id);
            gettimeofday(&t->begin_expire_time,NULL);
            t->expire_time.tv_sec = 3 * updates_periodic_interval;
            t->expire_time.tv_usec = 0;
        }
    }
    
    /* Distance vector update. */
    int index = 0;
    int myindex;
    uint16_t *mycost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *my_next_hop = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    struct router *myrouter;
    LIST_FOREACH(myrouter,&router_list,next){
        mycost[index] = ntohs(myrouter->cost);
        my_next_hop[index] = ntohs(myrouter->next_hop);
        if(ntohs(myrouter->id) == receive_router_id)
            myindex = index;
        index++;
    }

    uint16_t *new_cost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *new_next_hop = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    for(int i = 0;i != number_of_routers;++i){
        new_cost[i] = (uint16_t)min((int)mycost[i],(int)mycost[myindex] + (int)receive_cost[i]);
        if(new_cost[i] != mycost[i])
            new_next_hop[i] = receive_router_id;
        else
            new_next_hop[i] = my_next_hop[i];
    }

    int indexofcost = 0;
    struct router *updatecost;
    LIST_FOREACH(updatecost,&router_list,next){
        updatecost->cost = htons(new_cost[indexofcost]);
        updatecost->next_hop = htons(new_next_hop[indexofcost]);
        indexofcost++;
    }
    
    /*uint16_t *old_id = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    uint16_t *old_cost = (uint16_t*)malloc(number_of_routers * sizeof(uint16_t));
    struct router *r;
    int b = 0;
    int s = 0;
    int *index = (int*)malloc(number_of_routers * sizeof(int));
    for(int i = 0;i != number_of_routers;++i)
        index[i] = -1;
    LIST_FOREACH(r,&router_list,next){
        old_id[b] = ntohs(r->id);
        old_cost[b] = ntohs(r->cost);
        if(ntohs(r->next_hop) == receive_router_id){
            index[s] = b;
            s++;
        }
        b++;
    }*/
    /*int g = 0;
    while(index[g] != -1){
        for(int i = 0;i != number_of_routers;++i){
            struct time *tim;
            LIST_FOREACH(tim,&time_list,next){
                if(tim->router_id == id[i] && tim->isconnect == FALSE)
                    continue;
            }
            if(old_cost[index[g]] + cost[i] < old_cost[i]){
                old_cost[i] = old_cost[index[g]] + cost[i];
                struct router *temp;
                LIST_FOREACH(temp,&router_list,next){
                    if(ntohs(temp->id) == old_id[i]){
                        temp->next_hop = htons(myid);
                        temp->cost = htons(old_cost[i]);
                    }
                }
            }
        }
        g++;
    }*/
    printf("-----------routing table------------\n");
    struct router *temp;
    LIST_FOREACH(temp,&router_list,next){
        printf("%d = %d|",ntohs(temp->id),ntohs(temp->cost));
    }
    printf("\n------------------------------------\n");
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

    int this_router_id = ntohs(r->id);
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
    printf("send %d buffer = %s\n",this_router_id,buffer);
    if ((numbytes = sendto(router_socket,buffer,strlen(buffer),0,(struct sockaddr *)&des_addr,sizeof(struct sockaddr_in))) < 0) {
        ERROR("Sendto error.");
    }
}
