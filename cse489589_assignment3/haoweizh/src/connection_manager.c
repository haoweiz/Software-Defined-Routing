/**
 * @connection_manager
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>
#include <netinet/in.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/router_handler.h"
#include "../include/data_handler.h"
#include "../include/time_manager.h"
#include "../include/sendfile_manager.h"

fd_set watch_list, master_list;
int head_fd;
struct timeval timeout;
char *last_data_packet;
char *panultimate_data_packet;

void main_loop()
{
    int selret, sock_index, fdaccept;
    timeout.tv_sec = 1000000;
    while(TRUE){
        watch_list = master_list;
        selret = select(head_fd+1, &watch_list, NULL, NULL, &timeout);

        if(selret < 0)
            ERROR("select failed.");

        if(selret > 0){
            /* Loop through file descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_fd; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){

                    /* control_socket */
                    if(sock_index == control_socket){
                        fdaccept = new_control_conn(sock_index);

                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_fd) head_fd = fdaccept;
                    }

                    /* router_socket */
                    else if(sock_index == router_socket){
                        //call handler that will call recvfrom() .....
                        recv_update(router_socket);
                    }

                    /* data_socket */
                    else if(sock_index == data_socket){
                        //new_data_conn(sock_index);
                        int newfd = new_data_conn(data_socket);
                        head_fd = newfd > head_fd ? newfd : head_fd;
                        FD_SET(newfd, &master_list);
                    }

                    /* Existing connection */
                    else{
                        if(isControl(sock_index)){
                            if(!control_recv_hook(sock_index)) 
                                FD_CLR(sock_index, &master_list);
                        }
                        else if(isData(sock_index)){
                            if(!data_recv_file(sock_index))
                                FD_CLR(sock_index, &master_list);
                        }
                        else ERROR("Unknown socket index");
                    }
                }
            }
        }
        
        if(selret == 0){
            update_time(router_socket);
            timeout = gettimeout();
            printf("Timeout:tv_sec = %ld,tv_usec = %ld\n",timeout.tv_sec,timeout.tv_usec);
        }
    }
}

void init()
{
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    last_data_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    panultimate_data_packet = (char*)malloc(PAYLOAD_LEN + DATA_PACKET_HEADER_OFFSET);
    main_loop();
}
