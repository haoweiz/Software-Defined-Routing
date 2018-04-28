#include <sys/queue.h>
#include <sys/time.h>

#include "../include/global.h"
#include "../include/time_manager.h"

void init_time(){
    LIST_INIT(&time_list);
    int i = 1;
    while(i <= number_of_routers){
        struct time *t = (struct time*)malloc(sizeof(struct time));
        t->router_id = i;
        gettimeofday(&t->begin_time,NULL);
        t->left_time.tv_sec = updates_periodic_interval;
        t->miss = 0;
        LIST_INSERT_HEAD(&time_list, t, next);
    }
}

struct timeval getdifftime(struct timeval begin, struct timeval end){
    struct timeval diff;
    if(end.tv_usec >= begin.tv_usec){
        diff.tv_usec = end.tv_usec - begin.tv_usec;
        diff.tv_sec = end.tv_sec - begin.tv_sec;
    }
    else{
        diff.tv_usec = 1000000 + end.tv_usec - begin.tv_usec;
        diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
    }
    return diff;
}

void update_time(){
    struct time *t;
    struct timeval current;
    gettimeofday(&current,NULL);
    LIST_FOREACH(t,&time_list,next){
        if(current.tv_sec - t->begin_time.tv_sec >= updates_periodic_interval){
            t->begin_time.tv_sec = current.tv_sec;
            t->begin_time.tv_usec = current.tv_usec;
            t->left_time.tv_sec = updates_periodic_interval;
            t->left_time.tv_usec = 0;
            t->miss++;
        }
        else{
            struct timeval diff = getdifftime(t->begin_time,current);
            struct timeval left = getdifftime(diff,t->left_time);
            t->left_time.tv_sec = left.tv_sec;
            t->left_time.tv_usec = left.tv_usec;
        }
    }
}

struct timeval gettimeout(){
    struct time *t;
    struct timeval result;
    int isinitial = 0;
    LIST_FOREACH(t,&time_list,next){
        if(isinitial == 0){
            result.tv_sec = t->left_time.tv_sec;
            result.tv_usec = t->left_time.tv_usec;
        }
        else{
            struct timeval diff = getdifftime(result,t->left_time);
            if(diff.tv_sec < 0 || diff.tv_usec < 0){
                result.tv_sec = t->left_time.tv_sec;
                result.tv_usec = t->left_time.tv_usec;
            }
        }        

    }
    return result;
}
