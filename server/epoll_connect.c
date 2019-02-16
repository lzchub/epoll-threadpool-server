#include "epoll_connect.h"
#include "log.h"

static void epoll_lock_state( int index,int lock );

extern EPOLL_CONNECT epoll_confd[MAX_EVENTS];
extern timer_list_t* timer_list;


/*****************************************************************************
 �� �� ��  : SRV_epoll_con_init
 ��������  : ��ʼ��epoll����
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��1��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_get_free_con_index
 ��������  : �õ��¼������п����¼�����
 �������  : void
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��2��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_lock_state
 ��������  : ���ӽڵ�����״̬
 �������  : int index,int lock
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��2��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_con_init
 ��������  : ��ʼ���¼����нڵ�
 �������  : int index,int confd,struct sockaddr_in conaddr
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��2��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_change_status
 ��������  : �ı�����status����
 �������  : int index
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��8��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
void epoll_change_status( int index )
{
    epoll_lock_state(index,1);
	epoll_confd[index].status=1;
	epoll_lock_state(index,0);
}



/*****************************************************************************
 �� �� ��  : epoll_match_index
 ��������  : ͨ��fd���¼�������ƥ������
 �������  : int fd
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��7��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_free
 ��������  : �ͷ��¼����нڵ�
 �������  : int index
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��8��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : epoll_update_timer
 ��������  : ���¶�ʱ��
 �������  : int index
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��9��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
void epoll_update_timer( int index )
{
    time_t cur=time(NULL);
	epoll_confd[index].timer->expire = cur + TIMESLOT*3;
	timer_adjust(timer_list,epoll_confd[index].timer);
}

/*****************************************************************************
 �� �� ��  : epoll_get_timer
 ��������  : ����index��Ӧ�Ķ�ʱ��
 �������  : int index
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��9��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
struct timers_t* epoll_get_timer( int index )
{
    return epoll_confd[index].timer;
}






