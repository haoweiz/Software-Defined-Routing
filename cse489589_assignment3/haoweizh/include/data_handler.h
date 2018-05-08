#ifndef _DATA_HANDLER_H_
#define _DATA_HANDLER_H_
#include "../include/global.h"

bool isData(int sock_index);
bool data_recv_file(int sock_index);
int new_data_conn(int data_socket);

#endif
