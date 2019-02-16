#ifndef EPOLL_CONNECT_H_
#define EPOLL_CONNECT_H_

#include "global.h"
#include "timer.h"

#define MAX_EVENTS	65535
#define LISTEN	4096

typedef struct _EPOLL_CONNECT
{
	/*connect handler*/
	int con_fd;
	/*0->epoll 1->job or thread   for timer*/
	int status;
	/*connect addr info*/
	struct sockaddr_in con_addr;

	/*ascending list timer*/
	timers_t* timer;
	
	/*lock*/
	pthread_mutex_t mutex;
}EPOLL_CONNECT;


void SRV_epoll_con_init( void );
int epoll_get_free_con_index( void );
void epoll_con_init( int index,int confd,struct sockaddr_in conaddr );
void epoll_change_status( int index );
int epoll_match_index( int fd );
void epoll_free( int index );

#endif

