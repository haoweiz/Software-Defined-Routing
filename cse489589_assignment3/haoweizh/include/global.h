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
extern struct timeval timeout;
extern int head_fd;
extern fd_set master_list;
extern fd_set watch_list;
extern bool isinit;                          //H
extern uint16_t **distance_vector;           //H

/* Store as network byte order. */
struct router{
    bool connect;             //H
    uint16_t id;              //N
    uint16_t router_port;     //N
    uint16_t data_port;       //N
    uint32_t ip;              //N
    uint32_t cost;            //N
    uint16_t next_hop;        //N
    LIST_ENTRY(router) next;
};


struct data_connection{
    uint32_t fd;
    LIST_ENTRY(data_connection) next;
};

LIST_HEAD(data_connection_head,data_connection) data_connection_list;
LIST_HEAD(router_head,router) router_list;

void printrt();

#endif
