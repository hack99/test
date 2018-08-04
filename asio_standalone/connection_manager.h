#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <set>

#include "tcp_connection.h"

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
{
public:
	/// Add the specified connection to the manager and start it.
	void add(tcp_connection::pointer c)
	{
		std::cout << "connection_manager::add" << std::endl;
		std::lock_guard<std::mutex> lock(mutex_);
		connections_.insert(c);
	}

	/// Stop the specified connection.
	void del(tcp_connection::pointer c)
	{
		std::cout << "connection_manager::del" << std::endl;
		std::lock_guard<std::mutex> lock(mutex_);
		connections_.erase(c);
	}

	/// Stop all connections.
	void close_all()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		std::cout << "connection_manager::close_all" << std::endl;
		for (std::set<tcp_connection::pointer >::iterator iter = connections_.begin(); iter != connections_.end(); iter++)
		{
			(*iter)->close();
		}
//		std::for_each(connections_.begin(), connections_.end(),
//			std::mem_fun_ref(static_cast<void (tcp_connection::*)()>(&tcp_connection::close)));
		//connections_.clear();
	}

private:
	/// The managed connections.
	std::set<tcp_connection::pointer > connections_;
	std::mutex mutex_;
};

#endif // HTTP_CONNECTION_MANAGER_HPP
