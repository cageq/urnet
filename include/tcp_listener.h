#pragma once
#include "uring_worker.h" 

template <class T> 
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
			struct sockaddr_in bindAddr = {0};
			bindAddr.sin_family = AF_INET;
			bindAddr.sin_port = htons(port);
			bindAddr.sin_addr.s_addr = htonl(INADDR_ANY); //opt.host
			ret = bind(listen_sock, (const struct sockaddr *)&bindAddr, sizeof(bindAddr));
			if (ret < 0)
			{ 
				elog("bind error"); 
				return false;
			}

			ret = listen(listen_sock, 10);
			if (ret < 0)
			{
				elog("listen error "); 
				return false;
			}
			net_worker = std::make_shared<UringWorker>(); 

		 	do_accept(); 
			net_worker->run(); 
			return true;
		}

		void stop(){

			net_worker->stop(); 

		}

		int do_accept()
		{ 
			struct sockaddr_in cliAddr;
			socklen_t cliAddrLen = sizeof(cliAddr); 
			struct io_uring_sqe *sqe = net_worker->get_sqe();
			io_uring_prep_accept(sqe, listen_sock, (struct sockaddr *)&cliAddr, &cliAddrLen, 0);

			auto req = new UringRequest([this](struct io_uring_cqe *cqe){
					dlog("on accept new connection ");  
					
					auto conn =  new T(); 

					conn->init(cqe->res, this->net_worker); 


					do_accept(); 
					}, URING_EVENT_ACCEPT ); 

			net_worker->submit_request(sqe, req);  
			return 0;
		}

		void do_read(){
			
		}

	private:
		UringWorkerPtr net_worker;
		int listen_sock;
};
