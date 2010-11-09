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
#include <boost/threadpool.hpp>
#include <boost/stm/transaction.hpp>
#include <boost/stm/tx/pointer.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost::threadpool;
using namespace boost::stm;
using namespace boost;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

// Create fifo thread pool container with two threads.
const int thread_count = 8;
pool tp(thread_count);
boost::barrier bar(thread_count);

boost::mutex the_mutex;

void init()
{
	boost::stm::transaction::initialize_thread();
	bar.wait();
}

class object_base
{
public:
	object_base() : id_(0)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "object_base::object_base()" << endl;
	}

	virtual ~object_base()
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "object_base::~object_base()" << endl;
	}

	virtual void write_id(int id)
	{
        transaction* tx=current_transaction();
        if (tx!=0) {
			tx->w(id_) = id;
		}
	}

	virtual int read_id() const
	{
		int result = 0;
        transaction* tx=current_transaction();
        if (tx!=0) {
			result = tx->r(id_);
		}
		return result;
	}

	virtual void print_id() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "id=" << id_ << endl;
	}
protected:
	native_trans<int> id_;
};

class manager_item : public transaction_object<manager_item>
{
	friend class object_manager;
public:
	manager_item() : internal_(NULL)
	{
	}
	virtual ~manager_item()
	{
		if (internal_)
			delete internal_;
	}
protected:

	object_base* internal_;
};

class object_manager
{
public:
	object_manager() : item_(NULL)
	{
	}

	object_base* get_obj()
	{
		object_base* tmp = NULL;
        transaction* tx=current_transaction();
        if (tx!=0) {
			tmp = tx->read_ptr(item_)->internal_;
		}
		return tmp;
	}
	void add_obj(object_base* obj)
	{
        transaction* tx=current_transaction();
        if (tx!=0) {
			item_ = tx->new_memory(item_);
			tx->write_ptr(item_)->internal_ = obj;
		}
	}
	void del_obj()
	{
        transaction* tx=current_transaction();
        if (tx!=0) {
			tx->delete_memory(*item_);
			item_ = NULL;
		}
	}
protected:
	manager_item *item_;
};

class test_parent : public object_base
{
public:
	test_parent() : parent_id_(0)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "test_parent::test_parent()" << endl;
	}

	virtual ~test_parent()
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "test_parent::~test_parent()" << endl;
	}

	virtual void write_id(int id)
	{
        transaction* tx=current_transaction();
        if (tx!=0) {
			tx->w(parent_id_) = id;
		}
	}

	virtual int read_id() const
	{
		int result = 0;
        transaction* tx=current_transaction();
        if (tx!=0) {
			result = tx->r(parent_id_);
		}
		return result;
	}

	virtual void print_id() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "id=" << id_ << ":" << parent_id_ << endl;
	}
protected:
	native_trans<int> parent_id_;
};

class test_child : public test_parent
{
public:
	test_child() : child_id_(0)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "test_child::test_child()" << endl;
	}

	virtual ~test_child()
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "test_child::~test_child()" << endl;
	}

	virtual void write_id(int id)
	{
		if (id-1 != child_id_)
		{
			boost::mutex::scoped_lock lock(the_mutex);
			cout << child_id_ << ":" << id << endl;
		}
        transaction* tx=current_transaction();
        if (tx!=0) {
			tx->w(child_id_) = id;
		}
	}

	virtual int read_id() const
	{
		int result = 0;
        transaction* tx=current_transaction();
        if (tx!=0) {
			result = tx->r(child_id_);
		}
		return result;
	}

	virtual void print_id() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "id=" << id_ << ":" << parent_id_ << ":" << child_id_ << endl;
	}
protected:
	native_trans<int> child_id_;
};

object_manager om;

static void create()
{
	try_atomic(t)
	{
		object_base* obj = new test_child;
		om.add_obj(obj);
	}
	before_retry
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "create" << endl;
	}
}

static void change()
{

	try_atomic(t)
	{
		object_base* obj = om.get_obj();
		int tmp = obj->read_id();
		obj->write_id(tmp+1);
	}
	before_retry
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "retry change" << endl;
	}

}

static void print()
{
	int my_id = 0;

	try_atomic(t)
	{
		object_base* obj = om.get_obj();
		my_id = obj->read_id();
	}
	before_retry
	{
		boost::mutex::scoped_lock lock(the_mutex);
		cout << "retry print" << endl;
	}

	boost::mutex::scoped_lock lock(the_mutex);
	cout << "id=" << my_id << endl;
}

static void destroy()
{
	try_atomic(t)
	{
		om.del_obj();
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
		tp.schedule(print);
	}	

	tp.wait();

	print();

	destroy();

	// Trigger defered delete
	transaction t;

	return 0;
}
