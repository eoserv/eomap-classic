#ifndef PALETTE_INCLUDED
#define PALETTE_INCLUDED

#include "common.hpp"

#include "GFX_Loader.hpp"

extern int shared_yoffs[7];
const bool floor_tiles[25] = { 0, 0, 0,   1,   0,  1,    0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
extern const int num_tiles_spec;
extern const char *tiles_spec[1][48];

class Palette
{
	public:
		int height = 0;

		int file;
		int layer = 0;

		struct Tile
		{
			short id;
			unsigned short x, y;
			unsigned short w, h;
		};

		std::list<Tile> tiles;
		short selected_tile = 0;

		// saved offset
		int* yoff;

		Palette(int file_) : file(file_)
		{
			switch (file_)
			{
				case 3: yoff = &shared_yoffs[0]; break;
				case 4: yoff = &shared_yoffs[1]; break;
				case 5: yoff = &shared_yoffs[2]; break;
				case 6: yoff = &shared_yoffs[3]; break;
				case 7: yoff = &shared_yoffs[4]; break;
				case 22: yoff = &shared_yoffs[5]; break;
				case -1: yoff = &shared_yoffs[6]; break;
				default: std::abort();
			}
		}

		void Populate(GFX_Loader &loader);

		void Click(int x, int y);
		int RightClick(int x, int y);
};

class Pal_Renderer
{
	public:
		a5::Display &target;
		GFX_Loader gfxloader;
		Palette *pal = nullptr;
		int yoff = 0;
		int scrollyoff = 0;
		int animation_state = 0;
		a5::Bitmap *cursor;

		Pal_Renderer(a5::Display &target_) :
			target(target_)
		{ }

		void SetPal(int layer, Palette &pal)
		{
			if (this->pal)
				*this->pal->yoff = yoff;

			pal.Populate(this->gfxloader);
			this->pal = &pal;
			this->pal->layer = layer;

			this->yoff = *pal.yoff;

			gfxloader.Reset();
		}

		void Move(int y)
		{
			this->yoff = y;
		}

		void ResetView()
		{
			this->Move(0);
		}

		void Render();
};

#endif // PALETTE_INCLUDED
