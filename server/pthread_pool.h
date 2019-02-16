#include "global.h"
#include "timer.h"

/*10 thread*/
#define THREAD_NUMBER 	20
#define BUFFER_SIZE   	4096


typedef void* (*FUNC)(void* arg, int index);

typedef struct _thpool_job_func_parameter{
	char recv_buffer[BUFFER_SIZE];
	int fd;
}thpool_job_func_parameter;


/**
 * define a task note
 */
typedef struct _thpool_job_t{
    FUNC             			function;		// function point
//  void*                 		arg;     		// function parameter
    thpool_job_func_parameter 	arg;
	//timers_t* timer;
    struct _thpool_job_t* 		prev;     		// point previous note
    struct _thpool_job_t* 		next;     		// point next note
}thpool_job_t;

/**
 * define a job queue
 */
typedef struct _thpool_job_queue{
   thpool_job_t*    head;            	//queue head point
   thpool_job_t*    tail;             	//queue tail point
   int              jobN;               //task number
   sem_t*           queueSem;           //queue semaphore
}thpool_jobqueue;

/**
 * thread pool
 */
typedef struct _thpool_t{
   pthread_t*      	threads;    // thread point
   int             	threadsN;   // thread pool number
   thpool_jobqueue* jobqueue;  	// job queue
}thpool_t;

typedef struct _thpool_thread_parameter
{
	thpool_t*	thpool;
	int 		thread_index;
}thpool_thread_parameter;

int thpool_get_jobn(thpool_t* tp);
int thpool_add_job( thpool_t* tp,void*(*func_p)(void*arg,int idx),int sockfd,char* buffer,int index );
thpool_t* SRV_thpool_init(int thpool_number);
void SRV_thpool_destroy( thpool_t* tp );


	