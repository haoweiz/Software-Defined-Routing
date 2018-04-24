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

struct timeval timeout;
uint16_t number_of_routers;
uint16_t updates_periodic_interval;
uint16_t CONTROL_PORT;
extern int head_fd;
extern fd_set master_list;


/* Store as network byte order. */
struct router{
    uint16_t id;
    uint16_t router_port;
    uint16_t data_port;
    uint16_t cost;
    uint32_t ip;
    uint16_t next_hop;
    LIST_ENTRY(router) next;
};

LIST_HEAD(router_head,router) router_list;

#endif
