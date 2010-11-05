#ifndef TCP_CONNECTOR_H
#define TCP_CONNECTOR_H

#include <boost/asio.hpp>

#include "handler_base.h"
#include "tcp_connection.h"

class tcp_connector
{
public:
	tcp_connector(boost::asio::io_service& io_service, handler_base& handler) : io_service_(io_service), handler_(handler)
	{
	}

	virtual ~tcp_connector()
	{
	}

	tcp_connection::pointer connect(tcp::resolver::iterator endpoint_iterator)
	{
	    tcp_connection::pointer new_connection =
	        tcp_connection::create(io_service_, handler_);
	  
	    new_connection->connect(endpoint_iterator);
	  
	    return new_connection;
	}

	handler_base& get_handler()
	{
		return handler_;
	}
private:
	boost::asio::io_service& io_service_;
	handler_base& handler_;
};


#endif // TCP_CONNECTOR_H
