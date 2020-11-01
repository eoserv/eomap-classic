
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file common.hpp
/// @ingroup Common
/// Contains classes and functions several a5ses components need

#ifndef A5SES_COMMON_HPP_INCLUDED
#define A5SES_COMMON_HPP_INCLUDED

#ifdef Rectangle
#undef Rectangle
#endif

#define Rectangle WINDOWS_Rectangle
#include <allegro5/allegro.h>
#undef Rectangle

#ifdef A5SES_IMAGE_ADDON
#include <allegro5/allegro_image.h>
#endif

#ifdef A5SES_FONT_ADDON

#include <allegro5/allegro_font.h>

#ifdef A5SES_TTF_ADDON
#include <allegro5/allegro_ttf.h>
#endif

#endif // A5SES_FONT_ADDON

#if defined(A5SES_AUDIO_ADDON) || defined(A5SES_FLAC_ADDON) || defined(A5SES_VORBIS_ADDON)

#include <allegro5/allegro_audio.h>

#ifdef A5SES_FLAC_ADDON
#include <allegro5/allegro_flac.h>
#endif

#ifdef A5SES_VORBIS_ADDON
#include <allegro5/allegro_vorbis.h>
#endif

#endif // defined(A5SES_AUDIO_ADDON) || defined(A5SES_FLAC_ADDON) || defined(A5SES_VORBIS_ADDON)

#ifdef RGB
#undef RGB
#ifndef WINDOWS_RGB
#define WINDOWS_RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif
#endif // RGB

/// @ingroup Common
/// A5SES version string
/// First 3 digits should match the Allegro version it's made for
#define A5SES_VERSION_STR "4.9.21.2"

/// @ingroup Common
/// A5SES major version
/// Should match the Allegro version it's made for
#define A5SES_MAJOR_VERSION 5

/// @ingroup Common
/// A5SES minor version
/// Should match the Allegro version it's made for
#define A5SES_MINOR_VERSION 2

/// @ingroup Common
/// A5SES WIP version
/// Should match the Allegro version it's made for
#define A5SES_WIP_VERSION 5

/// @ingroup Common
/// A5SES version
#define A5SES_VERSION 1

#if (A5SES_MAJOR_VERSION != ALLEGRO_VERSION) || (A5SES_MINOR_VERSION != ALLEGRO_SUB_VERSION)
#warning Allegro version does not match the A5SES version
#endif

#include <exception>
#include <memory>
#include <string>
#include <stdexcept>

/// Contains all A5SES classes and functions
namespace a5
{

/// @ingroup Common
/// Type used to represent bitmap/screen coordinates
#ifdef A5SES_INT_COORD
typedef int unit;
#else
typedef float unit;
#endif

/// @ingroup Common
/// Type used to represent angles
typedef float angle;

/// @ingroup Exception Common
/// Base class for A5SES exceptions
class Exception : public std::exception
{
	public:
		/// Returns the name of the exception as a string
		virtual const char *what() const throw()
		{
			return "[a5::Exception]";
		}

		virtual ~Exception() throw() { }
};

/// @ingroup Exception Common
/// Thrown when a function requiring a non-loaded addon is called
class Need_Addon : public Exception
{
	private:
		const char *addon_;
		std::string str;

	public:
		/// Creates a Need_Addon exception
		Need_Addon(const char *addon__)
			: addon_(addon__)
		{
			ALLEGRO_USTR *ustr = al_ustr_newf(
				"[a5::Need_Addon] Addon is not available: %s",
				addon());
			str = al_cstr(ustr);
			al_ustr_free(ustr);
		}

		virtual const char *what() const throw()
		{
			return str.c_str();
		}

		/// Returns the name of the missing addon
		const char *addon() const
		{
			return this->addon_;
		}

		virtual ~Need_Addon() throw() { }
};

/// @ingroup Exception Common
/// Thrown when an core Allegro module or addon fails to initialize
class Init_Failed : public Exception
{
	private:
		const char *addon_;
		std::string str;

	public:
		/// Creates a Need_Addon exception
		Init_Failed(const char *addon__)
			: addon_(addon__)
		{
			ALLEGRO_USTR *ustr = al_ustr_newf(
				"[a5::Init_Failed] Initialization failed: %s",
				addon());
			str = al_cstr(ustr);
			al_ustr_free(ustr);
		}

		virtual const char *what() const throw()
		{
			return str.c_str();
		}

		/// Returns the name of the missing addon
		const char *addon() const
		{
			return this->addon_;
		}

		virtual ~Init_Failed() throw() { }
};

/// @ingroup Common
/// Stores four coordinates representing a rectangle
class Rectangle
{
	public:
		/// X coordinate of top-left corner of rectangle
		unit x1;

		/// Y coordinate of top-left corner of rectangle
		unit y1;

		/// X coordinate of bottom-right corner of rectangle
		unit x2;

		/// Y coordinate of bottom-right corner of rectangle
		unit y2;

		/// Create a rectangle using the specified set of four coordinates
		Rectangle(unit x1_, unit y1_, unit x2_, unit y2_)
			: x1(x1_)
			, y1(y1_)
			, x2(x2_)
			, y2(y2_)
		{ }

		/// Returns the width of the rectangle
		unit Width() const
		{
			return this->x2 - this->x1;
		}

		/// Returns the height of the rectangle
		unit Height() const
		{
			return this->y2 - this->y1;
		}

		/// Returns the size of the rectangle (width * height)
		unit Size() const
		{
			return this->Width() * this->Height();
		}
};

/// @internal
/// @ingroup Internal Common
/// Initializes the Allegro library
inline void _init()
{
	if (!al_get_system_driver())
	{
		if (!(al_init()))
			throw Init_Failed("core");

#ifdef A5SES_IMAGE_ADDON
		if (!(al_init_image_addon()))
			throw Init_Failed("image");
#endif

#ifdef A5SES_AUDIO_ADDON
		if (!(al_install_audio(ALLEGRO_AUDIO_DRIVER_AUTODETECT)))
			throw Init_Failed("audio");
#endif

#ifdef A5SES_FONT_ADDON
		al_init_font_addon();

#ifdef A5SES_TTF_ADDON
		if (!(al_init_ttf_addon()))
			throw Init_Failed("ttf");
#endif

#endif // A5SES_FONT_ADDON
	}
}

}

#endif // A5SES_COMMON_HPP_INCLUDED
