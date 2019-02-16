CC 		=gcc
CCFLAGS =-Wall -g -o
OBJ 	=main.c parse_config.c log.c epoll_connect.c signal.c pthread_pool.c timer.c
OBJGEN 	=https-server
LIB 	=-lpthread -lssl -lcrypto

$(OBJGEN):$(OBJ)
	$(CC) $(CCFLAGS) $@ $^ $(LIB)
	
.PHONY:
clean:
	rm -f https-server server.log pid.file
