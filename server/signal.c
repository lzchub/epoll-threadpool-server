#include "signal.h"
#include "log.h"

static void sig_handler( int sig );


extern int pipefd[2];


/*****************************************************************************
 函 数 名  : set_nonblocking
 功能描述  : 将链接设置为非阻塞
 输入参数  : int fd
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

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
 函 数 名  : SRV_add_sig
 功能描述  : 添加需要捕获的信号
 输入参数  : int sig
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月3日
    作    者   : chuan
    修改内容   : 新生成函数

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
 函 数 名  : sig_handler
 功能描述  : 信号回调函数
 输入参数  : int sig
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月3日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void sig_handler( int sig )
{
    //save old errno
    int save_errno = errno;
	int msg = sig;//set pipe
	send(pipefd[1],(char*)&msg,1,0);
	errno = save_errno;
}






