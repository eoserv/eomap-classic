
/* A5SES is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "Atlas.hpp"

#include "a5ses.hpp"

namespace a5
{

bool disable_auto_target = false;

void Atlas::NewPage()
{
	alloc_map.push_back(std::vector<char>((width / x_res) * (height / y_res)));
	pages.push_back(new Bitmap(width, height));
}

std::unique_ptr<Bitmap> Atlas::Add(Bitmap &bitmap, Rectangle& rect)
{
	std::unique_ptr<Bitmap> atlas_bitmap = Alloc(rect.Width(), rect.Height());
	if (!atlas_bitmap)
		return nullptr;
	auto tmp = al_get_target_bitmap();
	atlas_bitmap->Target();
	atlas_bitmap->Blit(bitmap, 0, 0, rect);
	al_set_target_bitmap(tmp);
	return atlas_bitmap;
}

std::unique_ptr<Bitmap> Atlas::Alloc(int width, int height)
{
	if (width < 0 || height < 0)
		return nullptr;

	if (width > this->width || height > this->height)
		throw Too_Big(width, height, this->width, this->height);

	if (Pages() == 0)
		NewPage();

	const int page_width = this->width / x_res - 1;
	const int page_height = this->height / y_res - 1;

	const int block_width = (width + (x_res - 1)) / x_res;
	const int block_height = (height + (x_res - 1)) / x_res;

	for (int page = 0; ; ++page)
	{
		for (int y = 0; y < page_height; ++y)
		{
			for (int x = 0; x < page_width; ++x)
			{
				for (int check_y = y; check_y < y + block_height; ++check_y)
				{
					int yoff = check_y * page_width;

					for (int check_x = x; check_x < x + block_width; ++check_x)
					{
						if (check_x > page_width)
							goto x_not_found;

						if (alloc_map[page][yoff + check_x])
							goto not_found;
					}

					if (check_y > page_height)
						goto y_not_found;
				}

				for (int set_y = y; set_y < y + block_height; ++set_y)
				{
					int yoff = set_y * page_width;

					for (int set_x = x; set_x < x + block_width; ++set_x)
					{
						alloc_map[page][yoff + set_x] = 1;
					}
				}

				{
					std::unique_ptr<Bitmap> atlas_bitmap(new Atlas_Bitmap(this, pages[page]->Sub(Rectangle(x * x_res, y * y_res, x * x_res + width, y * y_res + height))->Release(), page, x, y));
					return atlas_bitmap;
				}

				not_found: ;
			}

			x_not_found: ;
		}

		y_not_found:

		if (page == Pages() - 1)
			NewPage();
	}
}

void Atlas::Free(Atlas_Bitmap *ref)
{
	if (destroying)
		return;

	const int page_width = ref->atlas->width / x_res;
	const int block_width = (ref->Width() + (x_res - 1)) / x_res;
	const int block_height = (ref->Height() + (x_res - 1)) / x_res;

	for (int set_y = ref->block_y; set_y < ref->block_y + block_width; ++set_y)
	{
		int yoff = set_y * page_width;

		for (int set_x = ref->block_x; set_x < ref->block_x + block_height; ++set_x)
		{
			alloc_map[ref->page][yoff + set_x] = 0;
		}
	}
}

Atlas::~Atlas()
{
	destroying = true;
}

}
