#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>

class network_service
{
public:
	network_service() : work_(new boost::asio::io_service::work(io_service_))
	{
	}
	
	void init(int worker_count = boost::thread::hardware_concurrency())
	{
		std::cout << "network_service::init with " << worker_count << " workers" << std::endl;
		for (int i = 0; i < worker_count; i++)
			workers_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
	}

	void fini()
	{
		std::cout << "network_service::fini" << std::endl;
		work_.reset();
		workers_.join_all();
	}

	virtual ~network_service()
	{	
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
