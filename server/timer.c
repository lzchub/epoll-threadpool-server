#include "timer.h"

static void timer_tick( timer_list_t* list );
static void timer_add_from_cur( timer_list_t* list,timers_t* timer );

static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;


/*****************************************************************************
 函 数 名  : timer_add
 功能描述  : 将定时器节点添加进链表
 输入参数  : struct timer_list_t* list,struct timer_t timer
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月3日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void timer_add( timer_list_t* list,timers_t* timer )
{
	pthread_mutex_lock(&timer_mutex);	
    if (NULL == timer)
    {
        goto EXIT;
    }
	if (NULL == list->head)
	{
	    list->head = timer;
		list->tail = timer;
		goto EXIT;
	}
	/*ascending list*/
	if (timer->expire <= list->head->expire)
	{
	    timer->next = list->head;
		list->head->prev = timer;
		list->head = timer;
	}
	else
	{
	    timers_t* tmp = list->head->next;
		while (NULL != tmp)
		{
		    if (timer->expire <= tmp->expire)
		    {
		        timer->next = tmp;
				timer->prev = tmp->prev;

				tmp->prev->next = timer;
				tmp->prev = timer;
				goto EXIT;
		    }
			tmp=tmp->next;
		}
		if (NULL == tmp)
		{
			timer->prev = list->tail;
		    list->tail->next = timer;
			list->tail = timer;
		}
	}
EXIT:
	pthread_mutex_unlock(&timer_mutex);
	return;
}


/*****************************************************************************
 函 数 名  : timer_adjust
 功能描述  : 调整定时器位置
 输入参数  : struct timer_list_t* list,struct timer_t* timer
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月3日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void timer_adjust( timer_list_t* list,timers_t* timer )
{
	pthread_mutex_lock(&timer_mutex);
	if (NULL == timer)	
	{
	    goto EXIT;
	}
	/*The timer is at the tail or the current time is less than the next timer time*/
	timers_t* tmp = timer->next;
	if (NULL == tmp || timer->expire <= tmp->expire)
	{
	    goto EXIT;
	}
	/*The timer is on the head, and it is taken out and re inserted*/
	if (timer == list->head)
	{
	    list->head = list->head->next;
		list->head->prev = NULL;
		timer_add(list,timer);
	}
	else
	{
	    timer_add_from_cur(list,timer);
	}
EXIT:
	pthread_mutex_unlock(&timer_mutex);
    return;
}


/*****************************************************************************
 函 数 名  : timer_add_from_cur
 功能描述  : 从当前位置插入定时器
 输入参数  : struct timer_list_t* list,struct timer_t* timer
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月4日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void timer_add_from_cur( timer_list_t* list,timers_t* timer )
{
    timers_t* tmp=timer->next;

	timer->next->prev = timer->prev;
	timer->prev->next = timer->next;

	while(NULL != tmp)
	{
	    if(timer->expire <= tmp->expire)
	    {
	        timer->next = tmp;
			timer->prev = tmp->prev;

			tmp->prev->next = timer;
			tmp->prev = timer;
	    }
		tmp=tmp->next;
	}
	if (NULL == tmp)
	{
	    timer->prev = list->tail;
		list->tail->next = timer;
		list->tail = timer;
	}
}

/*****************************************************************************
 函 数 名  : timer_list_distroy
 功能描述  : 销毁定时器链表
 输入参数  : struct timer_list_t* list
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月4日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void timer_list_distroy( timer_list_t* list )
{
    timers_t* tmp=list->head;
	while(NULL != tmp)
	{
	    list->head=tmp->next;
		free(tmp);
		tmp=list->head;
	}
}

/*****************************************************************************
 函 数 名  : timer_del
 功能描述  : 删除当前定时器
 输入参数  : struct timer_list_t* list,struct timer_t* timer
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月4日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void timer_del( timer_list_t* list,timers_t* timer )
{
	pthread_mutex_lock(&timer_mutex);
    if (NULL == timer)
    {
        goto EXIT;
    }
	if (timer == list->head && timer == list->tail)
	{
	    free(timer);
		list->head = NULL;
		list->tail = NULL;
		goto EXIT;
	}
	if (timer == list->head)
	{
	    list->head = list->head->next;
		list->head->prev = NULL;
		free(timer);
		goto EXIT;
	}
	if (timer == list->tail)
	{
	    list->tail = list->tail->prev;
		list->tail->next = NULL;
		free(timer);
		goto EXIT;
	}
	timer->next->prev = timer->prev;
	timer->prev->next = timer->next;
	free(timer);
EXIT:
	pthread_mutex_unlock(&timer_mutex);
	return;
}

/*****************************************************************************
 函 数 名  : timer_handler
 功能描述  : 定时器触发处理函数
 输入参数  : struct timer_list_t* list,struct timer_t* timer
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月4日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void timer_handler( timer_list_t* list )
{
    timer_tick(list);

	alarm(TIMESLOT);
}


/*****************************************************************************
 函 数 名  : timer_tick
 功能描述  : 定时器脉搏，处理过期定时器
 输入参数  : struct timer_list_t* list
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月4日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void timer_tick( timer_list_t* list )
{
	if(NULL == list->head)
	{
	    return;
	}

	timers_t* tmp=list->head;
	time_t cur=time(NULL);

	while (tmp != NULL)
	{
	    if (cur < tmp->expire)
	    {
	        break;
	    }
		epoll_con_free(tmp->index);
		list->head = tmp->next;
		if (NULL != list->head)
		{
		    list->head->prev = NULL;
		}
		tmp = list->head;
	}
	return;
}







