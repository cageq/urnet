#pragma once
#include "uring_worker.h"

class TcpListener
{

public:
	bool start(int16_t port)
	{

		listen_sock = socket(PF_INET, SOCK_STREAM, 0);
		int enable = 1;
		int ret = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
		if (ret < 0)
		{
			elog("set socket reuse failed");
			return false;
		}
		struct sockaddr_in bindAddr;
		bindAddr.sin_family = AF_INET;
		bindAddr.sin_port = htons(port);
		bindAddr.sin_addr.s_addr = htonl(INADDR_ANY); //opt.host

		ret = bind(listen_sock, (const struct sockaddr *)&bindAddr, sizeof(bindAddr));
		if (ret < 0)
		{ 
			return false;
		}

		ret = listen(listen_sock, 128);
		if (ret < 0)
		{

			return false;
		}

		do_accept(); 
		net_worker.run(); 
		return true;
	}

	int do_accept()
	{ 
		struct sockaddr_in cliAddr;
        socklen_t cliAddrLen = sizeof(cliAddr); 
		struct io_uring_sqe *sqe = net_worker.get_sqe();
		io_uring_prep_accept(sqe, listen_sock, (struct sockaddr *)&cliAddr, &cliAddrLen, 0);
		
		auto req = new UringRequest([this](struct io_uring_cqe *cqe){
			dlog("on accept new connection ");  

			do_accept(); 
		}, URING_EVENT_ACCEPT ); 

		net_worker.submit_request(req);  
		return 0;
	}

private:
	UringWorker net_worker;
	int listen_sock;
};
