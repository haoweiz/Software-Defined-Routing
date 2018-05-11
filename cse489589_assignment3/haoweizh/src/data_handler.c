#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <unistd.h>
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

bool hastid(uint8_t transfer_id){
    bool hastid = FALSE;
    struct sendfile_stats *ss;
    LIST_FOREACH(ss,&sendfile_stats_list,next){
        if(ss->transfer_id == transfer_id)
            hastid = TRUE;
    }
    return hastid;
}


void update_sendfile_stats_list(uint8_t transfer_id,uint16_t sequencenum,uint32_t next_hop_ip,uint8_t ttl){
    if(hastid(transfer_id) == FALSE){
        /* First time to receive packet. */
        struct sendfile_stats *newss = (struct sendfile_stats*)malloc(sizeof(struct sendfile_stats));
        newss->transfer_id = transfer_id;
        newss->ttl = ttl-1;
        newss->number = 1;
        newss->seqnum[0] = sequencenum;
        newss->buffer = (char*)malloc(PAYLOAD_LEN*PAYLOAD_LEN*10);
        memset(newss->buffer,0,PAYLOAD_LEN*PAYLOAD_LEN*10);
        LIST_INSERT_HEAD(&sendfile_stats_list, newss, next);

        if(next_hop_ip != this_router_ip)
            connect_socket = connect_destinate(next_hop_ip);
    }
    else{
        /* Already receive some packet from transfer id. */
        struct sendfile_stats *oldss;
        LIST_FOREACH(oldss,&sendfile_stats_list,next){
            if(oldss->transfer_id == transfer_id){
                oldss->seqnum[oldss->number] = sequencenum;
                oldss->number++;
            }
        }
    }
}

bool candrop(uint8_t ttl){
    if(ttl == 1)
        return TRUE;
    return FALSE;
}

uint32_t get_next_hop_ip(uint32_t des_ip){
    uint16_t next_hop_id;
    uint32_t next_hop_ip;
    struct router *r;
    LIST_FOREACH(r,&router_list,next)
        if(r->ip == des_ip)
            next_hop_id = r->next_hop;
    LIST_FOREACH(r,&router_list,next)
        if(r->id == next_hop_id)
            next_hop_ip = r->ip;
    return next_hop_ip;
}


bool data_recv_file(int sock_index){
    char *receive_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    memset(receive_packet,0,PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    if(recvALL(sock_index,receive_packet,PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET) < 0){
        free(receive_packet);
        return FALSE;
    }
    
    char *receive_payload = (char*)malloc(PAYLOAD_LEN);
    memset(receive_payload,0,PAYLOAD_LEN);
    memcpy(receive_payload,receive_packet + DATA_PACKET_HEADER_OFFSET,PAYLOAD_LEN);

    uint32_t des_ip;
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t seqencenum;
    uint8_t fin;
    memcpy(&des_ip,receive_packet,sizeof(uint32_t));
    memcpy(&transfer_id,receive_packet + TRANSFER_ID_OFFSET,sizeof(uint8_t));
    memcpy(&ttl,receive_packet + TTL_OFFSET,sizeof(uint8_t));
    memcpy(&seqencenum,receive_packet + SEQ_OFFSET,sizeof(uint16_t));
    memcpy(&fin,receive_packet + FIN_OFFSET,sizeof(uint8_t));

    memcpy(panultimate_data_packet,last_data_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);
    memcpy(last_data_packet,receive_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);

    if(candrop(transfer_id) == TRUE)
        return FALSE;

    uint32_t next_hop_ip = get_next_hop_ip(des_ip);
    update_sendfile_stats_list(transfer_id,seqencenum,next_hop_ip,ttl);

    if(this_router_ip == next_hop_ip){
        /* Receive this packet */

        if(fin == FIN_ONE){
            char *filename = (char*)malloc(MAX_FILENAME_LEN);
            memset(filename,0,MAX_FILENAME_LEN);
            sprintf(filename,"file-%d",transfer_id);
            FILE *stream = fopen(filename,"w");

            struct sendfile_stats *ss;
            LIST_FOREACH(ss,&sendfile_stats_list,next){
                if(ss->transfer_id == transfer_id){
                    memcpy(ss->buffer+(ss->number-1)*PAYLOAD_LEN,receive_payload,PAYLOAD_LEN);
                    fwrite(ss->buffer,(ss->number)*PAYLOAD_LEN,1,stream);
                    free(ss->buffer);
                    LIST_REMOVE(ss,next);
                }
            }
            FD_CLR(sock_index, &master_list);
            fclose(stream);
            free(filename);
        }
        else{
            struct sendfile_stats *ss;
            LIST_FOREACH(ss,&sendfile_stats_list,next){
                if(ss->transfer_id == transfer_id){
                    memcpy(ss->buffer+(ss->number-1)*PAYLOAD_LEN,receive_payload,PAYLOAD_LEN);
                }
            }
        }
    }
    else{
        /* Transfer to other routers */
        memcpy(receive_packet + TTL_OFFSET,&ttl,sizeof(uint8_t));
        sendALL(connect_socket,receive_packet,DATA_PACKET_HEADER_OFFSET + PAYLOAD_LEN);
        if(fin == FIN_ONE){
            close(connect_socket);
            FD_CLR(sock_index,&master_list);
        }
    }
    free(receive_packet);
    free(receive_payload);
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

