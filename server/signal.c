#include "signal.h"
#include "log.h"

static void sig_handler( int sig );


extern int pipefd[2];


/*****************************************************************************
 �� �� ��  : set_nonblocking
 ��������  : ����������Ϊ������
 �������  : int fd
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��2��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
int set_nonblocking( int fd )
{
    int opt=-1;

	opt = fcntl(fd,F_GETFL);
	if(0 > opt)
	{
	    return -1;
	}
	opt = opt|O_NONBLOCK;
	if (0 > fcntl(fd,F_SETFL,opt))
	{
	    return -1;
	}
	return 0;
}

/*****************************************************************************
 �� �� ��  : SRV_add_sig
 ��������  : �����Ҫ������ź�
 �������  : int sig
 �������  : ��
 �� �� ֵ  : int
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��3��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
void SRV_add_sig( int sig )
{
    struct sigaction sa;
	sa.sa_handler = sig_handler;	
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	if(-1 == sigaction(sig,&sa,NULL))
	{
		LOG_STRING(LOG_LEVEL_ERROR,"sigaction error.\n");
	}
	LOG_STRING(LOG_LEVEL_INFO,"sig num %d success.\n",sig);
}

/*****************************************************************************
 �� �� ��  : sig_handler
 ��������  : �źŻص�����
 �������  : int sig
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2018��6��3��
    ��    ��   : chuan
    �޸�����   : �����ɺ���

*****************************************************************************/
static void sig_handler( int sig )
{
    //save old errno
    int save_errno = errno;
	int msg = sig;//set pipe
	send(pipefd[1],(char*)&msg,1,0);
	errno = save_errno;
}






