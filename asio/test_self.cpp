#include <iostream>

#include "tcp_connector.h"
#include "tcp_acceptor.h"
#include "handler_base.h"
#include "network_service.h"

class client_handler : public handler_base
{
public:
	virtual void on_connect(boost::shared_ptr<tcp_connection> connection)
	{
		std::cout << "client_handler::on_connect" << std::endl;
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
				std::cout << "content=";
				std::cout.write(buffer+head+2, msg_len-2) << std::endl;
				head += msg_len;
			}
		}
	}
};

class server_handler : public handler_base
{
public:
	virtual void on_connect(boost::shared_ptr<tcp_connection> connection)
	{
		std::cout << "server_handler::on_connect" << std::endl;
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

int main(int argc, char* argv[])
{

	if (argc != 2)
	{
		std::cerr << "Usage: network_test <port>" << std::endl;
		return 1;
	}

	try
	{
		network_service ns;
		client_handler ch;
		server_handler sh;

		tcp_acceptor server(ns.io_service(), sh, std::atoi(argv[1]));

		tcp::resolver resolver(ns.io_service());
		tcp::resolver::query query("localhost", argv[1]);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp_connector client(ns.io_service(), ch);
		boost::weak_ptr<tcp_connection> conn(client.connect(endpoint_iterator));

		ns.init();
		while (1)
		{
			char name[100];
			std::cin.getline (name , 100);
			if (strcmp(name, "stop") == 0)
			{
				std::cout << "Stop test" << std::endl;
				break;
			}

			boost::shared_ptr<msg_base> msg(new msg_base(strlen(name)+3));
			*msg << msg_base::length_holder() << (char*)name;
			conn.lock()->write(msg); 

		}
		conn.lock()->close();
		server.stop();
		ns.fini();

	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
