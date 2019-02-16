#include "global.h"
#include "parse_config.h"
#include "log.h"
#include "epoll_connect.h"
#include "signal.h"
#include "pthread_pool.h"
#include "timer.h"

static int SRV_daemon( void );
static void SRV_print_pid( int option );
static void SRV_open_file_handle( void  );
static void closesock( int fd );
static void epoll_con_total( BOOL add_or_del, int value );
static void* accept_func( void );
static int SRV_creat_apt_task( void );
static int SRV_recv_buffer( int sockfd,unsigned char* buffer,int* length );
static void* respons_stb_info( thpool_job_func_parameter* paramter,int thread_index );
static int send_buffer_to_fd( int sockfd, char* sendbuf, int len );

#define PID_FILE	"pid.file"	

/*save the file relative path*/
char* g_relpath=NULL;
static pthread_t accept_thread;
static pthread_mutex_t total_mutex = PTHREAD_MUTEX_INITIALIZER;
static int apt_exit_flag=0;
static int cur_con_total=0;
static int stop_server=0;
static int epoll_fd=-1;
static int listenfd=-1;

EPOLL_CONNECT epoll_confd[MAX_EVENTS];
int pipefd[2];
timer_list_t* timer_list;

static int port;



/*****************************************************************************
 函 数 名  : SRV_daemon
 功能描述  : 将服务器进程设置为守护进程
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static int SRV_daemon( void )
{
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		LOG_STRING(LOG_LEVEL_ERROR,"fork error.\n");
		return -1;
	}
	if (pid > 0)
	{
		exit (EXIT_SUCCESS);
	}
	/*child process give to init process*/
	signal(SIGCHLD,SIG_IGN);
	/*set new session*/
	setsid();
	/*avoid open terminal again*/
	pid = fork();
	if (pid < 0)
	{
		LOG_STRING(LOG_LEVEL_ERROR,"fork error.\n");
		return -1;
	}
	if (pid > 0)
	{
		exit (EXIT_SUCCESS);
	}
	
	//设置屏蔽集
	umask(0);
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return 0;
}

/*****************************************************************************
 函 数 名  : SRV_print_pid
 功能描述  : 将服务器进程号打印到文件
 输入参数  : int option
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void SRV_print_pid( int option )
{
	if (option)
	{
	    FILE* fp=NULL;
		fp = fopen(PID_FILE,"w+");
		if ( NULL == fp )
		{
	    	LOG_STRING(LOG_LEVEL_ERROR,"fopen error.\n");
			return;
		}
		fprintf(fp,"%d",(int)getpid());
		fclose(fp);
	}
}

/*****************************************************************************
 函 数 名  : SRV_open_file_handle
 功能描述  : 打开linux最大文件限制
 输入参数  : 无
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void SRV_open_file_handle( void  )
{
    struct rlimit rlt;
	rlt.rlim_cur = MAX_EVENTS;
	rlt.rlim_cur = MAX_EVENTS;

	setrlimit(RLIMIT_NOFILE,&rlt);
	getrlimit(RLIMIT_NOFILE,&rlt);
	
	LOG_STRING(LOG_LEVEL_INFO,"cur file handle:%d\n",(int)rlt.rlim_cur);
	LOG_STRING(LOG_LEVEL_INFO,"max file handle:%d\n",(int)rlt.rlim_max);
}


/*****************************************************************************
 函 数 名  : closesock
 功能描述  : 关闭连接socket
 输入参数  : int fd
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void closesock( int fd )
{
	/*ensure that data will not be lost*/
    shutdown(fd,SHUT_RDWR);
	close(fd);
}


/*****************************************************************************
 函 数 名  : epoll_con_total
 功能描述  : 链接总数变化
 输入参数  : BOOL add_or_del
             int value
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void epoll_con_total( BOOL add_or_del, int value )
{
	pthread_mutex_lock(&total_mutex);
    if(add_or_del)
    {
    	cur_con_total+=value;
    }
	else
	{
	    cur_con_total-=value;
	}
	pthread_mutex_unlock(&total_mutex);
}


/*****************************************************************************
 函 数 名  : epoll_add_event
 功能描述  : 将事件添加到epoll链表
 输入参数  : int confd
 输出参数  : int
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月2日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int epoll_add_event(int confd)
{
	/*set no blocking*/
	if(-1 == set_nonblocking(confd))
	{
		LOG_STRING(LOG_LEVEL_ERROR,"set nonblocking error.\n");
		goto EXIT;
	}
    struct epoll_event ev;
	ev.data.fd = confd;
	ev.events = EPOLLIN | EPOLLET;  //epoll type
	if (-1 == epoll_ctl(epoll_fd,EPOLL_CTL_ADD,confd,&ev))
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"EPOLL_CTL_ADD errno %d,%s",errno,strerror(errno));
		goto EXIT;
	}
	return 0;
EXIT:
	if (-1!=confd)
	{
		    closesock(confd);
			confd=-1;
			return -1;
	}
}



/*****************************************************************************
 函 数 名  : accept_func
 功能描述  : 服务器的监听线程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : static void*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月1日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void* accept_func( void )
{
	int confd;
	int res=0;
	int val=1;
	int rcvsize=32* 1024;
	struct sockaddr_in srvaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen=sizeof(cliaddr);
	int epoll_con_index;

	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if (-1 == listenfd)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"socket error.\n");
		return NULL;
	}
	/*allow address reuse*/
	res=setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
	if (-1 == res)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"setsockopt SO_REUSEADDR error.\n");
		return NULL;
	}
	/*set recv buffur*/
	res=setsockopt(listenfd,SOL_SOCKET,SO_RCVBUF,(char*)(&rcvsize),sizeof(rcvsize));
	if(-1 == res)
	{
		LOG_STRING(LOG_LEVEL_ERROR,"listenfd setsockopt SO_RCVBUF error.\n");
		return NULL;
	}
	/*
	*In the third phase of the three handshake,
	*the server will not immediately establish a connection after receiving the ACK packet returned by the client,
	*but when the client sends the data, the connection is set up,
	*and the server will give up the connection if it is out of time
	*/
	/*val=10; //10s
	res=setsockopt(listenfd,IPPROTO_TCP,TCP_DEFER_ACCEPT,&val,sizeof(val));
	if (-1 == res)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"setsockopt TCP_DEFER_ACCEPT error.\n");
		return NULL;
	}*/
	srvaddr.sin_addr.s_addr = INADDR_ANY;
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(port);

	if (-1 == bind(listenfd,(struct sockaddr*)&srvaddr,sizeof(srvaddr)))
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"bind error.errno=%d,%s\n",errno,strerror(errno));
		return NULL;
	}

	if(-1 == listen(listenfd,LISTEN))
	{
		LOG_STRING(LOG_LEVEL_ERROR,"listen error.\n");
		return NULL;
	}
	LOG_STRING(LOG_LEVEL_INDISPENSABLE,"server bind port %d,and listen success.\n",port);

	/*init timer list*/
	timer_list = (timer_list_t*)malloc(sizeof(timer_list_t));
	if (NULL == timer_list)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"timer list init error.\n");
		return NULL;
	}
	timer_list->head=NULL;
	timer_list->tail=NULL;
	
	while(!apt_exit_flag)
	{	
		if (cur_con_total < MAX_EVENTS)
		{
		    confd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
			if(confd<0)
			{
				if (errno!=EAGAIN && errno!=EPROTO && errno!=EINTR && errno!=ECONNABORTED)
				{
				    LOG_STRING(LOG_LEVEL_ERROR,"accept errno %d,%s.\n",errno,strerror(errno));
				}
				continue;
			}
		}
		else
		{
		    sleep(1);
			LOG_STRING(LOG_LEVEL_INDISPENSABLE,"cur accept number achieve to maxnum(%d).\n",MAX_EVENTS);
			continue;
		}
		
		/*get free index*/
		epoll_con_index = epoll_get_free_con_index();
		
		if (-1 == epoll_con_index)
		{
		    LOG_STRING(LOG_LEVEL_ERROR,"not found free connect.\n");
			if (-1!=confd)
			{
			    closesock(confd);
				confd=-1;
			}
			continue;
		}
		res=setsockopt(confd,SOL_SOCKET,SO_RCVBUF,(char*)(&rcvsize),sizeof(rcvsize));
		if(-1 == res)
		{
			LOG_STRING(LOG_LEVEL_ERROR,"confd setsockopt SO_RCVBUF error.\n");
			if (-1!=confd)
			{
			    closesock(confd);
				confd=-1;
			}
			continue;
		}
		
		/*add the event to the epoll list*/
		if(0 > epoll_add_event(confd))
			continue;
		epoll_con_total(TRUE,1);
		epoll_con_init(epoll_con_index,confd,cliaddr);
		LOG_STRING(LOG_LEVEL_INFO,"connect from %s,sockfd %d,current connect total %d,index=%d\n",
			inet_ntoa(cliaddr.sin_addr),confd,cur_con_total,epoll_con_index);
	}	
	if(-1 != listenfd)
	{
		closesock(listenfd);
		listenfd=-1;
	}
	return NULL;
}



/*****************************************************************************
 函 数 名  : SRV_creat_apt_task
 功能描述  : 创建监听线程
 输入参数  : void
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月1日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static int SRV_creat_apt_task( void )
{
    int res=0;
	res = pthread_create(&accept_thread,NULL,accept_func,NULL);
	if (-1 == res)
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"create accept thread error.\n");
	}
	return res;
}

/*****************************************************************************
 函 数 名  : SRV_recv_buffer
 功能描述  : 接收客户端发送来的请求数据,且一次接收1024字节
 输入参数  : int sockfd,char* buffer,int* length
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月7日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static int SRV_recv_buffer( int sockfd,unsigned char* buffer,int* length )
{
    int total_length=0;
	int recv_length=0;
	int once_length=1024;
	int continue_read=1;
	int res=-1;
	
	while (continue_read)
	{
	    recv_length = recv(sockfd,&buffer[total_length],once_length,0);
		/*error*/
		if (0 > recv_length)
		{
		    continue_read = 0;
			break;
		}
		/*over*/
		else if(recv_length>=0 && recv_length<once_length)
		{
		    continue_read = 0;
			*length = total_length+recv_length;
			res = 0;
			break;
		}
		/*not end*/
		else
		{
		    total_length+=recv_length;
		}
	}
	DBG_PRINT(LOG_LEVEL_INFO,"buflen %d\n",*length);
	return res;
}

/*****************************************************************************
 函 数 名  : respons_stb_info
 功能描述  : 输入的回调处理函数
 输入参数  : struct thpool_job_func_parameter* paramter,int thread_index
 输出参数  : 无
 返 回 值  : static void*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void* respons_stb_info( thpool_job_func_parameter* paramter,int thread_index )
{
	char send_buffer[1024] = "server have get you data.\n";
	int sockfd = paramter->fd;
	LOG_STRING(LOG_LEVEL_INFO,"thidx=%d,fd=%d,recbuf=%s\n",thread_index,paramter->fd,paramter->recv_buffer);
	//send_buffer_to_fd(sockfd,send_buffer,strlen(send_buffer));

	
	/*if (-1 != sockfd)
	{
	    int index=epoll_match_index(sockfd);
		epoll_confd[index].status=0;
		epoll_con_free(index);
	}*/
}

/*****************************************************************************
 函 数 名  : epoll_con_free
 功能描述  : 将超时连接关闭,并移除epoll链表
 输入参数  : int index
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月8日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void epoll_con_free( int index )
{
	/*job or thread*/
    if (1 == epoll_confd[index].status)
    {
    	epoll_update_timer(index);
        return;
    }
	struct epoll_event ev;
	if (-1 == epoll_ctl(epoll_fd,EPOLL_CTL_DEL,epoll_confd[index].con_fd,&ev))
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"EPOLL_CTL_DEL error.\n");
	}
	if(-1 != epoll_confd[index].con_fd)
	{
	    closesock(epoll_confd[index].con_fd);
	}
	epoll_free(index);
	epoll_con_total(FALSE,1);
	return;
}

/*****************************************************************************
 函 数 名  : send_buffer_to_fd
 功能描述  : 将服务器准备的数据发送给客户端
 输入参数  : int sockfd
             char* sendbuf
             int len
 输出参数  : 无
 返 回 值  : static int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年6月12日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static int send_buffer_to_fd( int sockfd, char* sendbuf, int len )
{
    int size=0;
	int n = len;

	while (n > 0)
	{
	    size = write(sockfd,sendbuf+len-n,n);
		/*len > available buffer*/
		/*https://blog.csdn.net/sinat_35261315/article/details/78357586?locationNum=10&fps=1*/
		if (size<n)
		{
		    if (-1 == size)
		    {
		    	/*buffer full*/
		        if (errno!=EAGAIN && errno!=EWOULDBLOCK)
		        {
		            LOG_STRING(LOG_LEVEL_ERROR,"sockfd %d,write errno %d:%s.\n",sockfd,errno,strerror(errno));
					return -1;
		        }
				else
				{
				    LOG_STRING(LOG_LEVEL_INFO,"write EAGAIN.\n");
				}
		    }
		}
		n-=size;
	}
	return (len-n);
}





int main(int argc,char* argv[])
{
	char isdaemon;
	char* portp=NULL;
	char* logp=NULL;
	int event_number=0;
	struct epoll_event events[MAX_EVENTS];
	int index=0;
	int ret=0;
	BOOL timeout=FALSE;
	thpool_t* thpool=NULL;

	/*get the current file path*/
	g_relpath = get_current_dir_name();

	#ifdef HTTPS
	int sslport;
	char dossl;
	char* sslportp=NULL;
	#endif
	/*for config.ini*/
	#ifdef HTTPS
	SRV_parse_option(argc,argv,&isdaemon,&portp,&logp,&sslportp,&dossl);

	sslportp==NULL?(sslport=atoi(SRV_getconfig("https"))):(sslport=atoi(sslportp));
	if(1 != dossl)
	{
		if(0 == strcmp(SRV_getconfig("dossl"),"yes"))
		{
			dossl = 1;
		}
	}
	#else
	SRV_parse_option(argc,argv,&isdaemon,&portp,&logp);
	#endif

	portp==NULL?(port=atoi(SRV_getconfig("http"))):(port=atoi(portp));
	if(1 != isdaemon)
	{
		if(0 == strcmp(SRV_getconfig("daemon"),"yes"))
		{
			isdaemon= 1;
		}
	}
	if(NULL==logp)
	{
		logp=SRV_getconfig("log");
	}

	
	/*for log init*/
	SRV_set_log_name(logp);
	SRV_log_init();
	SRV_set_log_level(atoi(SRV_getconfig("loglevel")));
	
	/*set daemon*/
	if(0 == strcmp(SRV_getconfig("daemon"),"yes"))
	{
		if(-1 == SRV_daemon())
		{
			SRV_log_close();
			return 0;
		}
	}
	
	SRV_print_pid(1);
	/*open file restrict*/
	//SRV_open_file_handle();


	/*init epoll connect*/
	SRV_epoll_con_init();

	epoll_fd = epoll_create(MAX_EVENTS);


	/*create accept socket connect*/
	SRV_creat_apt_task();
	
	/*signal*/
	if(-1 == socketpair(PF_UNIX,SOCK_STREAM,0,pipefd))
	{
		LOG_STRING(LOG_LEVEL_ERROR,"socketpair error.\n");
	}
	SRV_add_sig(SIGPIPE);
	SRV_add_sig(SIGTERM);
	SRV_add_sig(SIGALRM);

	if (-1 == epoll_add_event(pipefd[0]))
	{
	    LOG_STRING(LOG_LEVEL_ERROR,"add sig to epoll error.\n");
	}

	/*init thpool*/
	thpool = SRV_thpool_init(THREAD_NUMBER);
	/*send SIGALRM*/

	alarm(TIMESLOT);


	while (0==stop_server)
	{
	    event_number = epoll_wait(epoll_fd,events,MAX_EVENTS,2000);
		if (event_number<0 && errno!=EINTR)
		{
		    LOG_STRING(LOG_LEVEL_ERROR,"epoll_wait error.\n");
			break;
		}
		for (index=0; index<event_number; ++index)
		{
		    int sockfd = events[index].data.fd;
			/*signal handle*/
			if ((sockfd==pipefd[0]) && (events[index].events & EPOLLIN))
			{
			    int i;
				char signals[64];
				ret = recv(pipefd[0],signals,sizeof(signals),0);
				if (-1 == ret)
					continue;
				else if (0 == ret)
					continue;
				else
				{
				    for (i=0;i<ret;++i)
				    {
				        switch (signals[i])
				        {
							case SIGTERM:
							{
								stop_server=1;
								LOG_STRING(LOG_LEVEL_INFO,"SIGTERM handle.\n");
			               		break;
							}
				            case SIGPIPE:
							{
								signal(SIGPIPE,SIG_IGN);
								LOG_STRING(LOG_LEVEL_INFO,"SIGPIPE handle.\n");
				                break;
							}  
							case SIGALRM:
							{
								timeout=TRUE;
								LOG_STRING(LOG_LEVEL_INFO,"SIGALRM handle.\n");
			                	break;
							}
				        }
				    }
				}
			}	
			/*have read event*/
			else if (events[index].events & EPOLLIN)
			{
			    int event_index = -1;
				int length = 0;
				unsigned char buffer[BUFFER_SIZE]="";
				event_index=epoll_match_index(sockfd);
				if (0 == SRV_recv_buffer(sockfd,buffer,&length))
				{
					//epoll_update_timer(event_index);
					epoll_change_status(event_index);
					/*recv buffer success,then add to job*/
					//LOG_STRING(LOG_LEVEL_INFO,"[INDEX %d][FD %d][JOBN %d]\n",event_index,sockfd,thpool_get_jobn(thpool));
					thpool_add_job(thpool,(void*)respons_stb_info,sockfd,buffer,event_index);
				}
				/*error or close*/
			}
		}
		/*finally, the timing event is processed because the IO event has a higher priority.
		 *Of course, this will lead to timing tasks not exactly as expected.
		 */
		if (timeout==TRUE)
		{
		    timer_handler(timer_list);
		    LOG_STRING(LOG_LEVEL_INFO,"timeout\n");
			timeout=FALSE;
		}	
	}
	

#if 0
	apt_exit_flag=1;

	SRV_thpool_destroy(thpool);

	SRV_log_close();
#endif
	
	return 0;
}




