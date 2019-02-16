#include "parse_config.h"

static char* getconfig( const char* option );
static FILE* getfilefp( void );
static void usage( char* argv[] );
static void version( void );

extern char* g_relpath;

/*****************************************************************************
 函 数 名  : SRV_getconfig
 功能描述  : 得到配置信息
 输入参数  : char* option
 输出参数  : 无
 返 回 值  : char*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月27日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
char* SRV_getconfig( const char* option )
{
    char* res=getconfig(option);
	if(NULL == res)
	{
		/*LOG*/
		printf("[%s %s %d] getconfig error.\n", __FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}
	return res;
}

/*****************************************************************************
 函 数 名  : getconfig
 功能描述  : 得到文件配置信息,内部函数提供接口
 输入参数  : char* option
 输出参数  : 无
 返 回 值  : char*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月27日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static char* getconfig( const char* option )
{
    static char info[CONFIG_INFO_SIZE];
	char config_file_buf[CONFIG_FILE_BUF_SIZE];
	char config_key_buf[CONFIG_KEY_BUF_SIZE];
	char* begin=NULL;
	char* equal=NULL;
	char* temp=NULL;
	FILE* fp = NULL;
	int find = 0;

	fp = getfilefp();
	if(NULL == fp)
	{
		printf("[%s %s %d] getfilefp error.\n", __FILE__, __FUNCTION__, __LINE__);
		exit(1);
	}

	while( NULL != fgets(config_file_buf,CONFIG_FILE_BUF_SIZE-1,fp))
	{
		/*serach option*/
		begin = config_file_buf;
		equal = strchr(config_file_buf,'=');
		if(NULL == equal)
		{
			continue;
		}

		while (isblank(*begin))
		{
		    begin++;
		}
		temp=begin;

		if(*temp == '#')
		{
			continue;
		}

		while(isalpha(*temp))
		{
		    temp++;
		}
		//temp--;
		strncpy(config_key_buf,begin,temp-begin);
		config_key_buf[temp-begin]='\0';

		if(0 != strcmp(config_key_buf,option))
		{
			continue;
		}
		find = 1;
		/*get option value*/
		equal++;
		while ( isblank(*equal))
		{
		    equal++;
		}
		begin = equal;

		while (!isblank(*equal) && *equal!='\n')
		{
		    equal++;
		}
		//equal--
		strncpy(info,begin,equal-begin);
		info[equal-begin]='\0';
		break;
	}
	if(NULL != fp)
	{
		fclose(fp);
	}
	if (0 == find)
	{
	    return NULL;
	}
	return info;
}

/*****************************************************************************
 函 数 名  : getfilefp
 功能描述  : 得到配置文件fp
 输入参数  : 无
 输出参数  : 无
 返 回 值  : static FILE*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月28日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static FILE* getfilefp( void )
{
	char file_path[CONFIG_PATH_SIZE];

	FILE* filefp = NULL;
	
	strcpy(file_path,g_relpath);

	strcat(file_path,"/config.ini");
	filefp = fopen(file_path,"r");
	return filefp;	
}

/*****************************************************************************
 函 数 名  : usage
 功能描述  : 输出有效的参数列表
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月27日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void usage( char* argv[] )
{
    fprintf(stderr,"usage:%s\n[-s sslport]\n[-d dossl]\n[-i isdaemon]\n[-p port]\n[-l log]\n[-v version]\n[-h help]\n",argv[0]);
	exit(-1);
}

/*****************************************************************************
 函 数 名  : void version
 功能描述  : 返回服务器版本信息
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月27日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
static void version( void )
{
	fprintf(stderr,"version:1.0\n"
				   "achieve:web server\n"
				   "service:provide GET POST function\n"
				   "and directory access and simple access control\n"
				   "secure:implement SSL secure connection\n"
				   "library:OPENSSL\n"
				   "author:chuan\n"
				   "time:2018/5/27\n");    
	exit(-1);
}

/*****************************************************************************
 函 数 名  : SRV_parse_option
 功能描述  : 解析用户输入的参数,若用户无输入,则使用默认配置参数
 输入参数  : int argc,char**argv,char*isdaemon,char**portp,char**logp,char**sslportp,char* dossl
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2018年5月27日
    作    者   : chuan
    修改内容   : 新生成函数

*****************************************************************************/
#ifdef HTTPS
void SRV_parse_option( int argc,char**argv,char*isdaemon,char**portp,char**logp,char**sslportp,char* dossl )
#else
void SRV_parse_option( int argc,char**argv,char*isdaemon,char**portp,char**logp )
#endif
{
	int opt;
	
	#ifdef HTTPS
	static char sslport[16];
	#endif
	static char port[16];
	static char log[64];

	struct option longopts[] =
	{ 
		#ifdef HTTPS
		{"sslport",1,NULL,'s'},  /*0->hasn't arg   1->has arg	2->can has or not*/
		{"dossl",0,NULL,'d'},
		#endif
		{"isdaemon",0,NULL,'i'},
		{"port",1,NULL,'p'},
		{"log",1,NULL,'l'},
		{"help",0,NULL,'h'},
		{"version",0,NULL,'v'},
		{0,0,0,0}	/*last must be a zero array*/
	};

	#ifdef HTTPS
	while(-1 != (opt = getopt_long(argc,argv,"s:dip:l:hv",longopts,NULL)) )
	#else
	while(-1 != (opt = getopt_long(argc,argv,"ip:l:hv",longopts,NULL)) )
	#endif
	{
		switch ( opt )
		{
			#ifdef HTTPS
		    case 's':
		        strncpy(sslport,optarg,sizeof(sslport)-1);
		        *sslportp = sslport;
		        break;
		    case 'd':
		        *dossl = 1;
		        break;
			#endif
		    case 'i':
		        *isdaemon = 1;
		        break;
		    case 'p':
		        strncpy(port,optarg,sizeof(port)-1);
		        *portp = port;
		        break;
		    case 'l':
		        strncpy(log,optarg,sizeof(log)-1);
		        *logp = log;
		        break;
		    case 'h':
		        usage(argv);
		        break;
		    case 'v':
		        version();
		        break;
		    case ':':
		        fprintf(stderr,"-%c:option needs a value.\n",optopt);
		        exit(1);
		        break;
		    case '?':
		    	//fprintf(stderr,"unknown option:%c\n",optopt);
		    	usage(argv);
		}
	}	
}

