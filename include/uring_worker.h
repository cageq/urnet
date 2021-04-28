#pragma once
#include <liburing.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include "logger.h"

#define MAX_QUEUE_DEPTH 256
enum UringEventType
{
	URING_EVENT_ACCEPT = 1,
	URING_EVENT_READ,
	URING_EVENT_WRITE,
};
using AsyncCallback = std::function<void(struct io_uring_cqe *)>;

struct UringRequest
{
	UringRequest(AsyncCallback cb = nullptr, int type = 0) : callback(cb), event_type(type) {}
	
	int iovec_count;
	int client_socket;
	struct iovec iov[16];
	std::function<void(struct io_uring_cqe *)> callback;
	int event_type;
};

class UringWorker
{
public:
	void start(){
		io_uring_queue_init(MAX_QUEUE_DEPTH, &ring, 0);

	}
	virtual void init() {}
	virtual void deinit(){};

	struct io_uring_sqe *get_sqe()
	{
		return io_uring_get_sqe(&ring);
	}

	void submit_request(UringRequest *req)
	{ 
		auto sqe = io_uring_get_sqe(&ring);
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
			delete req;

			io_uring_cqe_seen(&ring, cqe);
		}

		std::call_once(deinit_flag, [&]() {
			this->deinit();
		});
	}

	std::once_flag init_flag;
	std::once_flag deinit_flag;

private:
	io_uring ring;
};

using UringWorkerPtr = std::shared_ptr<UringWorker>;
