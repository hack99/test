//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Justin E. Gottchlich 2009. 
// (C) Copyright Vicente J. Botet Escriba 2009. 
// Distributed under the Boost
// Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or 
// copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/synchro for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include <boost/stm.hpp>
#include <boost/threadpool.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost::threadpool;
using namespace boost::stm;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// Create fifo thread pool container with two threads.
const int thread_count = 8;
pool tp(thread_count);
boost::barrier bar(thread_count);

void init()
{
	boost::stm::transaction::initialize_thread();
	bar.wait();
}

class object_base : public transaction_object<object_base>
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

class test_parent : public transaction_object<test_parent, object_base>
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

class test_child : public transaction_object<test_child, test_parent>
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

object_base* obj = NULL;

static void create()
{
	atomic(t)
	{
		obj = t.new_memory<test_child>(NULL);
	}
	before_retry
	{
		cout << "create" << endl;
	}
}

static void change()
{
	try_atomic(t)
	{
		object_base* tmp = t.write_ptr(obj);
		for (int i = 0; i < 10; i++)
		{
			tmp->write_id(tmp->read_id()+1);
		}
	}
	before_retry
	{
		cout << "retry change" << endl;
	}
}

static void print()
{
	atomic(t)
	{
		obj->print_id();
	}
	before_retry
	{
		cout << "print" << endl;
	}
}

static void destroy()
{
	atomic(t)
	{
		t.delete_memory(*obj);
	}
	before_retry
	{
		cout << "destroy" << endl;
	}
}

int main()
{

	boost::stm::transaction::initialize();
	boost::stm::transaction::initialize_thread();

	for (int i = 0; i < thread_count; i++)
	{
		tp.schedule(init);
	}

	//boost::stm::transaction::do_direct_updating();
	boost::stm::transaction::do_deferred_updating();

	create();

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	for (int i = 0; i < 1000; i++)
	{
		tp.schedule(change);
	}	

	tp.wait();

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	print();

	destroy();

	return 0;
}


