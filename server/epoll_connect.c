#include "epoll_connect.h"
#include "log.h"

static void epoll_lock_state( int index,int lock );

extern EPOLL_CONNECT epoll_confd[MAX_EVENTS];
extern timer_list_t* timer_list;


/*****************************************************************************
 函 数 名  : SRV_epoll_con_init
 功能描述  : 初始化epoll数组
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月1日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void SRV_epoll_con_init( void )
{
	int index;

	memset(epoll_confd,'\0',sizeof(epoll_confd));
	for(index=0;index<MAX_EVENTS;++index)
	{
		epoll_confd[index].con_fd=-1;
		if(-1==pthread_mutex_init(&epoll_confd[index].mutex,NULL))
		{
			LOG_STRING(LOG_LEVEL_ERROR,"epoll_confd[%d] mutex init error.\n",index);
		}
	}
}

/*****************************************************************************
 函 数 名  : epoll_get_free_con_index
 功能描述  : 得到事件队列中空闲事件坐标
 输入参数  : void
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int epoll_get_free_con_index( void )
{
    int index=0;
	for ( index=0; index<MAX_EVENTS; ++index )
	{
	    if (-1 == epoll_confd[index].con_fd)
	    {
	        return index;
	    }
	}
	return -1;
}

/*****************************************************************************
 函 数 名  : epoll_lock_state
 功能描述  : 将队节点锁的状态
 输入参数  : int index,int lock
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void epoll_lock_state( int index,int lock )
{
	int res=-1;
    if (lock)
    {
        res = pthread_mutex_lock(&epoll_confd[index].mutex);
    }
	else
	{
	    res = pthread_mutex_unlock(&epoll_confd[index].mutex);
	}
	if (0 > res)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"event[%d] mutex lock[%d] error.\n",index,lock);
	}
}



/*****************************************************************************
 函 数 名  : epoll_con_init
 功能描述  : 初始化事件队列节点
 输入参数  : int index,int confd,struct sockaddr_in conaddr
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void epoll_con_init( int index,int confd,struct sockaddr_in conaddr )
{
    epoll_lock_state(index,1);
	epoll_confd[index].con_fd=confd;
	epoll_confd[index].con_addr=conaddr;
	epoll_confd[index].status=0;
	timers_t* timer=(timers_t*)malloc(sizeof(timers_t));
	time_t cur = time(NULL);
	/*timeout 15s*/
	timer->index = index;
	timer->expire = cur + TIMESLOT*3;
	timer->prev = NULL;
	timer->next = NULL;
	//timer.timer_func = cb_timer_func;
	//timer.con_data = &epoll_confd[index];
	epoll_confd[index].timer = timer;
	
	timer_add(timer_list,timer);
	
	epoll_lock_state(index,0);
}

/*****************************************************************************
 函 数 名  : epoll_change_status
 功能描述  : 改变连接status属性
 输入参数  : int index
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void epoll_change_status( int index )
{
    epoll_lock_state(index,1);
	epoll_confd[index].status=1;
	epoll_lock_state(index,0);
}



/*****************************************************************************
 函 数 名  : epoll_match_index
 功能描述  : 通过fd从事件队列中匹配坐标
 输入参数  : int fd
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月7日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int epoll_match_index( int fd )
{
    int index=0;
	for ( index=0; index<MAX_EVENTS; ++index )
	{
	    if (epoll_confd[index].con_fd == fd)
	    {
	        return index;
	    }
	}
	return -1;
}

/*****************************************************************************
 函 数 名  : epoll_free
 功能描述  : 释放事件队列节点
 输入参数  : int index
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void epoll_free( int index )
{
    epoll_lock_state(index,1);
	epoll_confd[index].con_fd=-1;
	timer_del(timer_list,epoll_confd[index].timer);
	epoll_confd[index].status=0;
	LOG_STRING(LOG_LEVEL_INFO,"close index=%d\n",index);
	epoll_lock_state(index,0);
}

/*****************************************************************************
 函 数 名  : epoll_update_timer
 功能描述  : 更新定时器
 输入参数  : int index
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void epoll_update_timer( int index )
{
    time_t cur=time(NULL);
	epoll_confd[index].timer->expire = cur + TIMESLOT*3;
	timer_adjust(timer_list,epoll_confd[index].timer);
}

/*****************************************************************************
 函 数 名  : epoll_get_timer
 功能描述  : 返回index对应的定时器
 输入参数  : int index
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月9日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
struct timers_t* epoll_get_timer( int index )
{
    return epoll_confd[index].timer;
}






