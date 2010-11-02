#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/functional/factory.hpp>
#include <boost/any.hpp>

class object_base
{
public:
	virtual ~object_base()
	{
	}
};

class timer_base : public object_base
{
public:
	timer_base(boost::asio::io_service& io) : timer_(io)
	{
		std::cout << "timer_base::timer_base()" << std::endl;
	}
	virtual ~timer_base()
	{
		std::cout << "timer_base::~timer_base()" << std::endl;
	}
protected:
	boost::asio::deadline_timer timer_;
};

class printer : public timer_base
{
public:
	printer(boost::asio::io_service& io) : timer_base(io), count_(0)
	{
		std::cout << "printer::printer()" << std::endl;
	}
	void schedule(int sec)
	{
		sec_ = sec;
		timer_.expires_from_now(boost::posix_time::seconds(sec_));
		timer_.async_wait(boost::bind(&printer::print, this));
	}
	virtual ~printer()
	{
		std::cout << "printer::~printer()" << std::endl;
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
	int count_;
	int sec_;
};

class object_factory
{
	typedef boost::function< boost::shared_ptr<object_base>() > a_factory;
public:
	void register_factory(const std::string& classname, a_factory factory)
	{
		m_factories[classname] = factory;
	}
	boost::shared_ptr<object_base> create(const std::string& classname)
	{
		return m_factories[classname]();
	}
protected:
	std::map<std::string, a_factory> m_factories;
};

int main()
{
	object_factory of;
	boost::asio::io_service io;
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io));
	of.register_factory("printer", boost::bind(boost::factory<boost::shared_ptr<printer> >(), boost::ref(io)));
	boost::shared_ptr<printer> p1 = boost::dynamic_pointer_cast<printer, object_base>(of.create("printer"));
	boost::shared_ptr<printer> p2 = boost::dynamic_pointer_cast<printer, object_base>(of.create("printer"));
	//factories["printer"] = boost::bind(boost::factory<boost::shared_ptr<printer> >(), boost::ref(io));
	//boost::shared_ptr<printer> p = boost::dynamic_pointer_cast<printer, timer_base>(factories["printer"]());
	p1->schedule(1);
	p2->schedule(2);

	t.join();
	return 0;
}

