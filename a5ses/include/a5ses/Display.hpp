
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Display.hpp
/// @ingroup Display
/// Contains display related classes

#ifndef A5SES_DISPLAY_HPP_INCLUDED
#define A5SES_DISPLAY_HPP_INCLUDED

#include "common.hpp"
#include "Bitmap.hpp"
#include "Event.hpp"

namespace a5
{

extern bool disable_auto_target;

/// @ingroup Display
/// Object representing a display (screen or window)
class Display : public Bitmap, public Event_Source
{
	private:
		/// Pointer to the Allegro display
		ALLEGRO_DISPLAY *display;

		/// Whether or not to destroy the display when the object is destroyed
		bool created;

		// Disable copying
		Display(const Display &);
		Display &operator =(const Display &);

	public:
		/// Creates the display as a window
		static const int Windowed = ALLEGRO_WINDOWED;

		/// Creates the display fullscreen
		static const int Fullscreen = ALLEGRO_FULLSCREEN;

		/// Allows the display to be resized
		/// Only effective in Windowed mode without NoFrame
		static const int Resizable = ALLEGRO_RESIZABLE;

		/// Removes the window border
		/// Only effective in Windowed mode
		static const int NoFrame = ALLEGRO_NOFRAME;

		/// Generates ExposeEvent events when parts of the screen are "exposed"
		static const int GenerateExposeEvents = ALLEGRO_GENERATE_EXPOSE_EVENTS;

		/// Makes Flip() wait for VSYNC before drawing
		static const int VSync = 0x80000000;

		/// @ingroup Event Display
		/// Display event object
		class Event : public a5::Event
		{
			public:
				// Display event Type ID
				static const unsigned int Type = 0x00000002;

				/// Event subtypes
				enum Sub
				{
					Close     = Type * 0x10000000 + 1,
					Resize    = Type * 0x10000000 + 2,
					Expose    = Type * 0x10000000 + 3,
					Lost      = Type * 0x10000000 + 4,
					Found     = Type * 0x10000000 + 5,
					SwitchOut = Type * 0x10000000 + 6,
					SwitchIn  = Type * 0x10000000 + 7
				};

			private:
				/// Raw event subtype
				Sub subtype;

			public:
				int x;
				int y;
				int width;
				int height;
				
				/// Returns the subtype ID of the event
				unsigned int SubType() const
				{
					return subtype;
				}

				/// Creates a Display event
				Event(const Event_Source *source_, Sub subtype_, int x_, int y_, int width_, int height_)
					: a5::Event(source_, Type)
					, subtype(subtype_)
					, x(x_)
					, y(y_)
					, width(width_)
					, height(height_)
				{ }
		};

		/// Creates a display with the specified width, height and flags
		/// flags is compatable with allegro5
		Display(unit width, unit height, int flags)
			: created(true)
		{
			if (width < 0 || height < 0)
				throw std::invalid_argument("Tried to create display with negative width/height");

			_init();

			bool vsync = flags & VSync;
			flags &= ~VSync;

			al_set_new_display_option(ALLEGRO_VSYNC, vsync ? 1 : 2, ALLEGRO_SUGGEST);
			al_set_new_display_flags(flags);

			this->display = al_create_display(width, height);
			this->bitmap = al_get_backbuffer(this->display);

			flags = al_get_display_flags(this->display);

			SetEventSource(al_get_display_event_source(*this));
		}

		/// Returns the current width of the display
		unit Width() const
		{
			return al_get_display_width(this->display);
		}

		/// Returns the current height of the display
		unit Height() const
		{
			return al_get_display_height(this->display);
		}

		/// Returns the refresh rate of the display
		int RefreshRate() const
		{
			return al_get_display_refresh_rate(this->display);
		}

		///Returns the display flags
		int Flags() const
		{
			int flags = al_get_new_display_flags();

			return flags;
		}

		/// Forces the thread to wait for VSYNC
		bool WaitForVSync()
		{
			if (!disable_auto_target)
				this->Target();
			return al_wait_for_vsync();
		}

		/// Flips the display
		void Flip()
		{
			if (!disable_auto_target)
				this->Target();
			al_flip_display();
		}

		/// Targets the display backbuffer
		/// Shouldn't need to be called if using pure A5SES
		void Target()
		{
			Bitmap::Target();
		}

		/// Sets the window title. No effect in fullscreen
		void SetTitle(const char *title)
		{
			al_set_window_title(this->display, title);
		}

		/// Resizes the display
		bool Resize(unit width, unit height)
		{
			return al_resize_display(this->display, width, height);
		}

		/// Acknowledges a Resize event
		bool AcknowledgeResize()
		{
			return al_acknowledge_resize(*this);
		}

		/// Moves the display
		void Move(unit x, unit y)
		{
			al_set_window_position(this->display, x, y);
		}

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_DISPLAY *Release()
		{
			this->created = false;
			return *this;
		}

		/// Destroys the display
		~Display()
		{
			if (this->created)
				al_destroy_display(*this);
		}

		std::unique_ptr<a5::Event> Handle(ALLEGRO_EVENT *raw_event) const
		{
			Event::Sub subtype = static_cast<Event::Sub>(0);

			switch (raw_event->type)
			{
				case ALLEGRO_EVENT_DISPLAY_CLOSE:
					subtype = Event::Close;
					break;

				case ALLEGRO_EVENT_DISPLAY_RESIZE:
					subtype = Event::Resize;
					break;

				case ALLEGRO_EVENT_DISPLAY_EXPOSE:
					subtype = Event::Expose;
					break;

				case ALLEGRO_EVENT_DISPLAY_LOST:
					subtype = Event::Lost;
					break;

				case ALLEGRO_EVENT_DISPLAY_FOUND:
					subtype = Event::Found;
					break;

				case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
					subtype = Event::SwitchOut;
					break;

				case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
					subtype = Event::SwitchIn;
					break;
			}

			return std::unique_ptr<a5::Event>(new Event(this, subtype, raw_event->display.x,
				raw_event->display.y, raw_event->display.width, raw_event->display.height));
		}

		/// Returns a pointer to the Allegro display struture
		operator ALLEGRO_DISPLAY *()
		{
			return this->display;
		}
};

}

#endif // A5SES_DISPLAY_HPP_INCLUDED
