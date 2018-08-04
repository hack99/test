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
#include <memory>
#include <asio.hpp>

using asio::ip::tcp;

#include "msg_base.h"
#include "handler_base.h"
#include "tcp_connection.h"
#include "network_service.h"
#include "connection_manager.h"
#include "tcp_connector.h"

class client_handler : public handler_base
{
public:
	virtual void on_connect(std::shared_ptr<tcp_connection> connection)
	{
		std::cout << "client_handler::on_connect" << std::endl;
		connection_manager_.add(connection);
	}

	virtual void on_read(std::shared_ptr<tcp_connection> connection)
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
	virtual void on_close(std::shared_ptr<tcp_connection> connection)
	{
		connection_manager_.del(connection);
	}
	void close_all()
	{
		std::cout << "client_handler::close_all" << std::endl;
		connection_manager_.close_all();
	}
protected:
	connection_manager connection_manager_;
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
	
		network_service ns;
		client_handler ch;
		
		ns.init();

		tcp::resolver resolver(ns.io_service());
		tcp::resolver::query query(argv[1], argv[2]);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	
		tcp_connector client(ns.io_service(), ch);
		std::weak_ptr<tcp_connection> conn(client.connect(endpoint_iterator));

		while (1)
		{
			char name[100];
			std::cin.getline (name , 100);
			if (strcmp(name, "stop") == 0)
			{
				std::cout << "Stop" << std::endl;
				break;
			}
			std::shared_ptr<msg_base> msg(new msg_base(strlen(name)+3));
			*msg << msg_base::length_holder() << (char*)name;
			tcp_connection::pointer conn_ptr = conn.lock();
			if (!conn_ptr) break;
			conn_ptr->write(msg); 
		}
		ch.close_all();
		ns.fini();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}

