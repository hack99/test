#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>
#include <asio.hpp>

#include "msg_base.h"

using asio::ip::tcp;

class handler_base;

class tcp_connection
  : public std::enable_shared_from_this<tcp_connection>
{
	enum { max_read_length = 512 };
public:
  	using pointer = std::shared_ptr<tcp_connection>;

	virtual ~tcp_connection()
	{
		std::cout << "tcp_connection::~tcp_connection() " << std::endl;
	}

	static pointer create(asio::io_service& io_service, handler_base& handler)
	{
		return pointer(new tcp_connection(io_service, handler));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		std::cout << "tcp_connection::start" << std::endl;
		io_service_.dispatch(std::bind(&tcp_connection::start_read, shared_from_this()));
		io_service_.dispatch(std::bind(&handler_base::on_connect, std::ref(handler_), shared_from_this()));
	}

	void connect(tcp::resolver::iterator endpoint_iterator)
	{
		std::cout << "tcp_connection::connect" << std::endl;
		io_service_.dispatch(std::bind(&tcp_connection::start_connect, shared_from_this(), endpoint_iterator));
	}

	void write(std::shared_ptr<msg_base>& msg)
	{
		io_service_.dispatch(std::bind(&tcp_connection::enqueue_write, shared_from_this(), msg));
	}

	void close(const std::error_code& error)
	{
		// Dispatch to io_service thread.
		io_service_.dispatch(std::bind(&tcp_connection::handle_close, shared_from_this(), error));
	}
	
	void close()
	{
		std::cout << "tcp_conneciton::close" << std::endl;
		close(std::error_code(0, std::system_category()));
	}
	
	char* buffer() { return &buffer_[0]; }
	int& buffer_head() { return buffer_head_; }
	int& buffer_tail() { return buffer_tail_; }
private:
	tcp_connection(asio::io_service& io_service, handler_base& handler)
	: io_service_(io_service), socket_(io_service), handler_(handler), buffer_head_(0), buffer_tail_(0), stopped_(false)
	{
		std::cout << "tcp_connection::tcp_connection() " << std::endl;
	}

	void start_connect(tcp::resolver::iterator endpoint_iterator)
	{
		std::cout << "tcp_connection::start_connect" << std::endl;
		// asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
		tcp::endpoint endpoint = *endpoint_iterator;
		socket_.async_connect(endpoint,
			std::bind(&tcp_connection::handle_connect,
				shared_from_this(),
				std::placeholders::_1,
				++endpoint_iterator)); 
	}

	void handle_connect(const std::error_code& error, tcp::resolver::iterator endpoint_iterator)
	{ 
		std::cout << "tcp_connection::handle_connect" << std::endl;
		// the connection to the server has now completed or failed and returned an error
		if (!error) // success, so start waiting for read data
		{
			start();
		}
		else if (endpoint_iterator != tcp::resolver::iterator())
		{ // failed, so wait for another connection event
			socket_.close();
			start_connect(endpoint_iterator);
		}
	} 

	void start_read(void)
	{ 
		std::cout << "tcp_connection::start_read" << std::endl;
		if (stopped_)
			return;
		// Start an asynchronous read and call read_complete when it completes or fails
		socket_.async_read_some(asio::buffer(buffer_+buffer_head_, max_read_length-buffer_tail_),
			std::bind(&tcp_connection::handle_read,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2));
	} 

	void enqueue_write(std::shared_ptr<msg_base>& msg)
	{
		std::lock_guard<std::mutex> lock(mutex_); 
		bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
		write_msgs_.push_back(msg);
		if (!write_in_progress)
			start_write();
	}

	void start_write()
	{
		if (stopped_)
			return;
		std::shared_ptr<msg_base>& msg = write_msgs_.front();
		asio::async_write(socket_, asio::buffer(msg->buffer()+msg->rd_ptr(), msg->wr_ptr()),
		std::bind(&tcp_connection::handle_write, shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2));
	}

	void handle_read(const std::error_code& error, size_t bytes_transferred)
	{ // the asynchronous read operation has now completed or failed and returned an error
		if (!error)
		{ // read completed, so process the data
			std::cout << "tcp_connection::handle_read " << bytes_transferred << std::endl;
			buffer_tail_ += bytes_transferred;
			//std::cout << "head:" << buffer_head_ << " tail:" << buffer_tail_ << " new:" <<  bytes_transferred << std::endl;
			handler_.on_read(shared_from_this());
			if (buffer_head_ > 0)
			{
				if (buffer_tail_-buffer_head_ > 0)
				{
					memmove(buffer_, buffer_+buffer_head_, buffer_tail_-buffer_head_);
				}
				buffer_tail_ -= buffer_head_;
				buffer_head_ = 0;
			}
			//std::cout << "head:" << buffer_head_ << " tail:" << buffer_tail_ << std::endl;
			//cout << "\n";
			start_read(); // start waiting for another asynchronous read again
		}
		else
		{
			std::cerr << "tcp_connection::handle_read " << error.message() << std::endl;
			close(error);
		}
	} 

	void handle_write(const std::error_code& error, size_t bytes_transferred)
	{
		if (!error)
		{
			std::cout << "tcp_connection::handle_write " << bytes_transferred << std::endl;
			std::lock_guard<std::mutex> lock(mutex_); 
			write_msgs_.pop_front(); // remove the completed data
			if (!write_msgs_.empty()) // if there is anthing left to be written
				start_write(); // then start sending the next item in the buffer
			handler_.on_write(shared_from_this());
		}
		else
		{
			std::cerr << "tcp_connection::handle_write " << error.message() << std::endl;
			close(error);
		}
	}

	void handle_close(const std::error_code& error)
	{
		if (stopped_)
			return;

		stopped_ = true;
		std::cout << "tcp_connection::handle_close" << std::endl;
		// Initiate graceful service_handler closure.
		std::error_code ignored_ec;
		socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
		socket_.close();

		handler_.on_close(shared_from_this());
	}

	asio::io_service& io_service_;
	tcp::socket socket_;
	std::mutex mutex_; 
	std::deque<std::shared_ptr<msg_base> > write_msgs_;
	handler_base& handler_;
	char buffer_[max_read_length]; // data read from the socket 
	int buffer_head_;
	int buffer_tail_;
	bool stopped_;
};


#endif // TCP_CONNECTION_H
