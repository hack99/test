#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>

class network_service
{
public:
	network_service(int worker_count = boost::thread::hardware_concurrency()) : 
		work_(new boost::asio::io_service::work(io_service_))
	{
		std::cout << "Create thread pool with " << worker_count << " workers" << std::endl;
		for (int i = 0; i < worker_count; i++)
			workers_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
	}

	virtual ~network_service()
	{	
		work_.reset();
		workers_.join_all();
	}
	
	boost::asio::io_service& io_service()
	{
		return io_service_;
	}

private:
	boost::asio::io_service io_service_;
	boost::thread_group workers_;
	boost::scoped_ptr<boost::asio::io_service::work> work_;
};

#endif // NETWORK_SERVICE_H
