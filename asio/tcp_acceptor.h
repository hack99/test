#ifndef TCP_ACCEPTOR_H
#define TCP_ACCEPTOR_H

#include <boost/asio.hpp>

#include "handler_base.h"
#include "tcp_connection.h"

class tcp_acceptor
{
public:
	tcp_acceptor(boost::asio::io_service& io_service, handler_base& handler, int port) : io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), handler_(handler)
	{
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
	        boost::bind(&tcp_acceptor::handle_accept, this, new_connection,
	        boost::asio::placeholders::error));
	}	

	void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
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
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	handler_base& handler_;
};

#endif // TCP_ACCEPTOR_H
