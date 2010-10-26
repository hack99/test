#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
class printer
{
public:
	printer(boost::asio::io_service& io, int sec)
		: timer_(io, boost::posix_time::seconds(sec)),
		count_(0),
		sec_(sec)
	{
		timer_.async_wait(boost::bind(&printer::print, this));
	}
	~printer()
	{
		std::cout << boost::date_time::microsec_clock< boost::posix_time::ptime >::universal_time() << " " << "Final count is " << sec_ << ":" << count_ << std::endl;
	}
	void print()
	{
		if (count_ < 5)
		{
			std::cout << boost::date_time::microsec_clock< boost::posix_time::ptime >::universal_time() << " " << sec_ << ":" << count_ << std::endl;
			++count_;
			timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(sec_));
			timer_.async_wait(boost::bind(&printer::print, this));
		}
	}
private:
	boost::asio::deadline_timer timer_;
	int count_;
	int sec_;
};
int main()
{
	boost::asio::io_service io;
	printer p1(io, 1), p2(io, 2), p3(io, 3);
	io.run();
	return 0;
}

