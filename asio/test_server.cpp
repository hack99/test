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
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
	enum { max_read_length = 512 };
public:
  	typedef boost::shared_ptr<tcp_connection> pointer;

	virtual ~tcp_connection()
	{
		std::cout << "tcp_connection::~tcp_connection() " << sn_ << std::endl;
	}

	static pointer create(boost::asio::io_service& io_service)
	{
		return pointer(new tcp_connection(io_service));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		message_ = make_daytime_string();

		start_write();
	
		start_read();
	}

private:
	tcp_connection(boost::asio::io_service& io_service)
	: socket_(io_service), sn_(count_++)
	{
		std::cout << "tcp_connection::tcp_connection() " << sn_ << std::endl;
	}

	void start_read(void)
	{ // Start an asynchronous read and call read_complete when it completes or fails
		socket_.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
			boost::bind(&tcp_connection::handle_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	} 

	void start_write()
	{
		boost::asio::async_write(socket_, boost::asio::buffer(message_),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
	{ // the asynchronous read operation has now completed or failed and returned an error
		if (!error)
		{ // read completed, so process the data
			std::cout.write(read_msg_, bytes_transferred); // echo to standard output
			//cout << "\n";
			start_read(); // start waiting for another asynchronous read again
		}
		else
			socket_.close();
	} 

	void handle_write(const boost::system::error_code& error,
	size_t bytes_transferred)
	{
		if (!error)
		{
			std::cout << sn_ << " handle_write " << bytes_transferred << std::endl;
		}
	}

	tcp::socket socket_;
	std::string message_;
	int sn_;
	static int count_;
	char read_msg_[max_read_length]; // data read from the socket 

};

int tcp_connection::count_ = 0;

class tcp_server
{
public:
	tcp_server(boost::asio::io_service& io_service, int port, int worker_count = boost::thread::hardware_concurrency())
: 		acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		work_(new boost::asio::io_service::work(io_service))
	{
		start_accept();
		std::cout << "Create thread pool with " << worker_count << " workers" << std::endl;
		for (int i = 0; i < worker_count; i++)
			workers_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
	}

	virtual ~tcp_server()
	{	
		work_.reset();
		workers_.join_all();
	}
private:
	void start_accept()
	{
		tcp_connection::pointer new_connection =
		tcp_connection::create(acceptor_.io_service());
		
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
	boost::thread_group workers_;
	boost::scoped_ptr<boost::asio::io_service::work> work_; 
};

int main()
{
	try
	{
		boost::asio::io_service io_service;
		tcp_server server(io_service, 12345);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}
