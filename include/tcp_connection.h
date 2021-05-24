#pragma once
#include "uring_worker.h"

#define READ_BUFFER_SIZE 8192

class TcpConnection
{
	public:
		void init(int sock, UringWorkerPtr worker)
		{
			tcp_sock = sock;
			net_worker = worker;
			do_read();
		}
		int do_read()
		{
			auto sqe = net_worker->get_sqe();
			auto req = new UringRequest([this](struct io_uring_cqe *cqe) {
					if (cqe->res > 0)
					{
					dlog("received message length {}", cqe->res);
					auto req = (UringRequest *)cqe->user_data;
					handle_message((const char *)req->iov[0].iov_base, cqe->res);
					do_read();
					}
					else
					{
					handle_event(EVT_DISCONNECT);
					}
					});
			req->iov[0].iov_base = malloc(READ_BUFFER_SIZE);
			req->iov[0].iov_len = READ_BUFFER_SIZE;
			req->event_type = URING_EVENT_READ;

			memset(req->iov[0].iov_base, 0, READ_BUFFER_SIZE);
			/* Linux kernel 5.5 has support for readv, but not for recv() or read() */
			io_uring_prep_readv(sqe, tcp_sock, &req->iov[0], 1, 0);
			net_worker->submit_request(sqe, req);
			return 0;
		}



		int send(const char *data, uint32_t len)
		{
			auto req = new UringRequest([this](struct io_uring_cqe *cqe) {
					if (cqe->res > 0)
					{
					dlog("send out message length {}", cqe->res);
					}
					else
					{
					handle_event(EVT_DISCONNECT);
					}
					});

			req->event_type = URING_EVENT_WRITE;
			req->iov[0].iov_base = malloc(len); 
			memcpy(req->iov[0].iov_base, data, len);
			req->iov[0].iov_len = len;

			auto sqe = net_worker->get_sqe();
			io_uring_prep_writev(sqe, tcp_sock, req->iov, 1, 0);
			net_worker->submit_request(sqe, req);

			return len;
		}

		virtual int handle_message(const char *data, uint32_t len)
		{ 

			return 0;
		}
		virtual int handle_event(NetEvent evt)
		{

			return 0;
		}

		UringWorkerPtr net_worker;
		// std::mutex mutex;
		// std::string send_buffer;
		// std::string cache_buffer;
		// bool is_writing  = false;
		int tcp_sock;
};
