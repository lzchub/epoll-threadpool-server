#include "log.h"

static void creat_log_current_time_stamp( void );

static char log_file_name[LOG_FILE_NAME_SIZE];
static FILE* log_file_handle = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char	log_time_stamp[LOG_TIME_STAMP_SIZE];
static int log_level = 0;
static char log_last_str[LOG_STR_BUF_SIZE];

/*****************************************************************************
 函 数 名  : SRV_set_log_name
 功能描述  : 设置日志文件名
 输入参数  : char* log_file_name
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void SRV_set_log_name( char* log_name )
{
    snprintf(log_file_name,strlen(log_name),"%s",log_name);
}

/*****************************************************************************
 函 数 名  : SRV_log_init
 功能描述  : 日志系统初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int SRV_log_init( void )
{
	/*only write*/
	log_file_handle = fopen(log_file_name,"w");
	if(NULL == log_file_handle)
	{
		printf("[%s %s %d] fopen error.\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(log_last_str,'\0',sizeof(log_last_str));

	DBG_PRINT(LOG_LEVEL_INDISPENSABLE,"log start!\n");
	//LOG_STRING(LOG_LEVEL_INDISPENSABLE,"LOG START!\n");
}

/*****************************************************************************
 函 数 名  : log_string
 功能描述  : 日志的写入
 输入参数  : int level,
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
int log_string( int level,const char* file,const char* func,long line,char* format,... )
{
	char log_str_buf[LOG_STR_BUF_SIZE];
	char log_str_temp[LOG_STR_BUF_SIZE];
	int log_need_record = 0;
	va_list arg;

	va_start(arg,format);
	vsnprintf(log_str_temp,LOG_STR_BUF_SIZE,format,arg);
	va_end(arg);

	pthread_mutex_lock(&log_mutex);
	creat_log_current_time_stamp();
	/*record log diffrent with last*/
	if(strcmp(log_last_str,log_str_temp))
	{	
		memset(log_last_str,'\0',sizeof(log_last_str));
		strcpy(log_last_str,log_str_buf);
		switch ( level )
		{
		    case LOG_LEVEL_INFO:
				if ( log_level<=level )
				{
					log_need_record = 1;
				    snprintf(log_str_buf,LOG_STR_BUF_SIZE,"[INFO %s][%s %s:%d]%s",log_time_stamp,file,func,(int)line,log_str_temp);
				}
		        break;
		    case LOG_LEVEL_WARNING:
		        if ( log_level<=level )
				{
					log_need_record = 1;
				    snprintf(log_str_buf,LOG_STR_BUF_SIZE,"[WARNING %s][%s %s:%d]%s",log_time_stamp,file,func,(int)line,log_str_temp);
				}
		        break;
		    case LOG_LEVEL_ERROR:
		        if ( log_level<=level )
				{
					log_need_record = 1;
				    snprintf(log_str_buf,LOG_STR_BUF_SIZE,"[ERROR %s][%s %s:%d]%s",log_time_stamp,file,func,(int)line,log_str_temp);
				}
		        break;
		    case LOG_LEVEL_FATAL:
		        if ( log_level<=level )
				{
					log_need_record = 1;
				    snprintf(log_str_buf,LOG_STR_BUF_SIZE,"[FATAL %s][%s %s:%d]%s",log_time_stamp,file,func,(int)line,log_str_temp);
				}
		        break;
		    case LOG_LEVEL_INDISPENSABLE:
					log_need_record = 1;
				    snprintf(log_str_buf,LOG_STR_BUF_SIZE,"[INDISPENSABLE %s][%s %s:%d]%s",log_time_stamp,file,func,(int)line,log_str_temp);	
		        	break;
		}
		if ( 1 == log_need_record )
		{
		    fprintf(log_file_handle,"%s",log_str_buf);
		}
	}
	fflush(log_file_handle);
	pthread_mutex_unlock(&log_mutex);
	return 0;
}

/*****************************************************************************
 函 数 名  : creat_log_current_time_stamp
 功能描述  : 得到当前时间戳
 输入参数  : void
 输出参数  : 无
 返 回 值  : static void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void creat_log_current_time_stamp( void )
{
    struct tm* time_stamp;
	time_t cur_time;

	cur_time = time(NULL);
	time_stamp = localtime(&cur_time);

	snprintf(log_time_stamp, LOG_TIME_STAMP_SIZE,"%02d/%02d/%02d %02d:%02d:%02d", 
		time_stamp->tm_mday,time_stamp->tm_mon + 1, time_stamp->tm_year % 100,
		time_stamp->tm_hour, time_stamp->tm_min, time_stamp->tm_sec);
}

/*****************************************************************************
 函 数 名  : SRV_set_log_level
 功能描述  : 设置需要输出的日志信息等级
 输入参数  : int level
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月29日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
void SRV_set_log_level( int level )
{
    log_level = level;
}

/*****************************************************************************
 函 数 名  : SRV_log_close
 功能描述  : 关闭日志文件
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
void SRV_log_close( void )
{
    if(NULL != log_file_handle)
    {
    	LOG_STRING(LOG_LEVEL_INDISPENSABLE,"LOG CLOSE\n");
		fclose(log_file_handle);
    }
}

