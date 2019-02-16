
#1.日志系统
	日志打印
	DBG
	
#2.配置信息解析
	参数解析
	配置文件解析
	
#3.守护进程

#4.信号处理
	socketpair管道处理

#5.定时器
	链表管理超时连接

#6.epoll
	新连接封装

#7.线程池
	
	
	
#打开系统文件限制	

  1.修改/etc/profile 
	echo ulimit -n 65535 >>/etc/profile     
	source /etc/profile    	#加载修改后的profile  
	ulimit -n     			#显示65535，修改完毕！ 
  2.修改/etc/security/limits.conf (添加到文件底部)
	* soft nofile 65536 
	* hard nofile 65536 

   
