#include <stdio.h>
#include <netinet/in.h>

#include "../include/global.h"

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
