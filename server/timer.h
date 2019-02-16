#ifndef TIMER_H_
#define TIMER_H_

#include"global.h"

#define TIMESLOT	3

/*list timer node*/
typedef struct _timer_t
{	
	/*timeout time*/
	time_t expire;
	/*timeout callback function*/
	//void (*timer_func)(struct EPOLL_CONNECT*);
	/*callback func arg*/
	//struct EPOLL_CONNECT* con_data;
	int index;

	struct _timer_t* prev;
	struct _timer_t* next;
}timers_t;


/*ascending timer list*/
typedef struct _timer_list_t
{
	timers_t* head;
	timers_t* tail;
}timer_list_t;

void timer_add( timer_list_t* list,timers_t* timer );
void timer_adjust( timer_list_t* list,timers_t* timer );
void timer_list_distroy( timer_list_t* list );
void timer_handler( timer_list_t* list );


#endif



