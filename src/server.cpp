
#include <csignal>
#include "tcp_listener.h"
TcpListener  * kTcpListener; 
void signal_handler(int signo) {
	printf("^C pressed. Shutting down.\n");
	kTcpListener->stop(); 
	exit(0);
}

int main(int argc, char * argv[]){

	  std::signal(SIGINT, signal_handler);
	KNetLogIns.add_console(); 
	TcpListener tcpListener; 

	kTcpListener = & tcpListener; 
	tcpListener.start(8888); 

	return 0; 

} 
