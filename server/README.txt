1.��־ϵͳ
	��־��ӡ
	DBG
2.������Ϣ����
	��������
	�����ļ�����
3.�ػ�����
4.�źŴ���
	socketpair�ܵ�����
5.��ʱ��
	�������ʱ����
5.epoll
	�����ӷ�װ
6.�̳߳�
	
	
	
#��ϵͳ�ļ�����	

  1.�޸�/etc/profile 
	echo ulimit -n 65535 >>/etc/profile     
	source /etc/profile    	#�����޸ĺ��profile  
	ulimit -n     			#��ʾ65535���޸���ϣ� 
  2.�޸�/etc/security/limits.conf (��ӵ��ļ��ײ�)
	* soft nofile 65536 
	* hard nofile 65536 

   