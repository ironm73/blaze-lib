//=================================================================================================
/*!
//  \file blaze/util/ThreadPool.h
//  \brief Header file of the ThreadPool class
//
//  Copyright (C) 2013 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_UTIL_THREADPOOL_THREADPOOL_H_
#define _BLAZE_UTIL_THREADPOOL_THREADPOOL_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <stdexcept>
#include <boost/bind.hpp>
#include <blaze/util/Assert.h>
#include <blaze/util/NonCopyable.h>
#include <blaze/util/PtrVector.h>
#include <blaze/util/StaticAssert.h>
#include <blaze/util/Thread.h>
#include <blaze/util/threadpool/Task.h>
#include <blaze/util/threadpool/TaskQueue.h>
#include <blaze/util/Types.h>


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup threads Thread parallelization
// \ingroup util
//
// The Blaze library offers the capability to either create individual threads for specific
// tasks (see the Thread class desciption for details) and to create a thread pool according
// to the thread pool pattern (see the ThreadPool class description). Both class descriptions
// offer examples for the setup of threads and the parallel execution of concurrent tasks.
*/
/*!\brief Implementation of a thread pool.
// \ingroup threads
//
// \section threadpool_general General
//
// The ThreadPool class template represents a thread pool according to the thread pool pattern
// (see for example http://en.wikipedia.org/wiki/Thread_pool_pattern). It manages a certain
// number of threads in order to process a larger number of independent tasks.
//
// \image html threadpool.png
// \image latex threadpool.eps "Thread pool pattern" width=430pt
//
// The primary purpose of a thread pool is the reuse of system resources: instead of creating
// a single thread for every individual task, threads are reused to handle several tasks. This
// increases the performance in comparison to different threading strategies, as illustrated
// in the graph below. The first bar indicates the sequential performance of 1000 matrix-matrix
// multiplications of arbitrarily sized square matrices. The second bar shows the performance
// of the same work performed by 1000 distinct threads (i.e. one thread for each matrix-matrix
// multiplication) on a quad-core system. In this case, all cores of the system can be used,
// but the additional overhead of creating and managing new threads prevents the expected
// performance increase by a factor of four. The third bar illustrates the performance of four
// threads distributing the work between them (i.e. 250 matrix-matrix multiplications per
// thread), again using the same quad-core system. This approach nearly achieves four times
// the performance of the sequential execution. The fourth bar represents the performance of
// the ThreadPool class using fourth threads for the execution of the 1000 individual
// multiplications.
//
// \image html threadpool2.png
// \image latex threadpool2.eps "Performance comparison of different thread strategies" width=450pt
//
// Additionally, the thread pool approach simplifies load balancing and increases the stability
// of the system.
//
//
// \section threadpool_definition Class Definition
//
// The implementation of the ThreadPool class template is based on the implementation of standard
// thread functionality as provided by the C++11 standard or the Boost library. Via the four
// template parameters it is possible to configure a ThreadPool instance as either a C++11 thread
// pool or as Boost thread pool:

   \code
   template< typename TT, typename MT, typename LT, typename CT >
   class ThreadPool;
   \endcode

//  - TT: specifies the type of the encapsulated thread. This can either be \c std::thread,
//        \c boost::thread, or any other standard conforming thread type.
//  - MT: specifies the type of the used synchronization mutex. This can for instance be
//        \c std::mutex, \c boost::mutex, or any other standard conforming mutex type.
//  - LT: specifies the type of lock used in combination with the given mutex type. This
//        can be any standard conforming lock type, as for instance \c std::unique_lock,
//        \c boost::unique_lock.
//  - CT: specifies the type of the used condition variable. This can for instance be
//        \c std::condition_variable, \c boost::condition_variable, or any other standard
//        conforming condition variable type.
//
// The following example demonstrates how to configure the ThreadPool class template as either
// C++11 standard thread pool or as Boost thread pool:

   \code
   typedef blaze::ThreadPool< boost::thread
                            , boost::mutex
                            , boost::unique_lock<boost::mutex>
                            , boost::condition_variable >  BoostThreadPool;

   typedef blaze::ThreadPool< std::thread
                            , std::mutex
                            , std::unique_lock<std::mutex>
                            , std::condition_variable >  StdThreadPool;
   \endcode

// For more information about the standard thread functionality, see [1] or [2] or the current
// documentation at the Boost homepage: www.boost.org.
//
//
// \section threadpool_setup Using the ThreadPool class
//
// The following example demonstrates the use of the ThreadPool class. In contrast to the setup
// of individual threads (see the Thread class description for more details), it is not necessary
// to create and manage individual threads, but only to schedules tasks for the accordingly sized
// thread pool.

   \code
   // Definition of a function with no arguments that returns void
   void function0() { ... }

   // Definition of a functor (function object) taking two arguments and returning void
   struct Functor2
   {
      void operator()( int a, int b ) { ... }
   };

   int main()
   {
      // Creating a thread pool with initially two working threads
      StdThreadPool threadpool( 2 );

      // Scheduling two concurrent tasks
      threadpool.schedule( function0 );
      threadpool.schedule( Functor2(), 4, 6 );

      // Waiting for the thread pool to complete both tasks
      threadpool.wait();

      // Resizing the thread pool to four working threads
      threadpool.resize( 4 );

      // Scheduling other concurrent tasks
      ...
      threadpool.schedule( function0 );
      ...

      // At the end of the thread pool scope, all tasks remaining in the task queue are removed
      // and all currently running tasks are completed. Additionally, all acquired resources are
      // safely released.
   }
   \endcode

// Note that the ThreadPool class template schedule() function allows for up to five arguments
// for the given functions/functors.
//
//
// \section thread_exception Throwing exceptions in a thread parallel environment
//
// It can happen that during the execution of a given task a thread encounters an erroneous
// situation and has to throw an exception. However, exceptions thrown in the usual way
// cannot be caught by a try-catch-block in the main thread of execution:

   \code
   // Definition of a function throwing a std::runtime_error during its execution
   void task()
   {
      ...
      throw std::runtime_error( ... );
      ...
   }

   // Creating a thread pool executing the throwing function. Although the setup, the scheduling
   // of the task, the wait() function and the destruction of the thread pool are encapsuled
   // inside a try-catch-block, the exception cannot be caught and results in an abortion of the
   // program.
   try {
      Threadpool threadpool( 2 );
      thread.schedule( task );
      threadpool.wait();
   }
   catch( ... )
   {
      ...
   }
   \endcode

// For a detailed explanation how to portably transport exceptions between threads, see [1] or
// [2]. In case of the Boost library, the according Boost functionality as demonstrated in the
// following example has to be used. Note that any function/functor scheduled for execution is
// responsible to handle exceptions in this way!

   \code
   #include <boost/bind.hpp>
   #include <boost/exception_ptr.hpp>

   // Definition of a function that happens to throw an exception. In order to throw the
   // exception, boost::enable_current_exception() is used in combination with throw.
   void throwException()
   {
      ...
      throw boost::enable_current_exception( std::runtime_error( ... ) );
      ...
   }

   // Definition of a thread function. The try-catch-block catches the exception and uses the
   // boost::current_exception() function to get a boost::exception_ptr object.
   void task( boost::exception_ptr& error )
   {
      try {
         throwException();
         error = boost::exception_ptr();
      }
      catch( ... ) {
         error = boost::current_exception();
      }
   }

   // The function that start a thread of execution can pass along a boost::exception_ptr object
   // that is set in case of an exception. Note that boost::current_exception() captures the
   // original type of the exception object. The exception can be thrown again using the
   // boost::rethrow_exception() function.
   void work()
   {
      boost::exception_ptr error;

      StdThreadPool threadpool( 2 );
      threadpool.schedule( boost::bind( task, boost::ref(error) ) );
      threadpool.wait();

      if( error ) {
         std::cerr << " Exception during thread execution!\n\n";
         boost::rethrow_exception( error );
      }
   }
   \endcode

// \section threadpool_references References
//
// [1] A. Williams: C++ Concurrency in Action, Manning, 2012, ISBN: 978-1933988771\n
// [2] B. Stroustrup: The C++ Programming Language, Addison-Wesley, 2013, ISBN: 978-0321563842\n
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
class ThreadPool : private NonCopyable
{
 private:
   //**Type definitions****************************************************************************
   typedef Thread<TT,MT,LT,CT>       ManagedThread;  //!< Type of the managed threads.
   typedef PtrVector<ManagedThread>  Threads;        //!< Type of the thread container.
   typedef threadpool::TaskQueue     TaskQueue;      //!< Type of the task queue.
   typedef MT                        Mutex;          //!< Type of the mutex.
   typedef LT                        Lock;           //!< Type of a locking object.
   typedef CT                        Condition;      //!< Condition variable type.
   //**********************************************************************************************

 public:
   //**Constructor*********************************************************************************
   /*!\name Constructor */
   //@{
   explicit ThreadPool( size_t n );
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   /*!\name Destructor */
   //@{
   ~ThreadPool();
   //@}
   //**********************************************************************************************

   //**Get functions*******************************************************************************
   /*!\name Get functions */
   //@{
   inline bool   isEmpty() const;
   inline size_t size()    const;
   inline size_t active()  const;
   inline size_t ready()   const;
   //@}
   //**********************************************************************************************

   //**Scheduling functions************************************************************************
   /*!\name Scheduling functions */
   //@{
   template< typename Callable >
   void schedule( Callable func );

   template< typename Callable, typename A1 >
   void schedule( Callable func, A1 a1 );

   template< typename Callable, typename A1, typename A2 >
   void schedule( Callable func, A1 a1, A2 a2 );

   template< typename Callable, typename A1, typename A2, typename A3 >
   void schedule( Callable func, A1 a1, A2 a2, A3 a3 );

   template< typename Callable, typename A1, typename A2, typename A3, typename A4 >
   void schedule( Callable func, A1 a1, A2 a2, A3 a3, A4 a4 );

   template< typename Callable, typename A1, typename A2, typename A3, typename A4, typename A5 >
   void schedule( Callable func, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5 );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
   void resize( size_t n );
   void wait();
   void clear();
   //@}
   //**********************************************************************************************

 private:
   //**Thread functions****************************************************************************
   /*!\name Thread functions */
   //@{
   void createThread();
   bool executeTask();
   //@}
   //**********************************************************************************************

   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   volatile size_t total_;     //!< Total number of threads in the thread pool.
   volatile size_t expected_;  //!< Expected number of threads in the thread pool.
                               /*!< This number may differ from the total number of threads
                                    during a resize of the thread pool. */
   volatile size_t active_;    //!< Number of currently active/busy threads.
   Threads threads_;           //!< The threads contained in the thread pool.
   TaskQueue taskqueue_;       //!< Task queue for the scheduled tasks.
   mutable Mutex mutex_;       //!< Synchronization mutex.
   Condition waitForTask_;     //!< Wait condition for idle threads.
   Condition waitForThread_;   //!< Wait condition for the thread management.
   //@}
   //**********************************************************************************************

   //**Friend declarations*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   friend class Thread<TT,MT,LT,CT>;
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constructor for the ThreadPool class.
//
// \param n Initial number of threads \f$[1..\infty)\f$.
//
// This constructor creates a thread pool with initially \a n new threads. All threads are
// initially idle until a task is scheduled.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
ThreadPool<TT,MT,LT,CT>::ThreadPool( size_t n )
   : total_     ( 0 )  // Total number of threads in the thread pool
   , expected_  ( 0 )  // Expected number of threads in the thread pool
   , active_    ( 0 )  // Number of currently active/busy threads
   , threads_      ()  // The threads contained in the thread pool
   , taskqueue_    ()  // Task queue for the scheduled tasks
   , mutex_        ()  // Synchronization mutex
   , waitForTask_  ()  // Wait condition for idle threads
   , waitForThread_()  // Wait condition for the thread management
{
   resize( n );
}
//*************************************************************************************************




//=================================================================================================
//
//  DESTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Destructor for the ThreadPool class.
//
// The destructor clears all remaining tasks from the task queue and waits for the currently
// active threads to complete their tasks.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
ThreadPool<TT,MT,LT,CT>::~ThreadPool()
{
   Lock lock( mutex_ );

   // Removing all currently queued tasks
   taskqueue_.clear();

   // Setting the expected number of threads
   expected_ = 0;

   // Notifying all idle threads
   waitForTask_.notify_all();

   // Waiting for all threads to terminate
   while( total_ != 0 ) {
      waitForThread_.wait( lock );
   }

   // Joining all threads
   typedef typename Threads::Iterator  Iterator;
   const Iterator end( threads_.end() );
   for( Iterator thread=threads_.begin(); thread!=end; ++thread ) {
      thread->join();
   }

   // Destroying all threads
   threads_.clear();
}
//*************************************************************************************************




//=================================================================================================
//
//  GET FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether any tasks are scheduled for execution.
//
// \return \a true in case task are scheduled, \a false otherwise.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
inline bool ThreadPool<TT,MT,LT,CT>::isEmpty() const
{
   Lock lock( mutex_ );
   return taskqueue_.isEmpty();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the current size of the thread pool.
//
// \return The total number of threads in the thread pool.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
inline size_t ThreadPool<TT,MT,LT,CT>::size() const
{
   Lock lock( mutex_ );
   return expected_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the number of currently active/busy threads.
//
// \return The number of currently active threads.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
inline size_t ThreadPool<TT,MT,LT,CT>::active() const
{
   Lock lock( mutex_ );
   return active_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the number of currently ready/inactive threads.
//
// \return The number of currently ready threads.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
inline size_t ThreadPool<TT,MT,LT,CT>::ready() const
{
   Lock lock( mutex_ );
   return expected_ - active_;
}
//*************************************************************************************************




//=================================================================================================
//
//  SCHEDULING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Scheduling the given zero argument function/functor for execution.
//
// \param func The given function/functor.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable without arguments and must return void.
*/
template< typename TT          // Type of the encapsulated thread
        , typename MT          // Type of the synchronization mutex
        , typename LT          // Type of the mutex lock
        , typename CT >        // Type of the condition variable
template< typename Callable >  // Type of the function/functor
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func )
{
   Lock lock( mutex_ );
   taskqueue_.push( func );
   waitForTask_.notify_one();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scheduling the given unary function/functor for execution.
//
// \param func The given function/functor.
// \param a1 The first argument.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable with one argument and must return void.
*/
template< typename TT        // Type of the encapsulated thread
        , typename MT        // Type of the synchronization mutex
        , typename LT        // Type of the mutex lock
        , typename CT >      // Type of the condition variable
template< typename Callable  // Type of the function/functor
        , typename A1 >      // Type of the first argument
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func, A1 a1 )
{
   Lock lock( mutex_ );
   taskqueue_.push( boost::bind<void>( func, a1 ) );
   waitForTask_.notify_one();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scheduling the given binary function/functor for execution.
//
// \param func The given function/functor.
// \param a1 The first argument.
// \param a2 The second argument.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable with two arguments and must return void.
*/
template< typename TT        // Type of the encapsulated thread
        , typename MT        // Type of the synchronization mutex
        , typename LT        // Type of the mutex lock
        , typename CT >      // Type of the condition variable
template< typename Callable  // Type of the function/functor
        , typename A1        // Type of the first argument
        , typename A2 >      // Type of the second argument
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func, A1 a1, A2 a2 )
{
   Lock lock( mutex_ );
   taskqueue_.push( boost::bind<void>( func, a1, a2 ) );
   waitForTask_.notify_one();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scheduling the given ternary function/functor for execution.
//
// \param func The given function/functor.
// \param a1 The first argument.
// \param a2 The second argument.
// \param a3 The third argument.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable with three arguments and must return void.
*/
template< typename TT        // Type of the encapsulated thread
        , typename MT        // Type of the synchronization mutex
        , typename LT        // Type of the mutex lock
        , typename CT >      // Type of the condition variable
template< typename Callable  // Type of the function/functor
        , typename A1        // Type of the first argument
        , typename A2        // Type of the second argument
        , typename A3 >      // Type of the third argument
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func, A1 a1, A2 a2, A3 a3 )
{
   Lock lock( mutex_ );
   taskqueue_.push( boost::bind<void>( func, a1, a2, a3 ) );
   waitForTask_.notify_one();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scheduling the given four argument function/functor for execution.
//
// \param func The given function/functor.
// \param a1 The first argument.
// \param a2 The second argument.
// \param a3 The third argument.
// \param a4 The fourth argument.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable with four arguments and must return void.
*/
template< typename TT        // Type of the encapsulated thread
        , typename MT        // Type of the synchronization mutex
        , typename LT        // Type of the mutex lock
        , typename CT >      // Type of the condition variable
template< typename Callable  // Type of the function/functor
        , typename A1        // Type of the first argument
        , typename A2        // Type of the second argument
        , typename A3        // Type of the third argument
        , typename A4 >      // Type of the fourth argument
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func, A1 a1, A2 a2, A3 a3, A4 a4 )
{
   Lock lock( mutex_ );
   taskqueue_.push( boost::bind<void>( func, a1, a2, a3, a4 ) );
   waitForTask_.notify_one();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Scheduling the given four argument function/functor for execution.
//
// \param func The given function/functor.
// \param a1 The first argument.
// \param a2 The second argument.
// \param a3 The third argument.
// \param a4 The fourth argument.
// \param a5 The fifth argument.
// \return void
//
// This function schedules the given function/functor for execution. The given function/functor
// must be copyable, must be callable with five arguments and must return void.
*/
template< typename TT        // Type of the encapsulated thread
        , typename MT        // Type of the synchronization mutex
        , typename LT        // Type of the mutex lock
        , typename CT >      // Type of the condition variable
template< typename Callable  // Type of the function/functor
        , typename A1        // Type of the first argument
        , typename A2        // Type of the second argument
        , typename A3        // Type of the third argument
        , typename A4        // Type of the fourth argument
        , typename A5 >      // Type of the fifth argument
void ThreadPool<TT,MT,LT,CT>::schedule( Callable func, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5 )
{
   Lock lock( mutex_ );
   taskqueue_.push( boost::bind<void>( func, a1, a2, a3, a4, a5 ) );
   waitForTask_.notify_one();
}
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Changes the total number of threads in the thread pool.
//
// \param n The new number of threads \f$[1..\infty)\f$.
// \return void
// \exception std::invalid_argument Invalid number of threads.
//
// This function changes the size of the thread pool, i.e. changes the total number of threads
// contained in the pool. If \a n is smaller than the current size of the thread pool, the
// according number of threads is removed from the pool, otherwise new threads are added to
// the pool.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
void ThreadPool<TT,MT,LT,CT>::resize( size_t n )
{
   // Checking the given number of threads
   if( n == 0 )
      throw std::invalid_argument( "Invalid number of threads" );

   // Adjusting the number of threads
   {
      Lock lock( mutex_ );

      // Adding new threads to the thread pool
      if( n > expected_ ) {
         for( size_t i=expected_; i<n; ++i )
            createThread();
      }

      // Removing threads from the pool
      else {
         expected_ = n;
         waitForTask_.notify_all();
      }
   }

   // Joining and destroying any terminated thread
   typedef typename Threads::Iterator  Iterator;
   for( Iterator thread=threads_.begin(); thread!=threads_.end(); ) {
      if( thread->hasTerminated() ) {
         thread->join();
         thread = threads_.erase( thread );
      }
      else ++thread;
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Waiting for all scheduled tasks to be completed.
//
// \return void
//
// This function blocks until all scheduled tasks have been completed.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
void ThreadPool<TT,MT,LT,CT>::wait()
{
   Lock lock( mutex_ );

   while( !taskqueue_.isEmpty() || active_ > 0 ) {
      waitForThread_.wait( lock );
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Removing all scheduled tasks from the thread pool.
//
// \return void
//
// This function removes all currently scheduled tasks from the thread pool. The total number
// of threads remains unchanged and all active threads continue completing their tasks.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
void ThreadPool<TT,MT,LT,CT>::clear()
{
   Lock lock( mutex_ );
   taskqueue_.clear();
}
//*************************************************************************************************




//=================================================================================================
//
//  THREAD FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Adding a new thread to the thread pool.
//
// \return void
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
void ThreadPool<TT,MT,LT,CT>::createThread()
{
   threads_.pushBack( new ManagedThread( this ) );
   ++total_;
   ++expected_;
   ++active_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Executing a scheduled task.
//
// \return \a true in case a task was successfully finished, \a false if not.
//
// This function is repeatedly called by every thread to execute one of the scheduled tasks.
// In case there is no task available, the thread blocks and waits for a new task to be
// scheduled.
*/
template< typename TT    // Type of the encapsulated thread
        , typename MT    // Type of the synchronization mutex
        , typename LT    // Type of the mutex lock
        , typename CT >  // Type of the condition variable
bool ThreadPool<TT,MT,LT,CT>::executeTask()
{
   threadpool::Task task;

   // Acquiring a scheduled task
   {
      Lock lock( mutex_ );

      while( taskqueue_.isEmpty() )
      {
         --active_;
         waitForThread_.notify_all();

         if( total_ > expected_ ) {
            --total_;
            return false;
         }

         waitForTask_.wait( lock );
         ++active_;
      }

      BLAZE_INTERNAL_ASSERT( !taskqueue_.isEmpty(), "Empty task queue detected" );
      task = taskqueue_.pop();
   }

   // Executing the task
   task();

   return true;
}
//*************************************************************************************************

} // namespace blaze

#endif
