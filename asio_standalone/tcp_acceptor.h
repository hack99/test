#ifndef TCP_ACCEPTOR_H
#define TCP_ACCEPTOR_H

#include <functional>
#include <system_error>
#include <asio.hpp>

#include "handler_base.h"
#include "tcp_connection.h"

using asio::ip::tcp;

class tcp_acceptor
{
public:
	tcp_acceptor(asio::io_service& io_service, handler_base& handler, int port) : io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(asio::ip::tcp::v4(), port)), handler_(handler)
	{
		acceptor_.set_option(asio::ip::tcp::no_delay(true));
		acceptor_.set_option(asio::socket_base::keep_alive(true));
		acceptor_.set_option(asio::socket_base::enable_connection_aborted(true));
		start_accept();
	}

	virtual ~tcp_acceptor()
	{	
	}

	handler_base& get_handler()
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
	    std::cout << "tcp_acceptor::start_accept" << std::endl;
	  
	    tcp_connection::pointer new_connection =
	        tcp_connection::create(io_service_, handler_);
	  
	    acceptor_.async_accept(new_connection->socket(),
	        std::bind(&tcp_acceptor::handle_accept, this, new_connection,
	        std::placeholders::_1));
	}	

	void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error)
	{
	  
	    std::cout << "tcp_acceptor::handle_accept" << std::endl;
	  
	    if (!error)
	    {
	        new_connection->start();
	        start_accept();
	    }
	    else
	    {
	        std::cerr << error.message() << std::endl;
	    }
	}	

private:
	asio::io_service& io_service_;
	asio::ip::tcp::acceptor acceptor_;
	handler_base& handler_;
};

#endif // TCP_ACCEPTOR_H
