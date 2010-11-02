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
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class tcp_client
{
	enum { max_read_length = 512 };
public:
	tcp_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
		: io_service_(io_service), socket_(io_service)
	{
		start_connect(endpoint_iterator);
	}

	void write(const char msg) // pass the write data to the do_write function via the io service in the other thread
	{
		bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
		write_msgs_.push_back(msg); // store in write buffer
		if (!write_in_progress) // if nothing is currently being written, then start
			start_write(); 
	} 

	void close() // call the do_close function via the io service in the other thread
	{
		io_service_.post(boost::bind(&tcp_client::handle_close, this));
	} 
private:
	void start_connect(tcp::resolver::iterator endpoint_iterator)
	{ 
		// asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
		tcp::endpoint endpoint = *endpoint_iterator;
		socket_.async_connect(endpoint,
			boost::bind(&tcp_client::handle_connect,
				this,
				boost::asio::placeholders::error,
				++endpoint_iterator)); 
	}

	void start_read(void)
	{ // Start an asynchronous read and call read_complete when it completes or fails
		socket_.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
			boost::bind(&tcp_client::handle_read,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	} 

	void start_write(void)
	{ // Start an asynchronous write and call write_complete when it completes or fails
		boost::asio::async_write(socket_,
			boost::asio::buffer(&write_msgs_.front(), 1),
			boost::bind(&tcp_client::handle_write,
				this,
				boost::asio::placeholders::error));
	}

	void handle_write(const boost::system::error_code& error)
	{ // the asynchronous read operation has now completed or failed and returned an error
		if (!error)
		{ // write completed, so send next write data
			write_msgs_.pop_front(); // remove the completed data
			if (!write_msgs_.empty()) // if there is anthing left to be written
				start_write(); // then start sending the next item in the buffer
		}
		else
			socket_.close();
	} 

	void handle_connect(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
	{ // the connection to the server has now completed or failed and returned an error
		if (!error) // success, so start waiting for read data
			start_read();
		else if (endpoint_iterator != tcp::resolver::iterator())
		{ // failed, so wait for another connection event
			socket_.close();
			start_connect(endpoint_iterator);
		}
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

	void handle_close()
	{
		std::cout << "handle_close" << std::endl;
		socket_.close();
	}
private:
	boost::asio::io_service& io_service_; // the main IO service that runs this connection
	tcp::socket socket_; // the socket this instance is connected to 
	char read_msg_[max_read_length]; // data read from the socket 
	std::deque<char> write_msgs_; // buffered write data 
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
	
		boost::asio::io_service io_service;
		
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], argv[2]);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	
		tcp_client c(io_service, endpoint_iterator);

		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service)); 

		for (;;)
		{
			char ch;
			std::cin.get(ch); // blocking wait for standard input
			//std::cout << "write " << ch << std::endl;
			if (ch == 3) // ctrl-C to end program
				break; 
			c.write(ch); 
			
		}
		
		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	return 0;
}

