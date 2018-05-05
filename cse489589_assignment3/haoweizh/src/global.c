#include <stdio.h>

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
