
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Mouse.hpp
/// @ingroup Mouse
/// Contains mouse related classes

#ifndef A5SES_MOUSE_HPP_INCLUDED
#define A5SES_MOUSE_HPP_INCLUDED

#include "Display.hpp"
#include "Event.hpp"

namespace a5
{

/// @internal
/// @ingroup Internal Mouse
/// Initializes the Allegro mouse library
inline void _mouse_init()
{
	if (!al_is_mouse_installed())
	{
		_init();

		if (!(al_install_mouse()))
			throw Init_Failed("mouse");
	}
}

/// @ingroup Mouse
/// Object representing a mouse
class Mouse : public Event_Source
{
	public:
		enum Button
		{
			Invalid = 0,
			Left = 1,
			Right = 2,
			Middle = 3,
		};

		enum Axis
		{
			X,
			Y,
			Z,
			W
		};

		/// @ingroup Event Mouse
		/// Mouse event object
		class Event : public a5::Event
		{
			public:
				// Keyboard event Type ID
				static const unsigned int Type = 0x00000004;

				/// Event subtypes
				enum Sub
				{
					Move  = Type * 0x10000000 + 1,
					Down  = Type * 0x10000000 + 2,
					Up    = Type * 0x10000000 + 3,
					Warp  = Type * 0x10000000 + 4,
					Enter = Type * 0x10000000 + 5,
					Leave = Type * 0x10000000 + 6
				};

			private:
				/// Event subtype
				Sub subtype;

			public:
				/// Display the mouse is using
				ALLEGRO_DISPLAY *display;

				/// Current position of the horizontal scroll
				int w;

				/// Current X coordinate of the mouse cursor
				int x;

				/// Current Y coordinate of the mouse cursor
				int y;

				/// Current position of the vertical scroll
				int z;

				/// Difference this event caused to the position of the horizontal scroll
				int dw;

				/// Difference this event caused to the X coordinate of the mouse cursor
				int dx;

				/// Difference this event caused to the Y coordinate of the mouse cursor
				int dy;

				/// Difference this event caused to the position of the vertical scroll
				int dz;

				/// Button that was clicked
				Button button;

				/// Returns the subtype ID of the event
				unsigned int SubType() const
				{
					return subtype;
				}

				/// Creates a Mouse event
				Event(const Event_Source *source_, Sub subtype_, ALLEGRO_DISPLAY *display_,
					int w_, int x_, int y_, int z_, int dw_, int dx_, int dy_, int dz_, Button button_)
					: a5::Event(source_, Type)
					, subtype(subtype_)
					, display(display_)
					, w(w_)
					, x(x_)
					, y(y_)
					, z(z_)
					, dw(dw)
					, dx(dx_)
					, dy(dy_)
					, dz(dz_)
					, button(button_)
				{ }
		};

		/// @ingroup Event Mouse
		/// Mouse state
		class State
		{
			public:
				/// Position of the horizontal scroll
				int w;

				/// X coordinate of the mouse cursor
				int x;

				/// Y coordinate of the mouse cursor
				int y;

				/// Position of the vertical scroll
				int z;

				/// Bitmask of buttons held down
				int buttons;

				/// Creates a Mouse State from an Allegro mouse state
				State(const ALLEGRO_MOUSE_STATE &state)
					: w(state.w)
					, x(state.x)
					, y(state.y)
					, z(state.z)
					, buttons(state.buttons)
				{ }

				/// Returns the Allegro mouse state
				operator ALLEGRO_MOUSE_STATE() const
				{
					ALLEGRO_MOUSE_STATE state;

					state.w = this->w;
					state.x = this->x;
					state.y = this->y;
					state.z = this->z;
					state.buttons = this->buttons;

					return state;
				}
		};

		/// @ingroup Event Mouse
		/// Mouse cursor
		class Cursor
		{
			private:
				/// Pointer to Allegro mouse cursor
				ALLEGRO_MOUSE_CURSOR *cursor;

			public:
				/// Creates a mouse cursor from a Bitmap
				Cursor(Bitmap &bmp, int focus_x, int focus_y)
				{
					this->cursor = al_create_mouse_cursor(bmp, focus_x, focus_y);
				}

				/// Destroys the mouse cursor
				~Cursor()
				{
					al_destroy_mouse_cursor(*this);
				}

				/// Returns a pointer to the Allegro mouse cursor
				operator ALLEGRO_MOUSE_CURSOR *()
				{
					return this->cursor;
				}
		};

		/// Creates a Mouse
		Mouse()
		{
			_mouse_init();
			SetEventSource(al_get_mouse_event_source());
		}

		/// Takes a snapshot of the current state of the mouse
		a5::Mouse::State GetState() const
		{
			ALLEGRO_MOUSE_STATE state;
			al_get_mouse_state(&state);
			return a5::Mouse::State(state);
		}

		/// Restores the coordinates to a previous State
		void SetState(a5::Display &display, a5::Mouse::State state)
		{
			al_set_mouse_xy(display, state.x, state.y);
			al_set_mouse_w(state.w);
			al_set_mouse_w(state.z);
		}

		/// Changes the mouse cursor image
		bool SetCursor(Display &display, Cursor &cursor)
		{
			return al_set_mouse_cursor(display, cursor);
		}

		std::unique_ptr<a5::Event> Handle(ALLEGRO_EVENT *raw_event) const
		{
			Event::Sub subtype = static_cast<Event::Sub>(0);

			switch (raw_event->type)
			{
				case ALLEGRO_EVENT_MOUSE_AXES:
					subtype = Event::Move;
					break;

				case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
					subtype = Event::Down;
					break;

				case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
					subtype = Event::Up;
					break;

				case ALLEGRO_EVENT_MOUSE_WARPED:
					subtype = Event::Warp;
					break;

				case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
					subtype = Event::Enter;
					break;

				case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
					subtype = Event::Leave;
					break;
			}

			return std::unique_ptr<a5::Event>(new Event(this, subtype, raw_event->mouse.display,
				raw_event->mouse.w, raw_event->mouse.x, raw_event->mouse.y, raw_event->mouse.z,
				raw_event->mouse.dw, raw_event->mouse.dx, raw_event->mouse.dy, raw_event->mouse.dz,
				static_cast<Button>(raw_event->mouse.button)));
		}
};

}

#endif // A5SES_MOUSE_HPP_INCLUDED
