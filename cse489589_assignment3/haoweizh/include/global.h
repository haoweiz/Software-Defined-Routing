#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/queue.h>
#include <sys/select.h>

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works


uint16_t updates_periodic_interval;           //H
uint16_t number_of_routers;                   //H
uint16_t CONTROL_PORT;                        //H
uint16_t this_router_id;                      //H
uint16_t this_data_port;                      //N
uint16_t this_router_port;                    //N
uint32_t this_router_ip;                      //N
uint16_t **distance_vector;                   //H
extern struct timeval timeout;
extern int head_fd;
extern fd_set master_list;
extern fd_set watch_list;
extern char *last_data_packet;
extern char *panultimate_data_packet;


/* Store as network byte order. */
struct router{
    int index;                //H
    bool connect;             //H
    uint16_t id;              //N
    uint16_t router_port;     //N
    uint16_t data_port;       //N
    uint32_t ip;              //N
    uint16_t cost;            //N
    uint16_t next_hop;        //N
    LIST_ENTRY(router) next;
};

struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
};


uint8_t global_transfer_id;       //H and N       
uint8_t global_ttl;               //H and N           
uint16_t init_seqnum;             //H
uint16_t last_seqnum;             //H
char *buffer;
bool send_finish;

LIST_HEAD(DataConnHead, DataConn) data_list;
LIST_HEAD(router_head, router) router_list;

void printrt();
void printnh();
void printtimecon();
void printroutercon();

#endif
