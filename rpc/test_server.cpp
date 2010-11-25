//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "msg_base.h"
#include "handler_base.h"
#include "tcp_connection.h"
#include "network_service.h"
#include "connection_manager.h"
#include "tcp_acceptor.h"
#include "service.pb.h"

class test_service_impl : public  test_service
{
  public:
    void test_call(::google::protobuf::RpcController* controller,
        const ::test_request* request,
        ::test_response* response,
        ::google::protobuf::Closure* done)
    {
      response->set_id (request->id ());
      response->set_message ("I'm the server. You sent me: " + request->message ());
    }
};

test_service_impl service;

class rpc_server_handler : public handler_base
{
public:
	virtual void on_connect(boost::shared_ptr<tcp_connection> connection)
	{
		connection_manager_.add(connection);
	}

	virtual void on_read(boost::shared_ptr<tcp_connection> connection)
	{
		char *buffer = connection->buffer();
		int& head = connection->buffer_head();
		int& tail = connection->buffer_tail();
		
		while (tail-head>2)
		{
			unsigned short msg_len = *(unsigned short*)(buffer+head);
			std::cout << "msg_len=" << msg_len << std::endl;
			if (msg_len <= tail-head)
			{
				std::stringstream ss (std::string (buffer+head+2, msg_len-2));
				test_request request;
				request.ParseFromIstream(&ss);
				handle_request(connection, request);
				head += msg_len;
			}
		}
	}
	
	virtual void on_close(boost::shared_ptr<tcp_connection> connection)
	{
		connection_manager_.del(connection);
	}

	void close_all()
	{
		connection_manager_.close_all();
	}
	
	virtual void handle_request(boost::shared_ptr<tcp_connection> connection, test_request& request)
	{
		test_response response;
		service.test_call(NULL, &request, &response, NULL);
		std::stringstream ss(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
		response.SerializeToOstream(&ss);

		boost::shared_ptr<msg_base> msg(new msg_base(200));
		*msg << (unsigned short)(response.ByteSize()+2);
		msg->write_data(ss.str().c_str(), response.ByteSize());
		connection->write(msg);
	}
protected:
	connection_manager connection_manager_;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>" << std::endl;
			return 1;
		}

		network_service ns;
		rpc_server_handler sh;
		ns.init();
		tcp_acceptor server(ns.io_service(), sh, std::atoi(argv[1]));
		while (1)
		{
			char name[100];
			std::cin.getline (name , 100);
			if (strcmp(name, "stop") == 0)
			{
				std::cout << "Stop" << std::endl;
				break;
			}
		}
		server.stop();
		sh.close_all();
		ns.fini();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
