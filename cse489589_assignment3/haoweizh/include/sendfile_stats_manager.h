#ifndef _SENDFILE_STATS_MANAGER_H_
#define _SENDFILE_STATS_MANAGER_H_

#define SENDFILE_STATS_HEAD 4

void sendfile_stats_response(int sock_index, char *cntrl_payload, uint16_t payload_len);

#endif
