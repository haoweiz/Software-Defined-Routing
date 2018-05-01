#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_

#define TIMEOUT 0
#define RECEIVEVECTOR 1
struct time{
    int isconnect;
    uint16_t router_id;
    uint32_t miss;
    struct timeval begin_time;
    struct timeval left_time;
    LIST_ENTRY(time) next;
};

LIST_HEAD(time_head,time) time_list;

void init_time();
struct timeval getdifftime(struct timeval begin, struct timeval end);
void update_time(int cause);
struct timeval gettimeout();

#endif
