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
		connection_manager_.add(connection);

		boost::shared_ptr<msg_base> msg(new msg_base(100));
		const std::string& daytime = make_daytime_string();
		*msg << msg_base::length_holder() << daytime;
		msg->dump();
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
	
	virtual void on_close(boost::shared_ptr<tcp_connection> connection)
	{
		connection_manager_.del(connection);
	}

	void close_all()
	{
		connection_manager_.close_all();
	}
protected:
	connection_manager connection_manager_;
};


template <class T>
class tcp_server
{
public:
	tcp_server(boost::asio::io_service& io_service, int port) : io_service_(io_service),
		acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
	{
		start_accept();
	}

	virtual ~tcp_server()
	{	
	}

	T& get_handler()
	{
		return handler_;
	}

	void stop()
	{
		acceptor_.close();
	}
private:
	void start_accept()
	{
		tcp_connection::pointer new_connection =
		tcp_connection::create(io_service_, &handler_);
		
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
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	T handler_;
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
		ns.init();
		tcp_server<server_handler> server(ns.io_service(), std::atoi(argv[1]));
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
		server.get_handler().close_all();
		ns.fini();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
