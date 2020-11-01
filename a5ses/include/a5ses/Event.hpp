
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Event.hpp
/// @ingroup Event
/// Contains generic event queue related classes

#ifndef A5SES_EVENT_HPP_INCLUDED
#define A5SES_EVENT_HPP_INCLUDED

#include <cstring>
#include <set>

#include "common.hpp"

namespace a5
{

class Event_Queue;
class Event;

/// @ingroup Event
/// An Event_Source can be registered to an Event_Queue
class Event_Source
{
	private:
		/// Pointer to the Allegro event source
		ALLEGRO_EVENT_SOURCE *event_source;

		// Disable copying
		Event_Source(const Event_Source &);
		Event_Source &operator =(const Event_Source &);

	public:
		/// Processes Allegro event messages in to A5SES Events
		virtual std::unique_ptr<Event> Handle(ALLEGRO_EVENT *raw_event) const = 0;

		/// Creates an invalid Event_Source object
		/// SetEventSource must be called before the object is valid
		Event_Source()
		{
			_init();
		}

		/// Points the Allegro event source towards this object
		Event_Source(ALLEGRO_EVENT_SOURCE *event_source_)
			: event_source(event_source_)
		{
			_init();
			al_set_event_source_data(*this, reinterpret_cast<intptr_t>(this));
		}

		/// Points the Allegro event source towards this object
		void SetEventSource(ALLEGRO_EVENT_SOURCE *event_source)
		{
			this->event_source = event_source;
			al_set_event_source_data(*this, reinterpret_cast<intptr_t>(this));
		}

		/// Returns a pointer to the Allegro event source
		operator ALLEGRO_EVENT_SOURCE *() const
		{
			return this->event_source;
		}

		/// Customary virtual destructor for base types
		virtual ~Event_Source() { }
};

/// @ingroup Event
/// Base class for event messages
class Event
{
	private:
		/// Type ID of the event
		unsigned int type_id;

		// Disable copying
		Event(const Event &);
		Event &operator =(const Event &);

	public:
		// User event Type ID
		static const unsigned int User_Type = 0x00000000;

		/// @internal
		/// Pointer to the raw Allegro event
		bool has_raw = false;
		ALLEGRO_EVENT raw;

		/// Pointer to the Event_Source that emitted the event
		const Event_Source *source;

		/// Sets the event source
		Event(ALLEGRO_EVENT* raw_event)
			: type_id(0)
			, has_raw(true)
			, source(nullptr)
		{
			memcpy(&raw, raw_event, sizeof raw);
		}

		/// Sets the event source
		Event(const Event_Source *source_, int type_id_)
			: type_id(type_id_)
			, source(source_)
		{ }
		
		/// Returns the Type ID of the event
		/// Returns 0 (User_Type) for all user events
		unsigned int Type() const
		{
			return type_id;
		}
		
		/// Returns the subtype ID of the event
		/// Returns the same as Type() for event types with no sub-types
		/// Returns 0 for all user events
		virtual unsigned int SubType() const
		{
			return 0;
		}
		/// Customary virtual destructor for base types
		virtual ~Event() { }
};

/// @ingroup Event
/// Processes events from Event_Sources
class Event_Queue
{
	private:
		/// Pointer to the Allegro event queue
		ALLEGRO_EVENT_QUEUE *event_queue;

		/// Set of pointers to registered event sources
		std::set<Event_Source *> sources;

		/// Whether or not to destroy the event queue when the object is destroyed
		bool created;

		// Disable copying
		Event_Queue(const Event_Queue &);
		Event_Queue &operator =(const Event_Queue &);

	public:
		/// Creates a new event queue
		Event_Queue()
			: created(true)
		{
			_init();
			this->event_queue = al_create_event_queue();
		}

		/// Checks a raw ALLEGRO_EVENT through the event source handlers
		/// Returns NULL if no source handlers caused the event
		std::unique_ptr<Event> Handle(ALLEGRO_EVENT *raw_event) const
		{
			std::unique_ptr<Event> event = std::unique_ptr<Event>(nullptr);
			ALLEGRO_EVENT_SOURCE *raw_event_source = raw_event->any.source;

			for (std::set<Event_Source *>::const_iterator i = this->sources.begin(); i != this->sources.end(); ++i)
			{
				if (reinterpret_cast<const Event_Source *>(al_get_event_source_data(raw_event_source)) == *i)
				{
					event = (*i)->Handle(raw_event);
					break;
				}
			}

			if (!event)
				event.reset(new Event(raw_event));

			return event;
		}

		/// Waits forever for an event
		std::unique_ptr<Event> Wait()
		{
			ALLEGRO_EVENT raw_event;

			al_wait_for_event(*this, &raw_event);
			std::unique_ptr<Event> event = this->Handle(&raw_event);

			if (event.get())
				return event;
			else
				return this->Wait();
		}

		/// Waits for an event for a specified number of seconds
		/// Returns NULL if the timer expires
		std::unique_ptr<Event> Wait(double seconds)
		{
			ALLEGRO_EVENT event;

			if (al_wait_for_event_timed(*this, &event, seconds))
				return this->Handle(&event);
			else
				return std::unique_ptr<Event>(nullptr);
		}

		/// Checks for and returns an event. Returns NULL if the queue is empty
		std::unique_ptr<Event> Get()
		{
			ALLEGRO_EVENT event;

			if (al_get_next_event(*this, &event))
				return this->Handle(&event);
			else
				return std::unique_ptr<Event>(nullptr);
		}

		/// Checks for and returns an event. The event remains in the queue.
		/// Returns NULL if the queue is empty
		std::unique_ptr<Event> Peek()
		{
			ALLEGRO_EVENT event;

			if (al_peek_next_event(*this, &event))
				return this->Handle(&event);
			else
				return std::unique_ptr<Event>(nullptr);
		}

		/// Returns true if the event queue is empty
		bool Empty() const
		{
			return al_event_queue_is_empty(*this);
		}

		/// Removes all events from the queue
		void Flush()
		{
			al_flush_event_queue(*this);
		}

		/// Removes the next even from the queue. Returns true if an event was dropped
		bool Drop()
		{
			return al_drop_next_event(*this);
		}

		/// Registers an event source
		void Register(Event_Source &source)
		{
			this->sources.insert(&source);
			al_register_event_source(*this, source);
		}

		/// Unregisters an event source
		void Unregister(Event_Source &source)
		{
			this->sources.erase(&source);
			al_unregister_event_source(*this, source);
		}
		
		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_EVENT_QUEUE *Release()
		{
			this->created = false;
			return *this;
		}

		/// Unregisters any event sources and frees resources
		~Event_Queue()
		{
			if (this->created)
			{
				for (std::set<Event_Source *>::iterator i = this->sources.begin(); i != this->sources.end(); ++i)
				{
					al_unregister_event_source(*this, *(*i));
				}

				al_destroy_event_queue(*this);
			}
		}

		/// Returns a pointer to the Allegro event queue structure
		operator ALLEGRO_EVENT_QUEUE *() const
		{
			return this->event_queue;
		}
};

}

#endif // A5SES_EVENT_HPP_INCLUDED
