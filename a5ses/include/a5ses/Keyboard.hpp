
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Keyboard.hpp
/// @ingroup Keyboard
/// Contains keyboard related classes

#ifndef A5SES_KEYBOARD_HPP_INCLUDED
#define A5SES_KEYBOARD_HPP_INCLUDED

#include "Display.hpp"
#include "Event.hpp"

namespace a5
{

/// @internal
/// @ingroup Internal Keyboard
/// Initializes the Allegro keyboard library
inline void _keyboard_init()
{
	if (!al_is_keyboard_installed())
	{
		_init();

		if (!(al_install_keyboard()))
			throw Init_Failed("keyboard");
	}
}

/// @ingroup Keyboard
/// Object representing a keyboard
class Keyboard : public Event_Source
{
	public:
		struct Key
		{
			enum KeyCode
			{
				Invalid = -1,
				A = ALLEGRO_KEY_A,
				B = ALLEGRO_KEY_B,
				C = ALLEGRO_KEY_C,
				D = ALLEGRO_KEY_D,
				E = ALLEGRO_KEY_E,
				F = ALLEGRO_KEY_F,
				G = ALLEGRO_KEY_G,
				H = ALLEGRO_KEY_H,
				I = ALLEGRO_KEY_I,
				J = ALLEGRO_KEY_J,
				K = ALLEGRO_KEY_K,
				L = ALLEGRO_KEY_L,
				M = ALLEGRO_KEY_M,
				N = ALLEGRO_KEY_N,
				O = ALLEGRO_KEY_O,
				P = ALLEGRO_KEY_P,
				Q = ALLEGRO_KEY_Q,
				R = ALLEGRO_KEY_R,
				S = ALLEGRO_KEY_S,
				T = ALLEGRO_KEY_T,
				U = ALLEGRO_KEY_U,
				V = ALLEGRO_KEY_V,
				W = ALLEGRO_KEY_W,
				X = ALLEGRO_KEY_X,
				Y = ALLEGRO_KEY_Y,
				Z = ALLEGRO_KEY_Z,
				Key0 = ALLEGRO_KEY_0,
				Key1 = ALLEGRO_KEY_1,
				Key2 = ALLEGRO_KEY_2,
				Key3 = ALLEGRO_KEY_3,
				Key4 = ALLEGRO_KEY_4,
				Key5 = ALLEGRO_KEY_5,
				Key6 = ALLEGRO_KEY_6,
				Key7 = ALLEGRO_KEY_7,
				Key8 = ALLEGRO_KEY_8,
				Key9 = ALLEGRO_KEY_9,
				Pad0 = ALLEGRO_KEY_PAD_0,
				Pad1 = ALLEGRO_KEY_PAD_1,
				Pad2 = ALLEGRO_KEY_PAD_2,
				Pad3 = ALLEGRO_KEY_PAD_3,
				Pad4 = ALLEGRO_KEY_PAD_4,
				Pad5 = ALLEGRO_KEY_PAD_5,
				Pad6 = ALLEGRO_KEY_PAD_6,
				Pad7 = ALLEGRO_KEY_PAD_7,
				Pad8 = ALLEGRO_KEY_PAD_8,
				Pad9 = ALLEGRO_KEY_PAD_9,
				F1 = ALLEGRO_KEY_F1,
				F2 = ALLEGRO_KEY_F2,
				F3 = ALLEGRO_KEY_F3,
				F4 = ALLEGRO_KEY_F4,
				F5 = ALLEGRO_KEY_F5,
				F6 = ALLEGRO_KEY_F6,
				F7 = ALLEGRO_KEY_F7,
				F8 = ALLEGRO_KEY_F8,
				F9 = ALLEGRO_KEY_F9,
				F10 = ALLEGRO_KEY_F10,
				F11 = ALLEGRO_KEY_F11,
				F12 = ALLEGRO_KEY_F12,
				Escape = ALLEGRO_KEY_ESCAPE,
				Tilde = ALLEGRO_KEY_TILDE,
				Minus = ALLEGRO_KEY_MINUS,
				Equals = ALLEGRO_KEY_EQUALS,
				Backspace = ALLEGRO_KEY_BACKSPACE,
				Tab = ALLEGRO_KEY_TAB,
				OpenBrace = ALLEGRO_KEY_OPENBRACE,
				CloseBrace = ALLEGRO_KEY_CLOSEBRACE,
				Enter = ALLEGRO_KEY_ENTER,
				Semicolon = ALLEGRO_KEY_SEMICOLON,
				Quote = ALLEGRO_KEY_QUOTE,
				Backslash = ALLEGRO_KEY_BACKSLASH,
				Blackslash2 = ALLEGRO_KEY_BACKSLASH2,
				Comma = ALLEGRO_KEY_COMMA,
				FullStop = ALLEGRO_KEY_FULLSTOP,
				Slash = ALLEGRO_KEY_SLASH,
				Space = ALLEGRO_KEY_SPACE,
				Insert = ALLEGRO_KEY_INSERT,
				Delete = ALLEGRO_KEY_DELETE,
				Home = ALLEGRO_KEY_HOME,
				End = ALLEGRO_KEY_END,
				PgUp = ALLEGRO_KEY_PGUP,
				PgDn = ALLEGRO_KEY_PGDN,
				Left = ALLEGRO_KEY_LEFT,
				Right = ALLEGRO_KEY_RIGHT,
				Up = ALLEGRO_KEY_UP,
				Down = ALLEGRO_KEY_DOWN,
				PadSlash = ALLEGRO_KEY_PAD_SLASH,
				PadAsterisk = ALLEGRO_KEY_PAD_ASTERISK,
				PadMinus = ALLEGRO_KEY_PAD_MINUS,
				PadPlus = ALLEGRO_KEY_PAD_PLUS,
				PadDelete = ALLEGRO_KEY_PAD_DELETE,
				PadEnter = ALLEGRO_KEY_PAD_ENTER,
				PrintScreen = ALLEGRO_KEY_PRINTSCREEN,
				Pause = ALLEGRO_KEY_PAUSE,
				ABNT_C1 = ALLEGRO_KEY_ABNT_C1,
				Yen = ALLEGRO_KEY_YEN,
				Kana = ALLEGRO_KEY_KANA,
				Convert = ALLEGRO_KEY_CONVERT,
				NoConvert = ALLEGRO_KEY_NOCONVERT,
				At = ALLEGRO_KEY_AT,
				Circumflex = ALLEGRO_KEY_CIRCUMFLEX,
				Colon2 = ALLEGRO_KEY_COLON2,
				Kanji = ALLEGRO_KEY_KANJI,
				LeftShift = ALLEGRO_KEY_LSHIFT,
				RightShift = ALLEGRO_KEY_RSHIFT,
				LeftCtrl = ALLEGRO_KEY_LCTRL,
				RightCtrl = ALLEGRO_KEY_RCTRL,
				Alt = ALLEGRO_KEY_ALT,
				AltGr = ALLEGRO_KEY_ALTGR,
				LeftWin = ALLEGRO_KEY_LWIN,
				RightWin = ALLEGRO_KEY_RWIN,
				Menu = ALLEGRO_KEY_MENU,
				ScrollLock = ALLEGRO_KEY_SCROLLLOCK,
				NumLock = ALLEGRO_KEY_NUMLOCK,
				CapsLock = ALLEGRO_KEY_CAPSLOCK,
				BackQuote = ALLEGRO_KEY_BACKQUOTE,
				Semicolon2 = ALLEGRO_KEY_SEMICOLON2,
				Command = ALLEGRO_KEY_COMMAND
			};
		};

		struct Modifier
		{
			enum KeyMod
			{
				Invalid = -1,
				Shift = ALLEGRO_KEYMOD_SHIFT,
				Ctrl = ALLEGRO_KEYMOD_CTRL,
				Alt = ALLEGRO_KEYMOD_ALT,
				LWin = ALLEGRO_KEYMOD_LWIN,
				RWin = ALLEGRO_KEYMOD_RWIN,
				Menu = ALLEGRO_KEYMOD_MENU,
				AltGr = ALLEGRO_KEYMOD_ALTGR,
				Command = ALLEGRO_KEYMOD_COMMAND,
				ScrollLock = ALLEGRO_KEYMOD_SCROLLLOCK,
				NumLock = ALLEGRO_KEYMOD_NUMLOCK,
				CapsLock = ALLEGRO_KEYMOD_CAPSLOCK,
				InAltsEq = ALLEGRO_KEYMOD_INALTSEQ,
				Accent1 = ALLEGRO_KEYMOD_ACCENT1,
				Accent2 = ALLEGRO_KEYMOD_ACCENT2,
				Accent3 = ALLEGRO_KEYMOD_ACCENT3,
				Accent4 = ALLEGRO_KEYMOD_ACCENT4
			};
		};

		/// @ingroup Event Keyboard
		/// Keyboard event object
		class Event : public a5::Event
		{
			public:
				// Keyboard event Type ID
				static const unsigned int Type = 0x00000003;

				/// Event subtypes
				enum Sub
				{
					Down = Type * 0x10000000 + 1,
					Char = Type * 0x10000000 + 2,
					Up   = Type * 0x10000000 + 3
				};

			private:
				/// Event subtype
				Sub subtype;

			public:
				/// Display the keyboard is using
				ALLEGRO_DISPLAY *display;

				/// Key code
				Keyboard::Key::KeyCode keycode;

				/// Modifier key
				Keyboard::Modifier::KeyMod modifier;

				/// Character representation (mapped)
				unsigned int unichar;

				/// Returns the subtype ID of the event
				unsigned int SubType() const
				{
					return subtype;
				}

				/// Creates a Keyboard event
				Event(const Event_Source *source_, Sub subtype_, ALLEGRO_DISPLAY *display_, Keyboard::Key::KeyCode keycode_, Keyboard::Modifier::KeyMod modifier_, unsigned int unichar_)
					: a5::Event(source_, Type)
					, subtype(subtype_)
					, display(display_)
					, keycode(keycode_)
					, modifier(modifier_)
					, unichar(unichar_)
				{ }
		};

		/// Creates a Keyboard
		Keyboard()
		{
			_keyboard_init();
			SetEventSource(al_get_keyboard_event_source());
		}

		std::unique_ptr<a5::Event> Handle(ALLEGRO_EVENT *raw_event) const
		{
			Event::Sub subtype = static_cast<Event::Sub>(0);
			Key::KeyCode keycode = Key::Invalid;
			Modifier::KeyMod modifier = Modifier::Invalid;

			switch (raw_event->type)
			{
				case ALLEGRO_EVENT_KEY_DOWN:
					subtype = Event::Down;
					keycode = Key::KeyCode(raw_event->keyboard.keycode);
					break;

				case ALLEGRO_EVENT_KEY_CHAR:
					subtype = Event::Char;
					keycode = Key::KeyCode(raw_event->keyboard.keycode);
					break;

				case ALLEGRO_EVENT_KEY_UP:
					subtype = Event::Up;
					keycode = Key::KeyCode(raw_event->keyboard.keycode);
					break;
			}

			return std::unique_ptr<a5::Event>(new Event(this, subtype, raw_event->keyboard.display,
				keycode, modifier, raw_event->keyboard.unichar));
		}
};

}

#endif // A5SES_KEYBOARD_HPP_INCLUDED
