#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <set>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include "tcp_connection.h"

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager : private boost::noncopyable
{
public:
	/// Add the specified connection to the manager and start it.
	void add(tcp_connection::pointer c)
	{
		std::cout << "connection_manager::add" << std::endl;
		boost::mutex::scoped_lock lock(mutex_);
		connections_.insert(c);
	}

	/// Stop the specified connection.
	void del(tcp_connection::pointer c)
	{
		std::cout << "connection_manager::del" << std::endl;
		boost::mutex::scoped_lock lock(mutex_);
		connections_.erase(c);
	}

	/// Stop all connections.
	void close_all()
	{
		boost::mutex::scoped_lock lock(mutex_);
		std::cout << "connection_manager::close_all" << std::endl;
		std::for_each(connections_.begin(), connections_.end(), boost::bind(&tcp_connection::close, _1));
		//connections_.clear();
	}

private:
	/// The managed connections.
	std::set<tcp_connection::pointer > connections_;
	boost::mutex mutex_;
};

#endif // HTTP_CONNECTION_MANAGER_HPP
