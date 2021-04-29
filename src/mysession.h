#pragma once 
#include "tcp_connection.h"


class MySession : public TcpConnection {

public: 

  virtual int handle_message(const char *data, uint32_t len)
    {

        dlog("handle message {}: {}", len, data);
        this->send(data, len); 
        return 0;
    }

}; 