
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Color.hpp
/// @ingroup Color
/// Contains color related classes

#ifndef A5SES_COLOR_HPP_INCLUDED
#define A5SES_COLOR_HPP_INCLUDED

#include "common.hpp"

namespace a5
{

/// @ingroup Color
/// Stores and converts colors
class Color
{
	private:
		/// Allegro color
		ALLEGRO_COLOR color;

	public:
		/// Creates a color, initialized to transparent black
		Color()
			: color(al_map_rgba(255, 255, 255, 0))
		{ }

		/// Maps an integer to a color
		Color(unsigned int c)
			: color(al_map_rgb((c << 16) & 0xFF, (c << 8) & 0xFF, c & 0xFF))
		{ }

		/// Wraps an Allegro color structure
		Color(ALLEGRO_COLOR c)
			: color(c)
		{ }

		/// Returns the Allegro color structure
		operator ALLEGRO_COLOR() const
		{
			return this->color;
		}

		/// Compares Color objects
		bool operator ==(const Color &b) const
		{
			unsigned char ar, ag, ab, aa;
			al_unmap_rgba(this->color, &ar, &ag, &ab, &aa);

			unsigned char br, bg, bb, ba;
			al_unmap_rgba(b.color, &br, &bg, &bb, &ba);

			return (ar == br && ag == bg && ab == bb && aa == ba);
		}
};

/// @ingroup Color
/// Stores Colors in the RGB format
struct RGB
{
	/// Red color component
	int r;

	/// Green color component
	int g;

	/// Blue color component
	int b;

	/// Creates an RGB object with the specified values
	RGB(int r_, int g_, int b_)
		: r(r_)
		, g(g_)
		, b(b_)
	{ }

	/// Creates an RGB object from a Color object
	RGB(const Color &c)
	{
		unsigned char r_, g_, b_;

		al_unmap_rgb(c, &r_, &g_, &b_);

		r = r_;
		g = g_;
		b = b_;
	}

	/// Converts to a Color object
	operator Color() const
	{
		return Color(al_map_rgb(r, g, b));
	}
};

/// @ingroup Color
/// Stores Colors in the RGBA format
struct RGBA
{
	/// Red color component
	int r;

	/// Green color component
	int g;

	/// Blue color component
	int b;

	/// Alpha (transparency) component
	int a;

	/// Creates an RGBA object with the specified values
	RGBA(int r_, int g_, int b_, int a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{ }

	/// Creates an RGBA object from a Color object
	RGBA(const Color &c)
	{
		unsigned char r_, g_, b_, a_;

		al_unmap_rgba(c, &r_, &g_, &b_, &a_);

		r = r_;
		g = g_;
		b = b_;
		a = a_;
	}

	/// Converts to a Color object
	operator Color() const
	{
		return Color(al_map_rgba(r, g, b, a));
	}
};

/// @ingroup Color
/// Stores Colors as floating point in the RGB format
struct RGB_Float
{
	/// Red color component
	float r;

	/// Green color component
	float g;

	/// Blue color component
	float b;

	/// Creates an RGB object with the specified values
	RGB_Float(float r_, float g_, float b_)
		: r(r_)
		, g(g_)
		, b(b_)
	{ }

	/// Creates an RGB object from a Color object
	RGB_Float(const Color &c)
	{
		float r_, g_, b_;

		al_unmap_rgb_f(c, &r_, &g_, &b_);

		r = r_;
		g = g_;
		b = b_;
	}

	/// Converts to a Color object
	operator Color() const
	{
		return Color(al_map_rgb_f(r, g, b));
	}
};

/// @ingroup Color
/// Stores Colors as floating point in the RGBA format
struct RGBA_Float
{
	/// Red color component
	float r;

	/// Green color component
	float g;

	/// Blue color component
	float b;

	/// Alpha (transparency) component
	float a;

	/// Creates an RGBA object with the specified values
	RGBA_Float(float r_, float g_, float b_, float a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{ }

	/// Creates an RGBA object from a Color object
	RGBA_Float(const Color &c)
	{
		float r_, g_, b_, a_;

		al_unmap_rgba_f(c, &r_, &g_, &b_, &a_);

		r = r_;
		g = g_;
		b = b_;
		a = a_;
	}

	/// Converts to a Color object
	operator Color() const
	{
		return Color(al_map_rgba_f(r, g, b, a));
	}
};

}

#endif // A5SES_COLOR_HPP_INCLUDED
