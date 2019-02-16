#include "global.h"

#define CONFIG_FILE_BUF_SIZE	256
#define CONFIG_KEY_BUF_SIZE		64
#define CONFIG_INFO_SIZE		64
#define CONFIG_PATH_SIZE		128

char* SRV_getconfig(const char* option);
#ifdef HTTPS
void SRV_parse_option( int argc,char**argv,char*isdaemon,char**portp,char**logp,char**sslportp,char* dossl );
#else
void SRV_parse_option( int argc,char**argv,char*isdaemon,char**portp,char**logp );
#endif




