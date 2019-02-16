#include"global.h"

#define LOG_FILE_NAME_SIZE	128
#define LOG_STR_BUF_SIZE	256
#define LOG_TIME_STAMP_SIZE	128

#define LOG_LEVEL_INFO			(0x00)
#define LOG_LEVEL_WARNING		(0x01)
#define LOG_LEVEL_ERROR			(0x02)
#define LOG_LEVEL_FATAL			(0x03)
#define LOG_LEVEL_INDISPENSABLE	(0x04)	//To display several indispensable information.

void SRV_set_log_name( char* log_name );
int SRV_log_init( void );
int log_string( int level,const char* file,const char* func,long line,char* format,... );
void SRV_set_log_level( int level );
void SRV_log_close( void );

/*__VA_ARGS__  variable parameter*/
#define LOG_STRING(a,...)	log_string(a,__FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)

#define DEBUG (1)
#if	DEBUG
	#define DBG_PRINT(a,...)	log_string(a,__FILE__, __FUNCTION__, __LINE__,__VA_ARGS__)
#else
	#define DBG_PRINT(...)	do{} while(FALSE)
#endif

