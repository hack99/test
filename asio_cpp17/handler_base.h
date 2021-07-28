#ifndef HANDLER_BASE_H
#define HANDLER_BASE_H

#include <memory>

class tcp_connection;

class handler_base
{
public:
	handler_base()
	{
	}
	virtual ~handler_base()
	{
	}
	virtual void on_connect(std::shared_ptr<tcp_connection> connection)
	{
		std::cout << "handler_base::on_connect" << std::endl;
	}
	virtual void on_read(std::shared_ptr<tcp_connection> connection)
	{
		std::cout << "handler_base::on_read" << std::endl;
	}
	virtual void on_write(std::shared_ptr<tcp_connection> connection)
	{
		std::cout << "handler_base::on_write" << std::endl;
	}
	virtual void on_close(std::shared_ptr<tcp_connection> connection)
	{
		std::cout << "handler_base::on_close" << std::endl;
	}
};

#endif // HANDLER_BASE_H
