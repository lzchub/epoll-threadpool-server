#include "pthread_pool.h"
#include "log.h"

static void* thpool_thread_do( thpool_thread_parameter* para );
static void thpool_jobqueue_add( thpool_t* tp,thpool_job_t* newjob );
static int thpool_jobqueue_init( thpool_t* tp );
static thpool_job_t* thpool_jobqueue_peek( thpool_t* tp );
static void thpool_jobqueue_removelast( thpool_t* tp );
static void thpool_jobqueue_destroy( thpool_t* tp );

static pthread_mutex_t job_mutex = PTHREAD_MUTEX_INITIALIZER;

static int thpool_keepalive = 1;


/*****************************************************************************
 函 数 名  : thpool_get_jobn
 功能描述  : 得到当前线程池任务队列中的任务数
 输入参数  : struct thpool_t*
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月7日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int thpool_get_jobn(thpool_t* tp)
{
    if (NULL != tp && NULL != tp->jobqueue)
    {
        return tp->jobqueue->jobN;
    }
	else
	{
	    return 0;
	}
}

/*****************************************************************************
 函 数 名  : thpool_jobqueue_add
 功能描述  : 将新加的任务节点添加到任务队列
 输入参数  : struct thpool_t* tp,struct thpool_job_t* newjob
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月7日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void thpool_jobqueue_add( thpool_t* tp,thpool_job_t* newjob )
{
    thpool_job_t* oldHeadJob = tp->jobqueue->head;

	if (0 == tp->jobqueue->jobN)
	{
	    tp->jobqueue->head = newjob;
		tp->jobqueue->tail = newjob;
	}
	else
	{
	    oldHeadJob->prev = newjob;
		newjob->next = oldHeadJob;
		tp->jobqueue->head = newjob;
	}
	
	(tp->jobqueue->jobN)++;
	//V +1
	sem_post(tp->jobqueue->queueSem);
}


/*****************************************************************************
 函 数 名  : thpool_add_job
 功能描述  : 将epoll事件添加入事件队列
 输入参数  : struct thpool_t* tp,void*(*func_p)(void*arg,int idx),int sockfd,char* buffer
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月7日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int thpool_add_job( thpool_t* tp,void*(*func_p)(void*arg,int idx),int sockfd,char* buffer,int index )
{
	thpool_job_t* newjob=(thpool_job_t*)malloc(sizeof(thpool_job_t));
	if(NULL == newjob)
	{
	  	return -1;
	}
	newjob->function = func_p;
	memcpy(newjob->arg.recv_buffer,buffer,BUFFER_SIZE);
	newjob->arg.fd = sockfd;
	//LOG_STRING(LOG_LEVEL_INFO,"sockfd=%d,buffer:%s\n",newjob->arg.fd,newjob->arg.recv_buffer);
	//newjob->timer = epoll_get_timer(index);
	newjob->next=NULL;
	newjob->prev=NULL;
	pthread_mutex_lock(&job_mutex);
	thpool_jobqueue_add(tp,newjob);
	pthread_mutex_unlock(&job_mutex);
	
	return 0;
	
}


/*****************************************************************************
 函 数 名  : thpool_jobqueue_init
 功能描述  : 任务池初始化
 输入参数  : struct thpool_t* tp
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static int thpool_jobqueue_init( thpool_t* tp )
{
    tp->jobqueue = (thpool_jobqueue*)malloc(sizeof(thpool_jobqueue));
	if (NULL == tp->jobqueue)
	{
	    return -1;
	}

	tp->jobqueue->jobN = 0;
	tp->jobqueue->head = NULL;
	tp->jobqueue->tail = NULL;
	
	return 0;
}


/*****************************************************************************
 函 数 名  : SRV_thpool_init
 功能描述  : 线程池初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  : struct thpool_t*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
thpool_t* SRV_thpool_init(int thpool_number)
{
    thpool_t* thpool=NULL;
	int num=0;

	thpool = (thpool_t*)malloc(sizeof(thpool_t));
	if (NULL == thpool)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"thpool malloc error.\n");
		return NULL;
	}
	
	if (thpool_number<1)
	{
	    thpool_number=1;
	}
	thpool->threadsN = thpool_number;

	thpool->threads = (pthread_t*)malloc(sizeof(pthread_t)*thpool_number);
	if (NULL == thpool->threads)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"thpool->threads malloc error.\n");
		goto EXIT1;
	}
	if (-1 == thpool_jobqueue_init(thpool))
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"thpool->jobqueue init error.\n");
		goto EXIT2;
	}

	thpool->jobqueue->queueSem = (sem_t*)malloc(sizeof(sem_t));
	if (NULL == thpool->jobqueue->queueSem)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"thpool->jobqueue->queueSem malloc error.\n");
		goto EXIT3;
	}
	/*init 0*/
	sem_init(thpool->jobqueue->queueSem,0,0);
	
	for ( num=0; num<thpool_number; ++num)
	{
	    thpool_thread_parameter* para=(thpool_thread_parameter*)malloc(sizeof(thpool_thread_parameter));
		para->thpool = thpool;
		para->thread_index = num;
		pthread_create(&(thpool->threads[num]),NULL,(void*)thpool_thread_do,(void*)para);
	}

	return thpool;

EXIT3:	
	free(thpool->jobqueue);
EXIT2:
	free(thpool->threads);
EXIT1:
	free(thpool);
	return NULL;
}


/*****************************************************************************
 函 数 名  : thpool_thread_do
 功能描述  : 线程池处理函数
 输入参数  : void* para
 输出参数  : 无
 返 回 值  : static void*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月12日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void* thpool_thread_do( thpool_thread_parameter* para )
{
    thpool_t* tp=para->thpool;
	int index =para->thread_index;
	LOG_STRING(LOG_LEVEL_INFO,"thread index %d create success.\n",index);

	/*exit release of resources*/
	pthread_detach(pthread_self());

	while (1 == thpool_keepalive)
	{
		//P -1
		if (-1 == sem_wait(tp->jobqueue->queueSem))
		{
		    LOG_STRING(LOG_LEVEL_ERROR,"sem_wait error.\n");
			exit(1);
		}

		if (1 == thpool_keepalive)
		{
			//LOG_STRING(LOG_LEVEL_INFO,"thread %d begin work.\n",index);
			/*thread handler func*/
		    FUNC func;
			thpool_job_t* job=NULL;
			/*use jobqueue*/
			pthread_mutex_lock(&job_mutex);
			/*get last job*/
			job = thpool_jobqueue_peek(tp);
			func = job->function;
			thpool_jobqueue_removelast(tp);
			pthread_mutex_unlock(&job_mutex);
			
			/*remove last job*/
			
			/*
			*arg->buf arg->fd
			*index:  thread index
			*/
			func((void*)&job->arg,index);
			//LOG_STRING(LOG_LEVEL_INFO,"thread %d end.\n",index);

			free(job);
			
		}		
	}
	free(para);
	return NULL;
}

/*****************************************************************************
 函 数 名  : thpool_jobqueue_peek
 功能描述  : 返回任务池链表最后一个任务
 输入参数  : thpool_t* tp
 输出参数  : 无
 返 回 值  : static thpool_job_t*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月12日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static thpool_job_t* thpool_jobqueue_peek( thpool_t* tp )
{
    return tp->jobqueue->tail;
}

/*****************************************************************************
 函 数 名  : thpool_jobqueue_removelast
 功能描述  : 移除任务池最后一个任务
 输入参数  : thpool_t* tp
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月12日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void thpool_jobqueue_removelast( thpool_t* tp )
{
    thpool_job_t* lastJob = NULL;
	lastJob = tp->jobqueue->tail;

	if (1 == tp->jobqueue->jobN)
	{
	    tp->jobqueue->head = NULL;
		tp->jobqueue->tail = NULL;
	}
	else
	{
	    lastJob->prev->next = NULL;
		tp->jobqueue->tail = lastJob->prev;
	}
	(tp->jobqueue->jobN)--;
	return;
}

/*****************************************************************************
 函 数 名  : SRV_thpool_destroy
 功能描述  : 释放线程池
 输入参数  : thpool* tp
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月20日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void SRV_thpool_destroy( thpool_t* tp )
{
	int i=0;
    thpool_keepalive=0;
	if (NULL == tp)
	{
	    return;
	}
	for ( i=0; i<(tp->threadsN); i++ )
	{
	    sem_post(tp->jobqueue->queueSem);
		LOG_STRING(LOG_LEVEL_INFO,"thread end.\n");
	}
	if (-1 == sem_destroy(tp->jobqueue->queueSem))
	{
	    LOG_STRING(LOG_LEVEL_INFO,"sem_destroy error.\n");
	}
	/*for ( i=0; i<(tp->threadsN); i++)
	{
	    pthread_join(tp->threads[i],NULL);
	}*/
	thpool_jobqueue_destroy(tp);

	free(tp->threads);
	free(tp->jobqueue->queueSem);
	free(tp->jobqueue);
	free(tp);
}

/*****************************************************************************
 函 数 名  : thpool_jobqueue_destroy
 功能描述  : 释放任务节点
 输入参数  : thpool_t* tp
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月20日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void thpool_jobqueue_destroy( thpool_t* tp )
{
    thpool_job_t* curjob = tp->jobqueue->head;

	while (tp->jobqueue->jobN)
	{
	    tp->jobqueue->head = curjob->next;
		free(curjob);
		curjob = tp->jobqueue->head;
		--tp->jobqueue->jobN;
	}
	tp->jobqueue->head=NULL;
	tp->jobqueue->tail=NULL;
}






