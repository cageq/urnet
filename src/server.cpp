
#include <csignal>
#include "tcp_listener.h"
#include "mysession.h"
TcpListener<MySession> *kTcpListener;
void signal_handler(int signo)
{
	printf("^C pressed. Shutting down.\n");
	kTcpListener->stop();
	exit(0);
}

int main(int argc, char *argv[])
{

	std::signal(SIGINT, signal_handler);
	KNetLogIns.add_console();
	TcpListener<MySession> tcpListener;

	kTcpListener = &tcpListener;
	tcpListener.start(9900);

	dlog("quit normally"); 
	return 0;
}
