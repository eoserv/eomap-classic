#include "Map_Renderer.hpp"

#include "Palette.hpp"

void Map_Renderer::Render()
{
    if (map->width <= 0 || map->height <= 0) return;

	int target_w = this->target.Width();
	int target_h = this->target.Height();

	std::vector<short> map_flat(9 * (map->width+1) * (map->height+1), -1);

	for (int i = 0; i < 9; ++i)
	{
		if (!this->show_layers[i]) continue;
		for (std::vector<EO_Map::GFX_Row>::iterator row = map->gfxrows[i].begin(); row != map->gfxrows[i].end(); ++row)
		{
			if (row->y > map->height)
			{
				continue;
			}

			for (std::vector<EO_Map::GFX>::iterator tile = row->tiles.begin(); tile != row->tiles.end(); ++tile)
			{
				if (tile->x > map->width)
				{
					continue;
				}

				map_flat[row->y * ((map->width + 1) * 9) + (tile->x * 9) + i] = tile->tile;
			}
		}
	}

	int file_map[9] = {  3,  4,  5,  6,  6,  7,  3, 22, 5};
	int xoff_map[9] = {  0,  0,  0,  0, 32, 32, 32,-54, 0};
	int yoff_map[9] = {  0,  0,  0,  0,  0,-64,-32,-42, 0};

	int starty = ((((this->yoff + 16) << 1) - (this->xoff)) >> 1) & INT_MAX;
	int startx = (this->xoff + starty) & INT_MAX;
	starty = (starty >> 5);
	startx = (startx >> 5) - 1;

	a5::Bitmap& base_gfx = this->gfxloader.Load(3, map->fill_tile, this->animation_state);
	int base_gfx_w = base_gfx.Width();
	int base_gfx_h = base_gfx.Height();

	for (int ii = 0; ii <= map->width + map->height; ++ii)
	{
		int x, y;
		if (ii < map->height)
		{
			x = 0;
			y = ii;
		}
		else
		{
			x = ii - map->height;
			y = map->height;
		}

		while (y >= 0 && x <= map->width)
		{
			int xoff = xoff_map[0] - this->xoff;
			int yoff = yoff_map[0] - this->yoff;
			short tile = map_flat[y * ((map->width + 1) * 9) + (x * 9)];
			if (tile >= 0)
			{
				a5::Bitmap& gfx = this->gfxloader.Load(file_map[0], tile, animation_state);
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();
				int draw_x = xoff + (x << 5) - (y << 5);
				int draw_y = yoff + (x << 4) + (y << 4);

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.Blit(gfx, draw_x, draw_y);

					if (this->gfxloader.IsError(gfx))
					{
						al_draw_textf(
							font, al_map_rgb(255, 255, 255),
							draw_x + 32, draw_y + 12, ALLEGRO_ALIGN_CENTER,
							"%d/%d", 0, tile
						);
					}
				}
			}
			else
			{
				int draw_x = xoff + (x << 5) - (y << 5);
				int draw_y = yoff + (x << 4) + (y << 4);

				if ((draw_x + base_gfx_w) >= 0 && (draw_y + base_gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.Blit(base_gfx, draw_x, draw_y);
				}
			}

			--y;
			++x;
		}
	}

	if (this->show_layers[10] || highlight_spec)
	{
		a5::Color tint = a5::RGBA(255, 255, 255, 64 * (highlight_spec * 2 + 1));

		for (std::vector<EO_Map::Tile_Row>::iterator i = map->tilerows.begin(); i != map->tilerows.end(); ++i)
		{
			for (std::vector<EO_Map::Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
			{
			    a5::Bitmap& gfx = [&]() -> a5::Bitmap&
				{
					try
					{
						return this->gfxloader.LoadRaw("/specs/" + util::to_string(int(ii->spec)) + ".bmp");
					}
					catch (a5::Bitmap::Load_Failed &e)
					{
						return this->gfxloader.LoadRaw("/specs/generic.bmp");
					}
				}();
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();

				int draw_x = (ii->x << 5) - (i->y << 5) - ((int(gfx_w) >> 1) - 32) - xoff;
				int draw_y = (ii->x << 4) + (i->y << 4) - yoff;

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.BlitTinted(gfx, tint, draw_x, draw_y);
				}
			}
		}
	}

	for (int ii = 0; ii <= map->width + map->height; ++ii)
	{
		int x, y;
		if (ii < map->height)
		{
			x = 0;
			y = ii;
		}
		else
		{
			x = ii - map->height;
			y = map->height;
		}
		while (y >= 0 && x <= map->width)
		{
			int xoff = xoff_map[7] - this->xoff;
			int yoff = yoff_map[7] - this->yoff;
			short tile = map_flat[y * ((map->width + 1) * 9) + (x * 9) + 7];
			//printf("\n%i\n", tile);
			if (tile >= 0)
			{
				a5::Bitmap&gfx = this->gfxloader.Load(file_map[7], tile, animation_state);
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();
				int draw_x = xoff + (x << 5) - (y << 5) + 32;
				int draw_y = yoff + (x << 4) + (y << 4) + 32;

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 50), draw_x, draw_y);

					if (this->gfxloader.IsError(gfx))
					{
						al_draw_textf(
							font, al_map_rgb(255, 255, 255),
							draw_x + 32, draw_y + 12, ALLEGRO_ALIGN_CENTER,
							"%d/%d", 7, tile
						);
					}
				}
			}

			--y;
			++x;
		}
	}

	if (this->show_layers[11])
	{
		a5::Color tint = a5::RGBA(255, 255, 255, 128);

		a5::Bitmap& gfx = this->gfxloader.LoadRaw("/outline.bmp");
		int gfx_w = gfx.Width();
		int gfx_h = gfx.Height();

		for (int ii = 0; ii <= map->width + map->height; ++ii)
		{
			int x, y;
			if (ii < map->height)
			{
				x = 0;
				y = ii;
			}
			else
			{
				x = ii - map->height;
				y = map->height;
			}

			while (y >= 0 && x <= map->width)
			{
				int draw_x = (x << 5) - (y << 5) - ((int(gfx_w) >> 1) - 32) - xoff;
				int draw_y = (x << 4) + (y << 4) - yoff;

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.BlitTinted(gfx, tint, draw_x, draw_y);
				}

				--y;
				++x;
			}
		}
	}

	for (int ii = 0; ii <= map->width + map->height; ++ii)
	{
		int x, y;
		if (ii < map->height)
		{
			x = 0;
			y = ii;
		}
		else
		{
			x = ii - map->height;
			y = map->height;
		}

		while (y >= 0 && x <= map->width)
		{
			for (int i = 1; i < 7; ++i)
			{
				if (i == 2) continue;
				int xoff = xoff_map[i] - this->xoff;
				int yoff = yoff_map[i] - this->yoff;
				short tile = map_flat[y * ((map->width+1) * 9) + (x * 9) + i];
				//printf("\n%i\n", tile);
				if (tile >= 0)
				{
					a5::Bitmap&gfx = this->gfxloader.Load(file_map[i], tile, animation_state);
					int gfx_w = gfx.Width();
					int gfx_h = gfx.Height();
					int draw_x, draw_y;
					draw_x = xoff + (x * 32) - (y * 32);
					draw_y = yoff + (x * 16) + (y * 16);
					int tile_w = int(gfx_w);
					int tile_h = int(gfx_h);
					draw_x -= tile_w / (1 + (i == 1)) - 32;
					draw_y -= tile_h - 32;

					if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
					{
						if (i == 7)
							this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 50), draw_x, draw_y);
						else
							this->target.Blit(gfx, draw_x, draw_y);

						if (this->gfxloader.IsError(gfx))
						{
							al_draw_textf(
								font, al_map_rgb(255, 255, 255),
								draw_x + 32, draw_y + 12, ALLEGRO_ALIGN_CENTER,
								"%d/%d", i, tile
							);
						}
					}
				}
			}

			--y;
			++x;
		}
	}

	for (int ii = 0; ii <= map->width + map->height; ++ii)
	{
		int x, y;
		if (ii < map->height)
		{
			x = 0;
			y = ii;
		}
		else
		{
			x = ii - map->height;
			y = map->height;
		}

		while (y >= 0 && x <= map->width)
		{
			int xoff = xoff_map[2] - this->xoff;
			int yoff = yoff_map[2] - this->yoff;
			short tile = map_flat[y * ((map->width + 1) * 9) + (x * 9) + 2];
			if (tile >= 0)
			{
				a5::Bitmap&gfx = this->gfxloader.Load(file_map[2], tile, animation_state);
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();
				int draw_x = xoff + (x << 5) - (y << 5) - ((int(gfx_w) >> 1) - 32);
				int draw_y = yoff + (x << 4) + (y << 4) - ((int(gfx_h)) - 32);

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.Blit(gfx, draw_x, draw_y);

					if (this->gfxloader.IsError(gfx))
					{
						al_draw_textf(
							font, al_map_rgb(255, 255, 255),
							draw_x + 32, draw_y + 12, ALLEGRO_ALIGN_CENTER,
							"%d/%d", 2, tile
						);
					}
				}
			}

			--y;
			++x;
		}
	}

	// Players, NPCs etc here

	for (int ii = 0; ii <= map->width + map->height; ++ii)
	{
		int x, y;
		if (ii < map->height)
		{
			x = 0;
			y = ii;
		}
		else
		{
			x = ii - map->height;
			y = map->height;
		}

		while (y >= 0 && x <= map->width)
		{
			int xoff = xoff_map[8] - this->xoff;
			int yoff = yoff_map[8] - this->yoff;
			short tile = map_flat[y * ((map->width + 1) * 9) + (x * 9) + 8];
			if (tile >= 0)
			{
				a5::Bitmap&gfx = this->gfxloader.Load(file_map[8], tile, animation_state);
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();				int draw_x = xoff + (x << 5) - (y << 5) - ((int(gfx_w) >> 1) - 32);
				int draw_y = yoff + (x << 4) + (y << 4) - ((int(gfx_h)) - 32);

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.Blit(gfx, draw_x, draw_y);
				}
			}

			--y;
			++x;
		}
	}

	if (this->show_layers[10] || highlight_spec)
	{
		a5::Color tint = a5::RGBA(255, 255, 255, 64);

		for (std::vector<EO_Map::Tile_Row>::iterator i = map->tilerows.begin(); i != map->tilerows.end(); ++i)
		{
			for (std::vector<EO_Map::Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
			{
			    a5::Bitmap& gfx = [&]() -> a5::Bitmap&
				{
					try
					{
						return this->gfxloader.LoadRaw("/specs/" + util::to_string(int(ii->spec)) + ".bmp");
					}
					catch (a5::Bitmap::Load_Failed &e)
					{
						return this->gfxloader.LoadRaw("/specs/generic.bmp");
					}
				}();
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();

				int draw_x = (ii->x << 5) - (i->y << 5) - ((int(gfx_w) >> 1) - 32) - xoff;
				int draw_y = (ii->x << 4) + (i->y << 4) - yoff;

				if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
				{
					this->target.BlitTinted(gfx, tint, draw_x, draw_y);
				}
			}
		}
	}

	if (this->show_layers[9] || highlight_spec)
	{
        for (std::vector<EO_Map::Warp_Row>::iterator i = map->warprows.begin(); i != map->warprows.end(); ++i)
		{
			for (std::vector<EO_Map::Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
			{
			    //gfxid = map->GetTileSpec(ii->x, i->y) == 9 ? 41 : 39;
                int object = map->GetObject(ii->x, i->y);

                int xoff1 = 0 - this->xoff;
                int yoff1 = 0 - this->yoff -12;

                if (object != -1)
                {
                    a5::Bitmap& obj = this->gfxloader.Load(4, object, 0);
                    yoff1 -= obj.Height();
                    yoff1 += 22;
                }

                int x, y;
                x = ii->x;
                y = i->y;
                a5::Bitmap& gfx = this->gfxloader.LoadRaw(tiles_spec[0][ii->door == EO_Map::HasDoor ? 45 : (ii->door == EO_Map::NoDoor ? 44 : 46)]);
				int gfx_w = gfx.Width();
				int gfx_h = gfx.Height();
                int draw_x = xoff1 + (x << 5) - (y << 5) - ((gfx_w >> 1) - 32);
                int draw_y = yoff1 + (x << 4) + (y << 4) - (gfx_h - 32);

                if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
                {
                    this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 128), draw_x, draw_y);
                }
			}
        }
	}

	if (this->show_layers[9] || highlight_spec)
	{
        for (std::vector<EO_Map::Chest>::iterator i = map->chests.begin(); i != map->chests.end(); ++i)
        {
            //gfxid = map->GetTileSpec(i->x, i->y) == 9 ? 41 : 39;
            int object = map->GetObject(i->x, i->y);

			int xoff1 = 0 - this->xoff;
            int yoff1 = 0 - this->yoff -12;

            if (object != -1)
            {
                a5::Bitmap& obj = this->gfxloader.Load(4, object, 0);
                yoff1 -= obj.Height();
                yoff1 += 22;
            }

            if (map->GetWarpTile(i->x, i->y)) { yoff1 -= 25; }

			int x, y;
			x = i->x;
			y = i->y;
			a5::Bitmap&gfx = this->gfxloader.LoadRaw(tiles_spec[0][map->GetTileSpec(i->x, i->y) == 9 ? 43 : 41]);
			int gfx_w = gfx.Width();
			int gfx_h = gfx.Height();
			int draw_x = xoff1 + (x << 5) - (y << 5) - ((int(gfx_w) >> 1) - 32);
			int draw_y = yoff1 + (x << 4) + (y << 4) - (int(gfx_h) - 32);

            if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
            {
                this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 128), draw_x, draw_y);
            }
        }
	}

	if (this->show_layers[9] || highlight_spec)
	{
        for (std::vector<EO_Map::Sign>::iterator i = map->signs.begin(); i != map->signs.end(); ++i)
        {
            int object = map->GetObject(i->x, i->y);

			int xoff1 = 0 - this->xoff;
            int yoff1 = 0 - this->yoff -12;

            if (object != -1)
            {
                a5::Bitmap& obj = this->gfxloader.Load(4, object, 0);
                yoff1 -= obj.Height();
                yoff1 += 22;
            }

            if (map->GetWarpTile(i->x, i->y))           { yoff1 -= 25; }
            if (map->GetChestSpawns(i->x, i->y).size()) { yoff1 -= 25; }

			int x, y;
			x = i->x;
			y = i->y;
			a5::Bitmap&gfx = this->gfxloader.LoadRaw(tiles_spec[0][42]);
			int gfx_w = gfx.Width();
			int gfx_h = gfx.Height();
			int draw_x = xoff1 + (x << 5) - (y << 5) - ((gfx_w >> 1) - 32);
			int draw_y = yoff1 + (x << 4) + (y << 4) - (gfx_h - 32);

            if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
            {
                this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 128), draw_x, draw_y);
            }
        }
	}
    if (this->show_layers[9] || highlight_spec)
	{
        for (std::vector<EO_Map::NPC>::iterator i = map->npcs.begin(); i != map->npcs.end(); ++i)
        {
            int object = map->GetObject(i->x, i->y);

			int xoff1 = 0 - this->xoff;
            int yoff1 = 0 - this->yoff -12;

            if (object != -1)
            {
                a5::Bitmap& obj = this->gfxloader.Load(4, object, 0);
                yoff1 -= obj.Height();
                yoff1 += 22;
            }

            if (map->GetWarpTile(i->x, i->y))           { yoff1 -= 25; }
            if (map->GetChestSpawns(i->x, i->y).size()) { yoff1 -= 25; }
            if (map->GetSign(i->x, i->y))               { yoff1 -= 25; }

			int x, y;
			x = i->x;
			y = i->y;
			a5::Bitmap&gfx = this->gfxloader.LoadRaw(tiles_spec[0][47]);
			int gfx_w = gfx.Width();
			int gfx_h = gfx.Height();
			int draw_x = xoff1 + (x << 5) - (y << 5) - ((gfx_w >> 1) - 32);
			int draw_y = yoff1 + (x << 4) + (y << 4) - (gfx_h - 32);

            if ((draw_x + gfx_w) >= 0 && (draw_y + gfx_h) >= 0 && draw_x < target_w && draw_y < target_h)
            {
                this->target.BlitTinted(gfx, a5::RGBA(255, 255, 255, 128), draw_x, draw_y);
            }
        }
	}
}

void Map_Renderer::RebuildTarget(int w, int h)
{
	auto tmp = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(tmp | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_MIPMAP);
	target = a5::Bitmap(w, h);
	al_set_new_bitmap_flags(tmp);
}
