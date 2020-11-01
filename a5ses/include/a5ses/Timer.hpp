
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Timer.hpp
/// @ingroup Timer
/// Contains time and timing related classes and functions

#ifndef A5SES_TIMER_HPP_INCLUDED
#define A5SES_TIMER_HPP_INCLUDED

#include "common.hpp"
#include "Event.hpp"

namespace a5
{

class Timer : public Event_Source
{
	private:
		/// Pointer to the Allegro timer
		ALLEGRO_TIMER *timer;

		/// Whether or not to destroy the timer when the object is destroyed
		bool created;

		// Disable copying
		Timer(const Timer &);
		Timer &operator =(const Timer &);

	public:
		/// @ingroup Event Timer
		/// Timer event object
		class Event : public a5::Event
		{
			public:
				// Timer event Type ID
				static const unsigned int Type = 0x00000001;
				
				/// Returns the subtype ID of the event
				unsigned int SubType() const
				{
					return Type;
				}

				/// Creates a Timer event
				Event(const Event_Source *source_)
					: a5::Event(source_, Type)
				{ }
		};

		/// Creates a new timer with the specified rate
		Timer(double hz)
			: created(true)
		{
			_init();
			this->timer = al_create_timer(1.0 / hz);
			SetEventSource(al_get_timer_event_source(*this));
		}

		/// Start a timer
		void Start()
		{
			al_start_timer(*this);
		}

		/// Stops a timer
		void Stop()
		{
			al_stop_timer(*this);
		}

		std::unique_ptr<a5::Event> Handle(ALLEGRO_EVENT *raw_event) const
		{
			return std::unique_ptr<a5::Event>(new Event(this));
		}

		operator ALLEGRO_TIMER *() const
		{
			return this->timer;
		}
		
		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_TIMER *Release()
		{
			this->created = false;
			return *this;
		}

		virtual ~Timer()
		{
			if (this->created)
				al_destroy_timer(*this);
		}
};

/// @ingroup Timer
/// Pauses the thread for the specified number of seconds.
inline void Rest(double seconds)
{
	al_rest(seconds);
}

/// @ingroup Timer
/// Returns the number of seconds since Allegro was initialized
inline double Time()
{
	_init();
	return al_current_time();
}

}

#endif // A5SES_TIMER_HPP_INCLUDED
