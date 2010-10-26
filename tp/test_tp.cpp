/*! \file
 * \brief Quick start example.
 *
 * This is a very simple example which can be used to configure the threadpool environment on your system. 
 *
 * Copyright (c) 2005-2006 Philipp Henkel
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * http://threadpool.sourceforge.net
 *
 */

#include <iostream>

#include <boost/threadpool.hpp>

using namespace std;
using namespace boost::threadpool;

boost::mutex the_mutex;

// Some example tasks
void first_task(int i)
{
	{
		boost::mutex::scoped_lock lock(the_mutex);
   	cout << "Loop " << i << " first task is running in " << boost::this_thread::get_id() << "\n" ;
	}
   sleep(1);
}

void second_task(int i)
{
	{
		boost::mutex::scoped_lock lock(the_mutex);	
   	cout << "Loop " << i << " second task is running in " << boost::this_thread::get_id() << "\n" ;
	}
   sleep(1);
}

int main(int argc,char *argv[])
{
   // Create fifo thread pool container with two threads.
   pool tp(10);
   
   // Add some tasks to the pool.
   for (int i = 0; i < 100; i++)
   {
   	if (i%2==0)
   	tp.schedule(boost::bind(first_task, i));
   	else
   	tp.schedule(boost::bind(second_task, i));   
	}
  
   //  Wait until all tasks are finished.
   tp.wait();
   
   // Now all tasks are finished!	
   return(0);
}
