#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>	/*getopt_long*/
#include <ctype.h>
#include <time.h>  /*struct tm*/
#include <stdarg.h>
#include <pthread.h>
#include <sys/resource.h> /**/
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <semaphore.h> /*sem_t*/
#include <sys/epoll.h>
#include <signal.h>
#include <sys/types.h>     /*setsockopt*/
#include <sys/socket.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h> /*inet_ntoa*/


#ifndef HTTPS
#define HTTPS
#endif

typedef int BOOL;

#define FALSE	0
#define TRUE	(!FALSE)










