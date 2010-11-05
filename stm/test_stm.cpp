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
#include <boost/stm/transaction.hpp>
#include <boost/threadpool.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost::threadpool;
using namespace boost::stm;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

native_trans<int> a = 0;

// Create fifo thread pool container with two threads.
const int thread_count = 10;
pool tp(thread_count);
boost::barrier bar(thread_count);

void init()
{
	cout << "init start" << endl;
	boost::stm::transaction::initialize_thread();
	cout << "init wait" << endl;
	bar.wait();
	cout << "init finish" << endl;
}

void func(int i)
{
	atomic (t)
	{
		for (int i = 0; i < 1000; i++)
			t.w(a)++;
	}end_atom

	cout << "Commit " << i << ":" << a << endl;
}

int main()
{

	boost::stm::transaction::initialize();

	for (int i = 0; i < thread_count; i++)
	{
		tp.schedule(init);
	}

	boost::stm::transaction::do_direct_updating();
	//boost::stm::transaction::do_deferred_updating();

	// Add some tasks to the pool.
	for (int i = 0; i < 1000; i++)
	{
		tp.schedule(boost::bind(func, i));
	}

	tp.wait();

	cout << "Final " << a << endl;

	return 0;
}


