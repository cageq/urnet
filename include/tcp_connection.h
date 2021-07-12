#pragma once
#include "uring_worker.h"

#define READ_BUFFER_SIZE 8192

class TcpConnection
{
public:
	enum { kReadBufferSize = 1024 * 8, kMaxPackageLimit = 8 * 1024  };
		void init(int sock, const UringWorkerPtr&  worker)
		{
			tcp_sock = sock;
			net_worker = worker;
			do_read();
		}
		virtual ~TcpConnection(){

			dlog("destroy tcp connection"); 
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


	virtual bool handle_data(const char * , uint32_t len )
	{
		return true; 
	}

	virtual int32_t handle_package(const char * data, uint32_t len ){
		return len  ; 
	}

	bool process_data(uint32_t nread) {
		if ( nread <= 0) {
			return false;
		}
		this->handle_event(EVT_RECV);				
		read_buffer_pos += nread; 
		int32_t pkgLen = this->handle_package ((char*)read_buffer, read_buffer_pos); 

		//package size is larger than data we have 
		if (pkgLen >  read_buffer_pos){
			if (pkgLen > kReadBufferSize) {
				elog("single package size {} exceeds max buffer size ({}) , please increase it", pkgLen, kReadBufferSize);
				this->close();
				return false;
			}
			dlog("need more data to get one package"); 
			return true; 
		}

		int32_t readPos = 0;
		while (pkgLen > 0) {
			dlog("process data size {} ,read buffer pos {}  readPos {}", pkgLen, read_buffer_pos, readPos);
			if (readPos + pkgLen <= read_buffer_pos) {
				char* pkgEnd = (char*)read_buffer + readPos + pkgLen + 1;
				char endChar = *pkgEnd;
				*pkgEnd = 0;
				this->handle_data((const char*)read_buffer + readPos, pkgLen);
				*pkgEnd = endChar;
				readPos += pkgLen;
			} else {
				if (read_buffer_pos > readPos )
				{
					rewind_buffer(readPos);
					break;
				} 
			}

			if (readPos < read_buffer_pos) {
				int32_t  dataLen = read_buffer_pos - readPos; 
				pkgLen = this->handle_package( (char*)read_buffer + readPos, dataLen); 	
				if (pkgLen <= 0 ||  pkgLen > dataLen) {

					if (pkgLen > kReadBufferSize) {
						elog("single package size {} exceeds max buffer size ({}) , please increase it", pkgLen, kReadBufferSize);
						this->close();
						return false; 
					}

					rewind_buffer(readPos);
					break;
				}  

			}else {
				read_buffer_pos = 0;
				break;
			}
		} 
		return true; 
	}


	void close(){
	}

	void rewind_buffer(int32_t readPos){
		if (read_buffer_pos >= readPos) {
			dlog("rewind buffer to front {} ", read_buffer_pos - readPos);
			memmove(read_buffer, (const char*)read_buffer + readPos, read_buffer_pos - readPos);
			read_buffer_pos -= readPos;
		}
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
	char read_buffer[kReadBufferSize+4];
	int32_t read_buffer_pos = 0;
		// std::mutex mutex;
		// std::string send_buffer;
		// std::string cache_buffer;
		// bool is_writing  = false;
		int tcp_sock;
};
