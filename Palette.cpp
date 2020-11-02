
#include "Palette.hpp"

int shared_yoffs[7] = {};

const int num_tiles_spec = 41;

const char *tiles_spec[1][48] = {
	{ // -1 (Special)
		"/specs/0.bmp",                   // Wall,
		"/specs/1.bmp",                   // ChairDown,
		"/specs/2.bmp",                   // ChairLeft,
		"/specs/3.bmp",                   // ChairRight,
		"/specs/4.bmp",                   // ChairUp,
		"/specs/5.bmp",                   // ChairDownRight,
		"/specs/6.bmp",                   // ChairUpLeft,
		"/specs/7.bmp",                   // ChairAll,
		"/specs/8.bmp",                   // UnknownDoor,
		"/specs/9.bmp",                   // Chest,
		"/specs/10.bmp",                  // SpecUnknown1,
		"/specs/11.bmp",                  // SpecUnknown2,
		"/specs/12.bmp",                  // SpecUnknown3,
		"/specs/13.bmp",                  // SpecUnknown4,
		"/specs/14.bmp",                  // SpecUnknown5,
		"/specs/15.bmp",                  // SpecUnknown6,
		"/specs/16.bmp",                  // BankVault,
		"/specs/17.bmp",                  // NPCBoundary,
		"/specs/18.bmp",                  // MapEdge,
		"/specs/19.bmp",                  // FakeWall,
		"/specs/20.bmp",                  // Board1,
		"/specs/21.bmp",                  // Board2,
		"/specs/22.bmp",                  // Board3,
		"/specs/23.bmp",                  // Board4,
		"/specs/24.bmp",                  // Board5,
		"/specs/25.bmp",                  // Board6,
		"/specs/26.bmp",                  // Board7,
		"/specs/27.bmp",                  // Board8,
		"/specs/28.bmp",                  // Jukebox,
		"/specs/29.bmp",                  // Jump,
		"/specs/30.bmp",                  // Water,
		"/specs/31.bmp",                  // SpecUnknown7,
		"/specs/32.bmp",                  // Arena,
		"/specs/33.bmp",                  // AmbientSource,
		"/specs/34.bmp",                  // Spikes1,
		"/specs/35.bmp",                  // Spikes2,
		"/specs/36.bmp",                  // Spikes3,
		"/specs/warp.bmp",                // Warp,
		"/specs/item.bmp",                // Item spawn
		"/specs/npc.bmp",                 // NPC,
		"/specs/sign.bmp",                // Sign,
		"/specs/genericitems.bmp",        // Items bubble,
		"/specs/signcaptioned.bmp",       // Sign bubble,
		"/specs/chestcaptioned.bmp",      // Chest bubble,
		"/specs/warpcaptioned.bmp",       // Warp bubble,
		"/specs/doorcaptioned.bmp",       // Door bubble,
		"/specs/lockeddoorcaptioned.bmp", // Locked door bubble
		"/specs/npccaptioned.bmp"         // NPC bubble
	}
};

inline int divup(int n, int d)
{
	return (n + (d - 1)) / d;
}

const int res = 32;

void Palette::Populate(GFX_Loader &loader)
{
	if (!this->tiles.empty())
	{
		return;
	}

	if (this->file > 0)
		loader.Prepare(this->file);

	Tile tile;

	int local_num_tiles = (this->file >= 0) ? (loader.CountBitmaps(this->file) - 1) : num_tiles_spec;

	if (this->file == -1 || this->file == 3)
	{
		for (int i = 0; i < local_num_tiles; ++i)
		{
			tile.id = i;
			tile.x = (i % 10) << 6;
			tile.y = (i / 10) << 5;
			tile.w = 64;
			tile.h = 32;
			tiles.push_back(tile);

			this->height = std::max(this->height, tile.y + tile.h);
		}
	}
	else
	{
		bool *tilemap = static_cast<bool *>(calloc(divup(640, res) * divup(200000, res), sizeof(bool)));

		for (int i = 1; i < local_num_tiles; ++i)
		{
			auto bmp_info = loader.Info(this->file, i);

			if (this->file == 3 || this->file == 6)
			{
				if (bmp_info.width >= 128)
					bmp_info.width /= 4;
			}

			int draw_x = 0, draw_y = 0;

			for (int cy = 0; cy < divup(200000, res); ++cy)
			{
				for (int cx = 0; cx < divup(640, res); ++cx)
				{
					for (int y = 0; y < divup(bmp_info.height, res); ++y)
					{
						for (int x = 0; x < divup(bmp_info.width, res); ++x)
						{
							if (cy+y >= divup(200000, res) || cx+x >= divup(640, res) || tilemap[(cy+y) * divup(640, res) + (cx+x)])
							{
								goto next_check;
							}
						}
					}

					draw_x = cx;
					draw_y = cy;

					goto found_ya;

					next_check:
					;
				}
			}
			continue;

			found_ya:
			tile.id = i;
			tile.x = draw_x * res;
			tile.y = draw_y * res;
			tile.w = bmp_info.width;
			tile.h = bmp_info.height;
			tiles.push_back(tile);

			for (int y = 0; y < divup(tile.h, res); ++y)
				for (int x = 0; x < divup(tile.w, res); ++x)
					tilemap[(draw_y+y) * divup(640, res) + (draw_x+x)] = true;

			this->height = std::max(this->height, tile.y + tile.h);
		}

		//delete tilemap;
		free(tilemap);
	}

	this->height += 64;
}

void Palette::Click(int x, int y)
{
	y -= 32;
	selected_tile = 0;
	for (std::list<Palette::Tile>::iterator i = this->tiles.begin(); i != this->tiles.end(); ++i)
	{
		if (x > i->x && x <= i->x + i->w
		 && y > i->y && y <= i->y + i->h)
		{
			selected_tile = i->id;
			return;
		}
	}
}

int Palette::RightClick(int x, int y)
{
	y -= 32;

	for (std::list<Palette::Tile>::iterator i = this->tiles.begin(); i != this->tiles.end(); ++i)
	{
		if (x > i->x && x <= i->x + i->w
		 && y > i->y && y <= i->y + i->h)
		{
			return i->id;
		}
	}

	return 0;
}

void Pal_Renderer::Render()
{
	int target_h = this->target.Height();

	std::deque<std::function<void()>> drawcmd;

	for (std::list<Palette::Tile>::iterator i = this->pal->tiles.begin(); i != this->pal->tiles.end(); ++i)
	{
		int draw_x = i->x;
		int draw_y = i->y - yoff + 32;
		if (draw_y + i->h >= 0 && draw_y < target_h)
		{
			a5::Bitmap& bmp = [&]() -> a5::Bitmap&
			{
				if (this->pal->file >= 0)
					return this->gfxloader.Load(this->pal->file, i->id, this->animation_state);
				else
					return this->gfxloader.LoadRaw(tiles_spec[std::abs(this->pal->file)-1][i->id]);
			}();

			if (i->id == this->pal->selected_tile)
			{
				drawcmd.emplace_back([this, &bmp, draw_x, draw_y]()
				{
					if (this->pal->layer == 7)
					{
						this->target.BlitTinted(bmp, a5::RGBA(255, 255, 255, 160), draw_x, draw_y);
					}
					else
					{
						this->target.Blit(bmp, draw_x, draw_y);
						this->target.BlitTinted(bmp, a5::RGBA(160, 160, 160, 255), draw_x, draw_y);
					}
				});
			}
			else
			{
				drawcmd.emplace_back([this, &bmp, draw_x, draw_y]()
				{
					this->target.Blit(bmp, draw_x, draw_y);
				});
			}
		}
	}

	al_hold_bitmap_drawing(true);
	for (const auto& cmd : drawcmd)
		cmd();
	al_hold_bitmap_drawing(false);
}
