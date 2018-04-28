#ifndef _ROUTING_HANDLER_H_
#define _ROUTING_HANDLER_H_

void recv_update(int router_socket);
void send_vector(int router_socket, struct router *r);

#endif
