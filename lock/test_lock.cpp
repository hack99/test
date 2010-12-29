#include <boost/thread.hpp>
#include <iostream>

using namespace std;

class share_lock
{
	public:
	share_lock() : count_(0)
	{
	}
	
	void inc_count()
	{
		boost::lock_guard<boost::mutex> lock(mutex_);
		count_++;
	}
	
	
	void wait()
	{
		cout << "share_lock::wait" << endl;
		boost::unique_lock<boost::mutex> lock(mutex_);
		while(count_ > 0)
		{
			cond_.wait(lock);
		}
	}
	
	
	void notify()
	{
		cout << "share_lock::notify" << endl;
		{
			boost::lock_guard<boost::mutex> lock(mutex_);
			if (count_ > 0) count_--; else return;
		}
		cond_.notify_one();
	}
	
	protected:
	boost::condition_variable cond_;
	boost::mutex mutex_;
	int count_;
};

void test_thread2(share_lock *lock)
{
	sleep(1);
	lock->notify();
}

void test_thread1()
{
	share_lock lock;
	lock.inc_count();
	
	boost::thread t1(boost::bind(test_thread2, &lock));

	lock.wait();

	t1.join();
}

int main()
{
	boost::thread t1(test_thread1);
	t1.join();
	return 0;
}
