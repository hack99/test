
#include <iostream>
#include <boost/thread.hpp>
#include <boost/threadpool.hpp>

using namespace std;
using namespace boost;
using namespace boost::threadpool;

// Create fifo thread pool container with two threads.
const int thread_count = 4;
pool tp(thread_count);
boost::barrier bar(thread_count);

class base_transaction_object
{
	enum
	{
		ORGINAL_OBJECT,
		TRANSACTION_OBJECT,
	};

public:
	base_transaction_object() : transaction_flag_(ORGINAL_OBJECT)
	{
	}
	virtual ~base_transaction_object()
	{
	}
	bool is_transaction_object()
	{
		return (transaction_flag_ == TRANSACTION_OBJECT);
	}
protected:
	int transaction_flag_;
};

class transaction
{
	enum 
	{
		NO_TRANSACTION,
		START_TRANSACTION,
		COMMITTED_TRANSACTION,
		CANCELLED_TRANSACTION,
		RESTART_TRANSACTION,
	};
public:
	transaction() : state_(NO_TRANSACTION), thread_id_(boost::this_thread::get_id())
	{
		cout << "transaction::transaction " << thread_id_ << endl;
	}
	virtual ~transaction()
	{
		cout << "transaction::~transaction " << thread_id_ << endl;
	}

	void start()
	{
		state_ = START_TRANSACTION;
	}
	void cancel()
	{
		state_ = CANCELLED_TRANSACTION;
	}
	void commit()
	{
		state_ = COMMITTED_TRANSACTION;
	}
	void restart()
	{
		state_ = RESTART_TRANSACTION;
	}
	void final()
	{
		state_ = NO_TRANSACTION;
	}

	bool check_state()
	{
		return ((state_ == START_TRANSACTION) || (state_ == RESTART_TRANSACTION));
	}
protected:
	int state_;
	boost::thread::id thread_id_;
	std::map<base_transaction_object*, base_transaction_object*> write_container_;
	std::map<base_transaction_object*, base_transaction_object*> read_container_;
};

class stm_manager
{
public:
	stm_manager()
	{
	}
	virtual ~stm_manager()
	{
	}

	void initialize_thread()
	{
		transaction* p = new transaction;
		transaction_.reset(p);
		transaction_container_.insert(pair<boost::thread::id, transaction*>(boost::this_thread::get_id(), p));
		cout << "Insert " << boost::this_thread::get_id() << ":" << p << endl;
	}

	void finalize_thread()
	{
		cout << "Erase " << boost::this_thread::get_id() << endl;
		transaction_container_.erase(boost::this_thread::get_id());
		transaction_.reset();
	}

	void start_transaction()
	{
		transaction_->start();
	}

	void cancel_transaction()
	{
		transaction_->cancel();
	}

	void commit_transaction()
	{
		transaction_->commit();
	}
	
	void restart_transaction()
	{
		transaction_->restart();
	}

	void final_transaction()
	{
		transaction_->final();
	}

	bool continue_transaction()
	{
		return transaction_->check_state();
	}

	template <class T> const T* read_ptr(const T* obj)
	{
		if (obj->is_transaction_object())
		{
			
		}
	}
 
	template <class T> T* write_ptr(T* obj)
	{
		
	}

protected:
	std::map<boost::thread::id, transaction*> transaction_container_;
	boost::thread_specific_ptr<transaction> transaction_;
};

class stm_manager;

stm_manager bobo2_stm;

void init()
{
	bobo2_stm.initialize_thread();
	bar.wait();
}

void fini()
{
	bobo2_stm.finalize_thread();
	bar.wait();
}

class object_base : public base_transaction_object
{
public:
	object_base() : id_(0)
	{
		cout << "object_base::object_base()" << endl;
	}

	virtual ~object_base()
	{
		cout << "object_base::~object_base()" << endl;
	}

	virtual void write_id(int id)
	{
		id_ = id;
	}

	virtual int read_id() const
	{
		return id_;
	}

	virtual void print_id() const
	{
		cout << "id=" << id_ << endl;
	}
protected:
	int id_;
};

class test_parent : public object_base
{
public:
	test_parent() : parent_id_(0)
	{
		cout << "test_parent::test_parent()" << endl;
	}

	virtual ~test_parent()
	{
		cout << "test_parent::~test_parent()" << endl;
	}

	virtual void write_id(int id)
	{
		parent_id_ = id;
	}

	virtual int read_id() const
	{
		return parent_id_;
	}

	virtual void print_id() const
	{
		cout << "id=" << id_ << ":" << parent_id_ << endl;
	}
protected:
	int parent_id_;
};

class test_child : public test_parent
{
public:
	test_child() : child_id_(0)
	{
		cout << "test_child::test_child()" << endl;
	}

	virtual ~test_child()
	{
		cout << "test_child::~test_child()" << endl;
	}

	virtual void write_id(int id)
	{
		child_id_ = id;
	}

	virtual int read_id() const
	{
		return child_id_;
	}

	virtual void print_id() const
	{
		cout << "id=" << id_ << ":" << parent_id_ << ":" << child_id_ << endl;
	}
protected:
	int child_id_;
};

test_child obj;

void func()
{
	bobo2_stm.start_transaction();
	while (!bobo2_stm.continue_transaction())
	try
	{
		bobo2_stm.commit_transaction();
	}
	catch (...)
	{
		bobo2_stm.restart_transaction();
	}
	bobo2_stm.final_transaction();
}

int main()
{
	for (int i = 0; i < thread_count; i++)
	{
		tp.schedule(init);
	}

	tp.schedule(func);
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	
	for (int i = 0; i < thread_count; i++)
	{
		tp.schedule(fini);
	}

	tp.wait();

	return 0;
}
