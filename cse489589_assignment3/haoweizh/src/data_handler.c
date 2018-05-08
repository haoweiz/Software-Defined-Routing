#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>

#include "../include/global.h"
#include "../include/data_handler.h"
#include "../include/sendfile_manager.h"
#include "../include/network_util.h"


bool isData(int sock_index){
    struct DataConn *data;
    LIST_FOREACH(data,&data_list,next){
        if(data->sockfd == sock_index)
            return TRUE;
    }
    return FALSE;
}

bool data_recv_file(int sock_index){
    char *receive_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    if(recvALL(sock_index,receive_packet,PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET) < 0){
        free(receive_packet);
        return FALSE;
    }
    
    uint32_t des_ip;
    uint8_t ttl;
    uint32_t fin;
    memcpy(&des_ip,receive_packet,sizeof(uint32_t));
    memcpy(&ttl,receive_packet + TTL_OFFSET,sizeof(uint8_t));
    memcpy(&fin,receive_packet + FIN_OFFSET,sizeof(uint32_t));

    memcpy(panultimate_data_packet,last_data_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);
    memcpy(last_data_packet,receive_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);

    if(this_router_ip == des_ip){
        /* Receive this packet */
        if(fin == FIN_ONE){
        }
        else{
        
        }
    }
    else{
        /* Transfer to other routers */
        uint16_t next_hop;
        uint32_t next_hop_ip;
        struct router *r;
        LIST_FOREACH(r,&router_list,next){
            if(r->ip == des_ip)
                next_hop = r->next_hop;
        }
        LIST_FOREACH(r,&router_list,next){
            if(r->id == next_hop){
                next_hop_ip = r->ip;
            }
        }
        ttl--;
        memcpy(receive_packet + TTL_OFFSET,&ttl,sizeof(uint8_t));
        sendALL(sock_index,receive_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);
    }
    free(receive_packet);
    return TRUE;
}

/* When another router receives sendfile response command, accept that connection. */
int new_data_conn(int data_socket){
    int newfd;
    struct sockaddr_in remoteaddr;
    int addrlen = sizeof(remoteaddr);
    if((newfd = accept(data_socket, (struct sockaddr*)&remoteaddr, (socklen_t*)&addrlen)) < 0){
        ERROR("Accept error!");
    }
    struct DataConn *newconn = (struct DataConn*)malloc(sizeof(struct DataConn));
    newconn->sockfd = newfd;
    LIST_INSERT_HEAD(&data_list, newconn, next);
    return newfd;
}

