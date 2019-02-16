#include "timer.h"

static void timer_tick( timer_list_t* list );
static void timer_add_from_cur( timer_list_t* list,timers_t* timer );

static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;


/*****************************************************************************
 �� �� ��  : timer_add
 ��������  : ����ʱ���ڵ���ӽ�����
 �������  : struct timer_list_t* list,struct timer_t timer
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��3��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : timer_adjust
 ��������  : ������ʱ��λ��
 �������  : struct timer_list_t* list,struct timer_t* timer
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��3��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : timer_add_from_cur
 ��������  : �ӵ�ǰλ�ò��붨ʱ��
 �������  : struct timer_list_t* list,struct timer_t* timer
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��4��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : timer_list_distroy
 ��������  : ���ٶ�ʱ������
 �������  : struct timer_list_t* list
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��4��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : timer_del
 ��������  : ɾ����ǰ��ʱ��
 �������  : struct timer_list_t* list,struct timer_t* timer
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��4��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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
 �� �� ��  : timer_handler
 ��������  : ��ʱ������������
 �������  : struct timer_list_t* list,struct timer_t* timer
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��4��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
void timer_handler( timer_list_t* list )
{
    timer_tick(list);

	alarm(TIMESLOT);
}


/*****************************************************************************
 �� �� ��  : timer_tick
 ��������  : ��ʱ��������������ڶ�ʱ��
 �������  : struct timer_list_t* list
 �������  : ��
 �� �� ֵ  : static void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��4��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

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







