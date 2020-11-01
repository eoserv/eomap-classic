
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Event_Reader.hpp
/// @ingroup Event
/// Simpler event reader interface

#ifndef A5SES_EVENT_READER_HPP_INCLUDED
#define A5SES_EVENT_READER_HPP_INCLUDED

#include "common.hpp"
#include "Event.hpp"
#include "Timer.hpp"
#include "Display.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

namespace a5
{

/// @ingroup Event
/// Simpler event reader interface
class Event_Reader
{
	public:
		/// @ingroup Exception Event
		/// Thrown when no event was stored in the reader
		class No_Event : public Exception
		{
			public:
				virtual const char *what() const throw()
				{
					return "[a5::Event_Reader::No_Event] Tried to access event when none was stored.";
				}

				virtual ~No_Event() throw() { }
		};

	private:
		/// Reference to the event queue the reader wraps
		Event_Queue &queue;

		/// Pointer to the current event
		std::unique_ptr<a5::Event> event;

		// Disable copying
		Event_Reader(const Event_Reader &);
		Event_Reader &operator =(const Event_Reader &);

	public:
		/// Create an event reader
		Event_Reader(Event_Queue &queue_)
			: queue(queue_)
		{ }

		/// Returns the wrapped queue
		Event_Queue &Queue() const
		{
			return queue;
		}

		/// Checks the queue for a new event
		/// Returns true if an event was waiting, false otherwise
		bool Next()
		{
			event = queue.Get();
			return HasEvent();
		}

		/// Waits for a new event
		void WaitNext()
		{
			event = queue.Wait();
		}

		/// Waits the specified time for a new event
		/// Returns true if an event was waiting, false otherwise
		bool WaitNext(double seconds)
		{
			event = queue.Wait(seconds);
			return HasEvent();
		}

		/// Returns true if Get() and friends return a valid pointer
		bool HasEvent() const
		{
			return event.get() != 0;
		}

		/// Gets the latest event as a reference to the specified type
		template <class T> T &Get() const
		{
			if (!HasEvent())
				throw No_Event();

			return *static_cast<T *>(event.get());
		}

		/// Gets the latest event as a reference to Event
		a5::Event &Event() const
		{
			return Get<a5::Event>();
		}

		/// Gets the latest event as a reference to Timer::Event
		a5::Timer::Event &Timer() const
		{
			return Get<a5::Timer::Event>();
		}

		/// Gets the latest event as a reference to Display::Event
		a5::Display::Event &Display() const
		{
			return Get<a5::Display::Event>();
		}

		/// Gets the latest event as a reference to Keyboard::Event
		a5::Keyboard::Event &Keyboard() const
		{
			return Get<a5::Keyboard::Event>();
		}

		/// Gets the latest event as a reference to Mouse::Event
		a5::Mouse::Event &Mouse() const
		{
			return Get<a5::Mouse::Event>();
		}

		/// Gets the Event type
		unsigned int Type() const
		{
			if (!HasEvent())
				throw No_Event();

			return event->Type();
		}

		/// Gets the Event sub-type
		unsigned int SubType() const
		{
			if (!HasEvent())
				throw No_Event();

			return event->SubType();
		}
};

}

#endif // A5SES_EVENT_READER_HPP_INCLUDED
