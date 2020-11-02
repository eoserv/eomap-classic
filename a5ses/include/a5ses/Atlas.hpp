
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

/// @file Atlas.hpp
/// @ingroup Atlas
/// Texure atlasing

#ifndef A5SES_ATLAS_HPP_INCLUDED
#define A5SES_ATLAS_HPP_INCLUDED

#include "common.hpp"
#include "Bitmap.hpp"

#include <vector>
#include <set>

namespace a5
{

class Atlas_Bitmap;

/// @ingroup Atlas
/// Texture atlas
class Atlas
{
	private:
		int width;
		int height;
		int x_res;
		int y_res;
		std::vector<Bitmap *> pages;
		std::vector<std::vector<char> > alloc_map; /* vector<bool> is slow in C++98 */
		bool destroying;

		void Free(Atlas_Bitmap *ref);

		// Disable copying
		Atlas(const Atlas &);
		Atlas &operator =(const Atlas &);

	public:
		/// @ingroup Atlas Event
		/// Thrown when trying to allocate a too-large section of the atlas
		class Too_Big : public Exception
		{
			private:
				int width_;
				int height_;
				int atlas_width_;
				int atlas_height_;
				std::string str;

			public:
				/// Creates a Too_Big exception
				Too_Big(int width__, int height__, int atlas_width__, int atlas_height__)
					: width_(width__)
					, height_(height__)
					, atlas_width_(atlas_width__)
					, atlas_height_(atlas_height__)
				{
					ALLEGRO_USTR *ustr = al_ustr_newf(
						"[a5::Atlas::Too_Big] Tried to allocate %ix%i piece of %ix%i texture atlas.",
						width(), height(), atlas_width(), atlas_height());
					str = al_cstr(ustr);
					al_ustr_free(ustr);
				}

				virtual const char *what() const throw()
				{
					return str.c_str();
				}

				/// Returns the width of the area the user attempted to allocate
				int width() const
				{
					return this->width_;
				}

				/// Returns the height of the area the user attempted to allocate
				int height() const
				{
					return this->height_;
				}

				/// Returns the width of the atlas used
				int atlas_width() const
				{
					return this->atlas_width_;
				}

				/// Returns the height of the atlas used
				int atlas_height() const
				{
					return this->atlas_height_;
				}

				virtual ~Too_Big() throw() { }
		};

		/// @ingroup Atlas Event
		/// Thrown when trying to free a bitmap that isn't a child of the atlas
		class Not_Child : public Exception
		{
			public:
				virtual const char *what() const throw()
				{
					return "[a5::Atlas::Not_Child] Tried to free a bitmap belonging to the wrong atlas.";
				}

				virtual ~Not_Child() throw() { }
		};

		/// Creates a texture atlas with the specified dimensions and resolution
		Atlas(int width_ = 2048, int height_ = 2048, int x_res_ = 32, int y_res_ = 32)
			: width(width_)
			, height(height_)
			, x_res(x_res_)
			, y_res(y_res_)
			, destroying(false)
		{
			if (width < 0 || height < 0)
				throw std::invalid_argument("Tried to create atlas with negative width/height");

			if (x_res <= 0 || y_res <= 0)
				throw std::invalid_argument("Tried to create atlas with negative or 0 resolution");

			if (x_res > width || y_res > height)
				throw std::invalid_argument("Tried to create atlas with higher resolution than size");
		}

		/// Creates a new page on the atlas
		void NewPage();

		/// Allocates room for and copies an existing bitmap in to the atlas
		/// @throw Atlas::Too_Big
		std::unique_ptr<Bitmap> Add(Bitmap &bitmap, a5::Rectangle &rect);

		/// Allocates an image to the atlas with the specified width/height
		/// @throw Atlas::Not_Child
		std::unique_ptr<Bitmap> Alloc(int width, int height);

		/// Returns the number of pages in the atlas
		int Pages() const
		{
			return pages.size();
		}

		/// Returns the atlas bitmap for the given page
		/// @throw std::out_of_range
		a5::Bitmap &GetPage(int page) const
		{
			if (page < 0 || page >= Pages())
				throw std::out_of_range("Tried to access atlas page out of range");

			return *pages[page];
		}

		/// Frees all of the allocated bitmaps in the atlas
		void Clear(bool delete_pages = false)
		{
			for (size_t page = 0; page < alloc_map.size(); ++page)
			{
				for (std::vector<char>::iterator it = alloc_map[page].begin(); it != alloc_map[page].end(); ++it)
				{
					*it = 0;
				}

				GetPage(page).Clear();
			}

			alloc_map.resize(0);

			if (delete_pages)
			{
				for (size_t page = 0; page < pages.size(); ++page)
				{
					delete pages[page];
				}

				pages.resize(0);
			}
		}

		/// Destroys all of the atlas pages and references
		~Atlas();

	friend class a5::Atlas_Bitmap;
};

/// @ingroup Atlas
/// Bitmap containing reference to an allocated part of a texture atlas
class Atlas_Bitmap : public a5::Bitmap
{
	private:
		a5::Atlas *atlas;
		int page;
		int block_x;
		int block_y;

		Atlas_Bitmap(a5::Atlas *atlas_, ALLEGRO_BITMAP *bitmap, int page_, int block_x_, int block_y_)
		: a5::Bitmap(bitmap, false)
		, atlas(atlas_)
		, page(page_)
		, block_x(block_x_)
		, block_y(block_y_)
		{ }

	public:
		/// Unregisters the bitmap from the atlas
		~Atlas_Bitmap()
		{
			//atlas->Free(this);
		}

	friend class a5::Atlas;
};

}

#endif // A5SES_ATLAS_HPP_INCLUDED
