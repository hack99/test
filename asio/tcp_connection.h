#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class handler_base;

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
	enum { max_read_length = 512 };
public:
  	typedef boost::shared_ptr<tcp_connection> pointer;

	virtual ~tcp_connection()
	{
		std::cout << "tcp_connection::~tcp_connection() " << std::endl;
	}

	static pointer create(boost::asio::io_service& io_service, handler_base* handler)
	{
		return pointer(new tcp_connection(io_service, handler));
	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		std::cout << "start" << std::endl;
		io_service_.dispatch(boost::bind(&tcp_connection::start_read, shared_from_this()));
		io_service_.dispatch(boost::bind(&handler_base::on_connect, handler_, shared_from_this()));
	}

	void connect(tcp::resolver::iterator endpoint_iterator)
	{
		std::cout << "connect" << std::endl;
		io_service_.dispatch(boost::bind(&tcp_connection::start_connect, shared_from_this(), endpoint_iterator));
	}

	void write(boost::shared_ptr<msg_base>& msg)
	{
		io_service_.dispatch(boost::bind(&tcp_connection::enqueue_write, shared_from_this(), msg));
	}

	void close(const boost::system::error_code& error)
	{
		// Dispatch to io_service thread.
		io_service_.dispatch(boost::bind(&tcp_connection::handle_close, shared_from_this(), error));
	}
	
	void close()
	{
		close(boost::system::error_code(0, boost::system::get_system_category()));
	}
	
	char* buffer() { return &buffer_[0]; }
	int& buffer_head() { return buffer_head_; }
	int& buffer_tail() { return buffer_tail_; }
private:
	tcp_connection(boost::asio::io_service& io_service, handler_base *handler)
	: io_service_(io_service), socket_(io_service), handler_(handler), buffer_head_(0), buffer_tail_(0)
	{
		std::cout << "tcp_connection::tcp_connection() " << std::endl;
	}

	void start_connect(tcp::resolver::iterator endpoint_iterator)
	{
		std::cout << "start_connect" << std::endl;
		// asynchronously connect a socket to the specified remote endpoint and call connect_complete when it completes or fails
		tcp::endpoint endpoint = *endpoint_iterator;
		socket_.async_connect(endpoint,
			boost::bind(&tcp_connection::handle_connect,
				shared_from_this(),
				boost::asio::placeholders::error,
				++endpoint_iterator)); 
	}

	void handle_connect(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
	{ 
		std::cout << "handle_connect" << std::endl;
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
	{ // Start an asynchronous read and call read_complete when it completes or fails
		socket_.async_read_some(boost::asio::buffer(buffer_+buffer_head_, max_read_length-buffer_tail_),
			boost::bind(&tcp_connection::handle_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	} 

	void enqueue_write(boost::shared_ptr<msg_base>& msg)
	{
		boost::lock_guard<boost::mutex> lock(mutex_); 
		bool write_in_progress = !write_msgs_.empty(); // is there anything currently being written?
		write_msgs_.push_back(msg);
		if (!write_in_progress)
			start_write();
	}

	void start_write()
	{
		boost::shared_ptr<msg_base>& msg = write_msgs_.front();
		boost::asio::async_write(socket_, boost::asio::buffer(msg->buffer()+msg->rd_ptr(), msg->wr_ptr()),
		boost::bind(&tcp_connection::handle_write, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred)
	{ // the asynchronous read operation has now completed or failed and returned an error
		if (!error)
		{ // read completed, so process the data
			std::cout << "handle_read " << bytes_transferred << std::endl;
			buffer_tail_ += bytes_transferred;
			//std::cout << "head:" << buffer_head_ << " tail:" << buffer_tail_ << " new:" <<  bytes_transferred << std::endl;
			handler_->on_read(shared_from_this());
			if (buffer_head_ > 0)
			{
				if (buffer_tail_-buffer_head_ > 0)
				{
					memmove(buffer_, buffer_+buffer_head_, buffer_tail_-buffer_head_);
				}
				buffer_tail_ -= buffer_head_;
				buffer_head_ = 0;
			}
			std::cout << "head:" << buffer_head_ << " tail:" << buffer_tail_ << std::endl;
			//cout << "\n";
			start_read(); // start waiting for another asynchronous read again
		}
		else
			close(error);
	} 

	void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
	{
		if (!error)
		{
			std::cout << "handle_write " << bytes_transferred << std::endl;
			boost::lock_guard<boost::mutex> lock(mutex_); 
			write_msgs_.pop_front(); // remove the completed data
			if (!write_msgs_.empty()) // if there is anthing left to be written
				start_write(); // then start sending the next item in the buffer
			handler_->on_write(shared_from_this());
		}
		else
			close(error);
	}

	void handle_close(const boost::system::error_code& error)
	{
		std::cout << "handle_close" << std::endl;
		// Initiate graceful service_handler closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
		socket_.close();

		handler_->on_close(shared_from_this());
	}

	boost::asio::io_service& io_service_;
	tcp::socket socket_;
	boost::mutex mutex_; 
	std::deque<boost::shared_ptr<msg_base> > write_msgs_;
	handler_base* handler_;
	char buffer_[max_read_length]; // data read from the socket 
	int buffer_head_;
	int buffer_tail_;
};


#endif // TCP_CONNECTION_H
