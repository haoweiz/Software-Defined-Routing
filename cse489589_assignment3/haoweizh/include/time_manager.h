#ifndef _TIME_MANAGER_H_
#define _TIME_MANAGER_H_

#define EXPIRE 1
#define SEND 2

struct time{
    bool isconnect;
    uint16_t router_id;
    struct timeval begin_expire_time;
    struct timeval begin_send_time;
    struct timeval expire_time;
    struct timeval send_time;
    LIST_ENTRY(time) next;
};

LIST_HEAD(time_head,time) time_list;

void init_time();
struct timeval getdifftime(struct timeval begin, struct timeval end);
struct timeval gettotaltime(struct timeval base,struct timeval interval);
void update_time();
struct timeval gettimeout();

#endif
