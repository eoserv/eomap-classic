#ifndef MAP_RENDERER_INCLUDED
#define MAP_RENDERER_INCLUDED

#include "common.hpp"

#include "EO_Map.hpp"
#include "GFX_Loader.hpp"

class Map_Renderer
{
	public:
		a5::Display &real_target;
		EO_Map *map = nullptr;
		GFX_Loader gfxloader;
		int xoff = 0, yoff = 0;
		int animation_state = 0;
		bool highlight_spec = false;
		int width = 0, height = 0;
		bool show_layers[12] = {};

		a5::Font& font;
		a5::Bitmap target;

		Map_Renderer(a5::Display &target_, a5::Font& font_) :
			real_target(target_), font(font_)
		{
			for (int i = 0; i < 10; ++i)
			{
				show_layers[i] = true;
			}

			show_layers[10] = false; // specials
			show_layers[11] = false; // grid lines

			this->RebuildTarget(target_.Width(), target_.Height());
		}

		void SetMap(EO_Map &map)
		{
			gfxloader.Reset();
			this->map = &map;
			this->width = std::max(map.width * 32, map.height * 32);
			this->height = std::max(map.width * 16, map.height * 16);
		}

		void Move(int x, int y)
		{
			this->xoff = x;
			this->yoff = y;
		}

		void ResetView()
		{
			this->Move(0 - (int(this->target.Width()) >> 1), 0);
		}

		void Render();

		void RebuildTarget(int w, int h);
};

#endif // MAP_RENDERER_INCLUDED
