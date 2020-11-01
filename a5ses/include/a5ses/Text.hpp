
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Text.hpp
/// @ingroup Audio
/// Contains pixel format and bitmap related classes

#ifndef A5SES_TEXT_HPP_INCLUDED
#define A5SES_TEXT_HPP_INCLUDED

#include "common.hpp"
#include "Bitmap.hpp"

#ifndef A5SES_FONT_ADDON
#error Please define A5SES_FONT_ADDON
#endif

namespace a5
{

/// @ingroup Text
/// Stores a font
class Font
{
	private:
		/// Pointer to the Allegro font
		ALLEGRO_FONT *font;

		/// Whether or not to destroy the bitmap when the object is destroyed
		bool created;

		// Disable copying
		Font(const Font &);
		Font &operator =(const Font &);

	public:
		/// @ingroup Exception Text
		/// Thrown when a font fails to load
		class Load_Failed : public Exception
		{
			private:
				const char *filename_;
				std::string str;

			public:
				/// Creates a Load_Failed exception
				Load_Failed(const char *filename__)
					: filename_(filename__)
				{
					ALLEGRO_USTR *ustr = al_ustr_newf(
						"[a5::Font::Load_Failed] Failed to load font file: %s",
						filename());
					str = al_cstr(ustr);
					al_ustr_free(ustr);
				}

				virtual const char *what() const throw()
				{
					return str.c_str();
				}

				/// Returns the name of the file that couldn't be loaded
				const char *filename() const
				{
					return this->filename_;
				}

				virtual ~Load_Failed() throw() { }
		};

		/// Font loading flags
		enum Flags
		{
#ifdef A5SES_TTF_ADDON
			NoKerning = ALLEGRO_TTF_NO_KERNING
#endif
		};

		/// Text alignment
		enum Align
		{
			Left = ALLEGRO_ALIGN_LEFT,
			Center = ALLEGRO_ALIGN_CENTRE,
			Right = ALLEGRO_ALIGN_RIGHT
		};

		/// Loads a font from a file in to a Font object
		/// @throw Font::Load_Failed
		Font(const char *filename, int size = 0, int flags = 0)
			: created(true)
		{
			if (!(this->font = al_load_font(filename, size, flags)))
				throw Load_Failed(filename);
		}

		/// Loads a bitmap font
		Font(Bitmap &bitmap, int ranges_n, int ranges[])
			: created(true)
		{
			this->font = al_grab_font_from_bitmap(bitmap, ranges_n, ranges);
		}

		/// Wraps an existing ALLEGRO_FONT in a Font object
		Font(ALLEGRO_FONT *font_, bool autofree = false)
			: font(font_)
			, created(autofree)
		{ }

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_FONT *Release()
		{
			this->created = false;
			return *this;
		}

		/// Destroys the font
		~Font()
		{
			if (this->created)
				al_destroy_font(*this);
		}

		/// Returns a pointer to the Allegro font structure
		operator ALLEGRO_FONT *() const
		{
			return this->font;
		}
};

/// @ingroup Text
/// Set and fire class for rendering text
class Text_Renderer
{
	public:
		const Font &font;
		unit x, y;
		Font::Align align;
		Color color;
		std::string text;

		Text_Renderer(const Font &font_)
			: font(font_)
			, x(0)
			, y(0)
			, align(Font::Left)
		{ }

		void Render(Bitmap &bmp) const
		{
			bmp.Target();
			al_draw_text(this->font, this->color, this->x, this->y, (int)align, this->text.c_str());
		}
};

}

#endif // A5SES_TEXT_HPP_INCLUDED
