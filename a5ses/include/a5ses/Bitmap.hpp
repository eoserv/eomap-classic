
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Bitmap.hpp
/// @ingroup Bitmap
/// Contains pixel format and bitmap related classes

#ifndef A5SES_BITMAP_HPP_INCLUDED
#define A5SES_BITMAP_HPP_INCLUDED

#include "common.hpp"
#include "Color.hpp"

#include <cstdio>

namespace a5
{

extern bool disable_auto_target;

/// @ingroup Bitmap
/// Bitmap pixel format
class Pixel_Format
{
	public:
		/// List of available pixel formats
		enum Format
		{
			Any = ALLEGRO_PIXEL_FORMAT_ANY,
			AnyNoAlpha = ALLEGRO_PIXEL_FORMAT_ANY_NO_ALPHA,
			AnyWithAlpha = ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA,
			Any15NoAlpha = ALLEGRO_PIXEL_FORMAT_ANY_15_NO_ALPHA,
			Any16NoAlpha = ALLEGRO_PIXEL_FORMAT_ANY_16_NO_ALPHA,
			Any16WithAlpha = ALLEGRO_PIXEL_FORMAT_ANY_16_WITH_ALPHA,
			Any24NoAlpha = ALLEGRO_PIXEL_FORMAT_ANY_24_NO_ALPHA,
			Any32NoAlpha = ALLEGRO_PIXEL_FORMAT_ANY_32_NO_ALPHA,
			Any32WithAlpha = ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA,
			ARGB_8888 = ALLEGRO_PIXEL_FORMAT_ARGB_8888,
			RGBA_8888 = ALLEGRO_PIXEL_FORMAT_RGBA_8888,
			ARGB_4444 = ALLEGRO_PIXEL_FORMAT_ARGB_4444,
			RGB_888 = ALLEGRO_PIXEL_FORMAT_RGB_888,
			RGB_565 = ALLEGRO_PIXEL_FORMAT_RGB_565,
			RGB_555 = ALLEGRO_PIXEL_FORMAT_RGB_555,
			RGBA_5551 = ALLEGRO_PIXEL_FORMAT_RGBA_5551,
			ARGB_1555 = ALLEGRO_PIXEL_FORMAT_ARGB_1555,
			ABGR_8888 = ALLEGRO_PIXEL_FORMAT_ABGR_8888,
			XBGR_8888 = ALLEGRO_PIXEL_FORMAT_XBGR_8888,
			BGR_888 = ALLEGRO_PIXEL_FORMAT_BGR_888,
			BGR_565 = ALLEGRO_PIXEL_FORMAT_BGR_565,
			BGR_555 = ALLEGRO_PIXEL_FORMAT_BGR_555,
			RGBX_8888 = ALLEGRO_PIXEL_FORMAT_RGBX_8888,
			XRGB_8888 = ALLEGRO_PIXEL_FORMAT_XRGB_8888,
			ABGR_F32 = ALLEGRO_PIXEL_FORMAT_ABGR_F32,
			ABGR_8888_LE = ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE
		};

	private:
		/// Stores the raw format ID
		Format format;

	public:
		/// Creates an uninitialized Pixel_Format instance
		explicit Pixel_Format() { }

		/// Creates a Pixel_Format instance
		Pixel_Format(Format format_)
			: format(format_)
		{ }

		/// Returns the size (in bytes) of one pixel using this pixel format, or 0 if the format is unknown.
		int ByteSize() const
		{
			return al_get_pixel_size(format);
		}

		/// Returns the size (in bits) of one pixel using this pixel format, or 0 if the format is unknown.
		int BitSize() const
		{
			return al_get_pixel_size(format) * 8;
		}

		/// Returns the raw Format number
		operator Format() const
		{
			return this->format;
		}
};

/// @ingroup Bitmap
/// Locked Bitmap region
class Bitmap_Lock
{
	private:
		/// Pointer to the Allegro region the lock is using
		ALLEGRO_LOCKED_REGION *region;

		/// Pointer to the Allegro bitmap the lock is using
		ALLEGRO_BITMAP *bitmap;

	public:
		/// Creates a Bitmap_Lock from the specified Allegro region and bitmap
		explicit Bitmap_Lock(ALLEGRO_LOCKED_REGION *region_, ALLEGRO_BITMAP *bitmap_)
			: region(region_)
			, bitmap(bitmap_)
		{ }

		Bitmap_Lock(const Bitmap_Lock&) = delete;
		Bitmap_Lock& operator =(const Bitmap_Lock&) = delete;

		/// Returns the pixel format for the locked region
		Pixel_Format Format() const
		{
			return static_cast<Pixel_Format::Format>(this->region->format);
		}

		/// Returns the pitch of the locked bitmap
		int Pitch() const
		{
			return this->region->pitch;
		}

		/// Returns the raw image data
		void *Data() const
		{
			return this->region->data;
		}

		/// Returns the raw image data
		operator void *() const
		{
			return this->Data();
		}

		/// Unlocks the bitmap and updates the video memory
		~Bitmap_Lock()
		{
			al_unlock_bitmap(this->bitmap);
		}
};

class Display;

/// @ingroup Bitmap
/// Stores an image
class Bitmap
{
	public:
		/// Describes the type of Lock
		enum LockType
		{
			ReadWrite = 0,
			ReadOnly = ALLEGRO_LOCK_READONLY,
			WriteOnly = ALLEGRO_LOCK_WRITEONLY
		};

	private:
		/// Pointer to the Allegro bitmap data
		ALLEGRO_BITMAP *bitmap;

		/// Whether or not to destroy the bitmap when the object is destroyed
		bool created;

	public:
		/// Flip the image on the x axis
		static const int FlipHorizontal = ALLEGRO_FLIP_HORIZONTAL;
		
		/// Flip the image on the y axis
		static const int FlipVertical = ALLEGRO_FLIP_VERTICAL;

		/// @ingroup Exception Bitmap
		/// Thrown when a bitmap fails to load
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
						"[a5::Bitmap::Load_Failed] Failed to load image file: %s",
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

		/// @ingroup Exception Bitmap
		/// Thrown when an attempt to lock a bitmap fails
		class Lock_Failed : public Exception
		{
			public:
				/// Creates a Lock_Failed exception
				Lock_Failed() { }

				virtual const char *what() const throw()
				{
					return "[a5::Bitmap::Lock_Failed] Failed to lock bitmap";
				}

				virtual ~Lock_Failed() throw() { }
		};

		/// Creates an uninitialized bitmap
		Bitmap()
			: created(false)
		{ }

		/// Creates a bitmap with the specified width and height
		Bitmap(unit width, unit height)
			: bitmap(al_create_bitmap(width, height))
			, created(true)
		{
			if (width < 0 || height < 0)
				throw std::invalid_argument("Tried to create bitmap with negative width/height");
		}

		/// Loads an image from a file in to a Bitmap object
		/// Requires A5SES_IMAGE_ADDON
		/// @throw Bitmap::Load_Failed, Need_Addon
		explicit Bitmap(const char *filename)
		{
#ifdef A5SES_IMAGE_ADDON
			if (!(this->bitmap = al_load_bitmap(filename)))
				throw Load_Failed(filename);

			this->created = true;
#else
			throw Need_Addon("image");
#endif
		}

		Bitmap(Bitmap &&other)
			: bitmap(other.bitmap)
			, created(other.created)
		{
			other.bitmap = nullptr;
			other.created = false;
		}

		/// Wraps an existing ALLEGRO_BITMAP in a Bitmap object
		Bitmap(ALLEGRO_BITMAP *bitmap_, bool autofree = false)
			: bitmap(bitmap_)
			, created(autofree)
		{ }

		/// Locks an entire bitmap
		Bitmap_Lock Lock(Pixel_Format format = Pixel_Format::Any, LockType type = ReadWrite)
		{
			ALLEGRO_LOCKED_REGION *region = al_lock_bitmap_region(*this, 0, 0, this->Width(), this->Height(), format, type);
			return Bitmap_Lock(region, *this);
		}

		/// Locks a region of a bitmap
		/// @throw Bitmap::Lock_Failed
		Bitmap_Lock Lock(const Rectangle rect, Pixel_Format format = Pixel_Format::Any, LockType type = ReadWrite)
		{
			if (rect.x1 < 0 || rect.y1 < 0 || rect.Width() > this->Width() || rect.Height() > this->Height())
				throw Bitmap::Lock_Failed();

			ALLEGRO_LOCKED_REGION *region = al_lock_bitmap_region(*this, rect.x1, rect.y1, rect.Width(), rect.Height(), format, type);
			return Bitmap_Lock(region, *this);
		}

		/// Returns the width of the bitmap
		unit Width() const
		{
			return al_get_bitmap_width(*this);
		}

		/// Returns the height of the bitmap
		unit Height() const
		{
			return al_get_bitmap_height(*this);
		}

		/// Clears the bitmap to the specified color
		void Clear(Color c = RGB(0, 0, 0))
		{
			if (!disable_auto_target)
				this->Target();
			al_clear_to_color(c);
		}

		/// Sets the global Allegro target bitmap to this
		/// Shouldn't need to be called if using pure A5SES
		void Target()
		{
			al_set_target_bitmap(*this);
		}

		/// Creates a sub-bitmap object from the specified rectangle
		std::unique_ptr<Bitmap> Sub(Rectangle r)
		{
			ALLEGRO_BITMAP *sub = al_create_sub_bitmap(*this, r.x1, r.y1, r.Width(), r.Height());
			return std::make_unique<Bitmap>(sub, true);
		}

		/// Sets a pixel to a color
		void PutPixel(unit x, unit y, Color c)
		{
			if (!disable_auto_target)
				this->Target();
			al_put_pixel(x, y, c);
			unsigned char r, g, b;
			al_unmap_rgb(c, &r, &g, &b);
		}

		/// Returns the color of a pixel
		Color GetPixel(unit x, unit y) const
		{
			return al_get_pixel(*this, x, y);
		}

		/// Draws an entire Bitmap to this one
		void Blit(Bitmap &src, unit x, unit y, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_bitmap(src, x, y, flags);
		}

		/// Draws a partial Bitmap to this one
		void Blit(Bitmap &src, unit x, unit y, Rectangle clip, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_bitmap_region(src, clip.x1, clip.y1, clip.Width(), clip.Height(), x, y, flags);
		}

		/// Draws a bitmap scaled to @p r to this one
		void BlitScaled(Bitmap &src, Rectangle r, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_scaled_bitmap(src, 0, 0, src.Width(), src.Height(), r.x1, r.y1, r.Width(), r.Height(), flags);
		}

		/// Draws an entire Bitmap to this one (tinted)
		void BlitTinted(Bitmap &src, Color tint, unit x, unit y, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_tinted_bitmap(src, tint, x, y, flags);
		}

		/// Draws a partial Bitmap to this one (tinted)
		void BlitTinted(Bitmap &src, Color tint, unit x, unit y, Rectangle clip, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_tinted_bitmap_region(src, tint, clip.x1, clip.y1, clip.Width(), clip.Height(), x, y, flags);
		}

		/// Draws a bitmap scaled to @p r to this one (tinted)
		void BlitTintedScaled(Bitmap &src, Color tint, unit x, unit y, Rectangle r, int flags = 0)
		{
			if (!disable_auto_target)
				this->Target();

			al_draw_tinted_scaled_bitmap(src, tint, 0, 0, src.Width(), src.Height(), r.x1, r.y1, r.Width(), r.Height(), flags);
		}

		Bitmap &operator =(Bitmap &&other)
		{
			if (bitmap && created)
				al_destroy_bitmap(bitmap);

			bitmap = other.bitmap;
			created = other.created;
			other.bitmap = nullptr;
			other.created = false;
			return *this;
		}

		/// Releases the held C structure so it is no longer automatically freed
		ALLEGRO_BITMAP *Release()
		{
			this->created = false;
			return *this;
		}

		/// Destroys the bitmap if @a created is true
		virtual ~Bitmap()
		{
			if (this->created)
				al_destroy_bitmap(*this);
		}

		/// Returns a pointer to the Allegro bitmap structure
		operator ALLEGRO_BITMAP *() const
		{
			return this->bitmap;
		}

	friend class Display;
};

}

#endif // A5SES_BITMAP_HPP_INCLUDED
