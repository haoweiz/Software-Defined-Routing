#include <stdio.h>
#include <netinet/in.h>
#include <sys/queue.h>

#include "../include/global.h"
#include "../include/time_manager.h"

void printrt(){
    printf("------------routing table------------\n");
    for(int i = 0;i != number_of_routers;++i){
        for(int j = 0;j != number_of_routers;++j){
            printf("%d|",distance_vector[i][j]);
        }
        printf("\n");
    }
    printf("-------------------------------------\n");
}

void printnh(){
    printf("------------next hop------------------\n");
    struct router *r;
    LIST_FOREACH(r,&router_list,next){
        printf("%d = %d\n",ntohs(r->id),ntohs(r->next_hop));
    }
    printf("---------------------------------------\n");
}

void printtimecon(){
    struct time *t;
    printf("------------time connection-------------\n");
    LIST_FOREACH(t,&time_list,next){
        printf("%d is connected?:%d\n",t->index,t->isconnect);
    }
    printf("----------------------------------------\n");
}

void printroutercon(){
    struct router *r;
    printf("------------router connection-------------\n");
    LIST_FOREACH(r,&router_list,next){
        printf("%d is connected?:%d\n",ntohs(r->id),r->connect);
    }
    printf("----------------------------------------\n");
}
