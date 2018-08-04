#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <functional>
#include <list>
#include <memory>
#include <thread>

#include <asio.hpp>

class network_service
{
public:
	network_service() : work_(new asio::io_service::work(io_service_))
	{
	}
	
	void init(int worker_count = std::thread::hardware_concurrency())
	{
		std::cout << "network_service::init with " << worker_count << " workers" << std::endl;
		for (int i = 0; i < worker_count; i++) {
			std::thread *thrd = new std::thread(
				std::bind(static_cast<std::size_t (asio::io_service::*)()>(&asio::io_service::run),
					&io_service_));
			workers_.push_back(thrd);
		}
	}

	void fini()
	{
		std::cout << "network_service::fini" << std::endl;
		work_.reset();
		std::for_each(workers_.begin(), workers_.end(), [](std::thread *worker){ worker->join(); delete worker;});
	}

	virtual ~network_service()
	{	
	}
	
	asio::io_service& io_service()
	{
		return io_service_;
	}

private:
	asio::io_service io_service_;
	std::list<std::thread*> workers_;
	std::unique_ptr<asio::io_service::work> work_;
};

#endif // NETWORK_SERVICE_H
