#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <netinet/in.h>

#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <liburing.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "logger.h"

#define MAX_QUEUE_DEPTH 32
enum UringEventType
{
	URING_EVENT_ACCEPT = 1,
	URING_EVENT_READ,
	URING_EVENT_WRITE,
};
using AsyncCallback = std::function<void(struct io_uring_cqe *)>;

enum NetEvent
{
	EVT_THREAD_INIT,
	EVT_THREAD_EXIT,
	EVT_LISTEN_START,
	EVT_LISTEN_FAIL,
	EVT_LISTEN_STOP,
	EVT_CREATE,
	EVT_RELEASE,
	EVT_CONNECT,
	EVT_CONNECT_FAIL,
	EVT_RECV,
	EVT_SEND,
	EVT_DISCONNECT,
	EVT_RECYLE_TIMER,
	EVT_USER1,
	EVT_USER2,
	EVT_END
};




struct UringRequest
{
	UringRequest(AsyncCallback cb = nullptr, int type = 0) : callback(cb), event_type(type) {}

	int iovec_count;
	int client_socket;
	std::function<void(struct io_uring_cqe *)> callback;
	int event_type;
	struct iovec iov[4];

};

class UringWorker
{
	public:
		UringWorker(){
			io_uring_queue_init(MAX_QUEUE_DEPTH, &ring, 0);
		}

		void stop(){
			io_uring_queue_exit(&ring);
		}


		virtual void init() {}
		virtual void deinit(){};

		struct io_uring_sqe *get_sqe()
		{
			return io_uring_get_sqe(&ring);
		}

		void submit_request(struct io_uring_sqe * sqe, UringRequest *req)
		{  
			io_uring_sqe_set_data(sqe, req);
			io_uring_submit(&ring);
		}

		void run()
		{

			std::call_once(init_flag, [&]() {
					this->init();
					});

			struct io_uring_cqe *cqe = nullptr;
			while (true)
			{
				int ret = io_uring_wait_cqe(&ring, &cqe);

				dlog("uring wait cqe is {}", ret); 
				auto req = (UringRequest *)cqe->user_data;
				if (ret < 0)
				{
					continue;
				}

				if (cqe->res < 0)
				{
					return;
				}

				req->callback(cqe);
				io_uring_cqe_seen(&ring, cqe);
				delete req;
			}

			std::call_once(deinit_flag, [&]() {
					this->deinit();
					});
		}

		std::once_flag init_flag;
		std::once_flag deinit_flag;

	private:
		struct io_uring ring;
};

using UringWorkerPtr = std::shared_ptr<UringWorker>;
