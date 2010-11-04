//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

#include "msg_base.h"
#include "handler_base.h"
#include "tcp_connection.h"
#include "network_service.h"

class client_handler : public handler_base
{
public:
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
				std::cout << "content=";
				std::cout.write(buffer+head+2, msg_len-2) << std::endl;
				head += msg_len;
			}
		}
	}
};


template <class T>
class tcp_client
{
public:
	tcp_client(boost::asio::io_service& io_service)
		: io_service_(io_service)
	{
	}

	tcp_connection::pointer connect(tcp::resolver::iterator endpoint_iterator)
	{
		tcp_connection::pointer new_connection =
		tcp_connection::create(io_service_, &handler_);

		new_connection->connect(endpoint_iterator);
		
		return new_connection;
	}

private:
	boost::asio::io_service& io_service_;
	T handler_;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>" << std::endl;
			return 1;
		}
	
		network_service net_service;
		
		tcp::resolver resolver(net_service.io_service());
		tcp::resolver::query query(argv[1], argv[2]);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	
		tcp_client<client_handler> c(net_service.io_service());
		tcp_connection::pointer conn = c.connect(endpoint_iterator);

		for (;;)
		{
			char name[100];
			std::cin.getline (name , 100);
			boost::shared_ptr<msg_base> msg(new msg_base(strlen(name)+3));
			*msg << msg_base::length_holder() << (char*)name;
			conn->write(msg); 
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}

