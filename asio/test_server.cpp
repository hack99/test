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

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class server_handler : public handler_base
{
public:
	virtual void on_connect(boost::shared_ptr<tcp_connection> connection)
	{
		boost::shared_ptr<msg_base> msg(new msg_base(100));
		const std::string& daytime = make_daytime_string();
		*msg << (short)(daytime.length()+3) << daytime;
		connection->write(msg);
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
				boost::shared_ptr<msg_base> msg(new msg_base(msg_len));
				msg->write_data(buffer+head, msg_len);
				connection->write(msg);
				head += msg_len;
			}
		}
	}
};


template <class T>
class tcp_server
{
public:
	tcp_server(boost::asio::io_service& io_service, int port) : 
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		start_accept();
	}

	virtual ~tcp_server()
	{	
	}
private:
	void start_accept()
	{
		tcp_connection::pointer new_connection =
		tcp_connection::create(acceptor_.io_service(), &handler_);
		
		acceptor_.async_accept(new_connection->socket(),
		boost::bind(&tcp_server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
	}
	
	void handle_accept(tcp_connection::pointer new_connection,
	const boost::system::error_code& error)
	{
		if (!error)
		{
			new_connection->start();
			start_accept();
		}
	}

private:
	tcp::acceptor acceptor_;
	T handler_;
};

int main()
{
	try
	{
		network_service net_service;
		tcp_server<server_handler> server(net_service.io_service(), 12345);
		while (1)
		{
			char ch;
			std::cin.get(ch); // blocking wait for standard input
			if (ch == 3) // ctrl-C to end program
				break;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
