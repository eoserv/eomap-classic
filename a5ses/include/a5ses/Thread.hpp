
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Thread.hpp
/// @ingroup Thread
/// Contains threading related classes

#ifndef A5SES_THREAD_HPP_INCLUDED
#define A5SES_THREAD_HPP_INCLUDED

#include "common.hpp"

namespace a5
{

/// @ingroup Thread
/// Object representing a mutex
class Mutex
{
	public:
		/// Allows recursive locks
		static const int Recursive = 0x80000000;

	private:
		/// Pointer to the Allegro mutex
		ALLEGRO_MUTEX *mutex;

		/// Whether or not to destroy the mutex when the object is destroyed
		bool created;

		// Disable copying
		Mutex(const Mutex &);
		Mutex &operator =(const Mutex &);

	public:
		/// Creates a mutex with the specified flags
		Mutex(int flags = 0)
			: created(true)
		{
			if (flags & Recursive)
				this->mutex = al_create_mutex_recursive();
			else
				this->mutex = al_create_mutex();
		}

		/// Locks the mutex
		void Lock()
		{
			al_lock_mutex(this->mutex);
		}

		/// Unlocks the mutex
		void Unlock()
		{
			al_unlock_mutex(this->mutex);
		}

		/// Destroys the mutex
		~Mutex()
		{
			if (this->created)
				al_destroy_mutex(this->mutex);
		}

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_MUTEX *Release()
		{
			this->created = false;
			return *this;
		}

		/// Returns a pointer to the Allegro mutex structure
		operator ALLEGRO_MUTEX *() const
		{
			return this->mutex;
		}
};

/// @ingroup Thread
/// RAII method of locking mutexes
class Lock
{
	private:
		/// Pointer to the mutex
		Mutex *mutex;

		/// Whether or not to destroy the mutex when the object is destroyed
		bool created;

		// Disable copying
		Lock(const Lock &);
		Lock &operator =(const Lock &);

	public:
		/// Creates and locks an anonymous mutex
		Lock()
			: mutex(new Mutex)
			, created(true)
		{
			this->mutex->Lock();
		}

		/// Locks the mutex
		Lock(Mutex &mutex_)
			: mutex(&mutex_)
			, created(false)
		{
			mutex_.Lock();
		}

		/// Unlocks the mutex
		~Lock()
		{
			this->mutex->Unlock();

			if (this->created)
				delete this->mutex;
		}
};

/// @ingroup Thread
/// Object representing a condition
class Condition
{
	private:
		/// Pointer to the Allegro condition
		ALLEGRO_COND *cond;

		/// Whether or not to destroy the condition when the object is destroyed
		bool created;

		// Disable copying
		Condition(const Condition &);
		Condition &operator =(const Condition &);

	public:
		/// Creates the condition
		Condition()
			: created(true)
		{
			this->cond = al_create_cond();
		}

		/// Waits for the condition to be true
		void Wait(Mutex &mutex)
		{
			al_wait_cond(*this, mutex);
		}

		/// Waits for the condition to be true for as long as a specified time
		void Wait(Mutex &mutex, double timeout)
		{
			ALLEGRO_TIMEOUT timeout_;
			al_init_timeout(&timeout_, timeout);
			al_wait_cond_until(*this, mutex, &timeout_);
		}

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_COND *Release()
		{
			this->created = false;
			return *this;
		}

		/// Destroys the condition
		~Condition()
		{
			if (this->created)
				al_destroy_cond(*this);
		}

		/// Returns a pointer to the Allegro condition structure
		operator ALLEGRO_COND *() const
		{
			return this->cond;
		}
};

class Thread;

/// @ingroup Thread
/// Object representing a thread function
class Thread_Proc
{
	public:
		/// Pointer to parent Thread object
		Thread *thread;

		/// Thread code
		virtual void operator()() = 0;
		
		virtual ~Thread_Proc() { }
};

void *thread_proc_call(ALLEGRO_THREAD *al_thread, void *raw_thread);

/// @ingroup Thread
/// Object representing at thread
class Thread
{
	private:
		/// Pointer to the Allegro thread
		ALLEGRO_THREAD *thread;

		/// Whether or not to destroy the thread when the object is destroyed
		bool created;

		// Disable copying
		Thread(const Thread &);
		Thread &operator =(const Thread &);

	public:
		/// Reference to the thread proc
		Thread_Proc *proc;

		/// Creates a thread
		Thread(Thread_Proc &proc_)
			: created(true)
			, proc(&proc_)
		{
			this->thread = al_create_thread(thread_proc_call, this);
		}

		/// Starts a thread
		void Start()
		{
			al_start_thread(this->thread);
		}

		/// Signals that the thread should stop and waits for it to finish
		void Join()
		{
			al_join_thread(this->thread, 0);
		}

		/// Signals that the thread should stop
		void Stop()
		{
			al_set_thread_should_stop(this->thread);
		}

		/// Checks if the stop flag has been set for this thread
		bool ShouldStop() const
		{
			return al_get_thread_should_stop(this->thread);
		}

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_THREAD *Release()
		{
			this->created = false;
			return *this;
		}

		/// Destroys the thread
		/// Automatically joins the thread if it is still running
		virtual ~Thread()
		{
			if (this->created)
				al_destroy_thread(this->thread);
		}

		/// Returns a pointer to the Allegro thread structure
		operator ALLEGRO_THREAD *() const
		{
			return this->thread;
		}
};

inline void *thread_proc_call(ALLEGRO_THREAD *al_thread, void *raw_thread)
{
	Thread *thread = static_cast<Thread *>(raw_thread);
	thread->proc->thread = thread;
	thread->proc->operator()();

	return 0;
}

}

#endif // A5SES_THREAD_HPP_INCLUDED
