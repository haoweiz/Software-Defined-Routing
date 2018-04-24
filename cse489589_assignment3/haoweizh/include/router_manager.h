#ifndef _ROUTER_MANAGER_H_
#define _ROUTER_MANAGER_H_

int create_router_sock();
void routing_table_response(int sock_index);
void update_response(int sock_index, char *cntrl_payload);
void crash_response(int sock_index);

#endif
