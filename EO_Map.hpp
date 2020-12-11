#ifndef MAP_HPP_INCLUDED
#define MAP_HPP_INCLUDED

#include "common.hpp"

std::unique_ptr<char> EOEN(unsigned int number);
unsigned char EON(unsigned char b1);
unsigned short EON(unsigned char b1, unsigned char b2);
unsigned int EON(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4 = 254);

class EO_Map
{
	public:
		unsigned int revision;
		std::string name;

		enum class Type : unsigned short
		{
			Default,
			Unknown1,
			Unknown2,
			PK
		};

		Type type;

		enum class Effect : unsigned short
		{
			None,
			HPDrain,
			TPDrain,
			Quake
		};

		Effect effect;

		unsigned char music;
		unsigned char music_extra;
		unsigned short ambient_noise;
		unsigned char width;
		unsigned char height;
		unsigned short fill_tile;
		unsigned char map_available;
		unsigned char can_scroll;
		unsigned char relog_x;
		unsigned char relog_y;
		unsigned char unknown;

		enum class Tile_Spec : unsigned char
		{
			Wall,
			ChairDown,
			ChairLeft,
			ChairRight,
			ChairUp,
			ChairDownRight,
			ChairUpLeft,
			ChairAll,
			UnknownDoor,
			Chest,
			SpecUnknown1,
			SpecUnknown2,
			SpecUnknown3,
			SpecUnknown4,
			SpecUnknown5,
			SpecUnknown6,
			BankVault,
			NPCBoundary,
			MapEdge,
			FakeWall,
			Board1,
			Board2,
			Board3,
			Board4,
			Board5,
			Board6,
			Board7,
			Board8,
			Jukebox,
			Jump,
			Water,
			SpecUnknown7,
			Arena,
			AmbientSource,
			Spikes1,
			Spikes2,
			Spikes3
		};

		typedef unsigned short Door;
		static const Door NoDoor = 0;
		static const Door HasDoor = 1;

		struct NPC
		{
			unsigned char x;
			unsigned char y;
			unsigned short id;
			unsigned char spawn_type;
			unsigned short spawn_time;
			unsigned char amount;
		};

		std::vector<NPC> npcs;

		struct Unknown_1
		{
			unsigned char data[4];
		};

		std::vector<Unknown_1> unknown1s;

		struct Chest
		{
			unsigned char x;
			unsigned char y;
			unsigned short key;
			unsigned char slot;
			unsigned short item;
			unsigned short time;
			unsigned int amount;
		};

		std::vector<Chest> chests;

		struct Tile
		{
			unsigned char x;
			Tile_Spec spec;
		};

		struct Tile_Row
		{
			typedef Tile tile_type;
			unsigned char y;
			std::vector<Tile> tiles;
		};

		std::vector<Tile_Row> tilerows;

		struct Warp
		{
			unsigned char x;
			unsigned short warp_map;
			unsigned char warp_x;
			unsigned char warp_y;
			unsigned char level;
			Door door;
		};

		struct Warp_Row
		{
			typedef Warp tile_type;
			unsigned char y;
			std::vector<Warp> tiles;
		};

		std::vector<Warp_Row> warprows;

        struct Sign
        {
            unsigned char x;
            unsigned char y;
            std::string title;
            std::string message;
        };

        std::vector<Sign> signs;

		struct GFX
		{
			unsigned char x;
			short tile;
		};

		struct GFX_Row
		{
			typedef GFX tile_type;
			unsigned char y;
			std::vector<GFX> tiles;
		};

		std::vector<GFX_Row> gfxrows[9];

		bool loaded;

		EO_Map() :
		revision(0), type(Type::Default), effect(Effect::None),
		music(0), music_extra(0), ambient_noise(0),
		width(0), height(0), fill_tile(0),
		map_available(1), can_scroll(1), relog_x(0),
		relog_y(0), unknown(0), loaded(false)
		{ }

		void Load(std::string filename);

		template <class T> static typename T::value_type::tile_type *GetTile(T &rows, int x, int y)
		{
			for (typename T::iterator i = rows.begin(); i != rows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (typename std::vector<typename T::value_type::tile_type>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						return &*ii;
					}
				}
			}

			return nullptr;
		}

		template <class T> static void DelTile(T &rows, int x, int y)
		{
			for (typename T::iterator i = rows.begin(); i != rows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (typename std::vector<typename T::value_type::tile_type>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						i->tiles.erase(ii);
						return;
					}
				}
			}
		}

		template <class T> static void SetTile(T &rows, int tile, int x, int y)
		{
			for (typename T::iterator i = rows.begin(); i != rows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (typename std::vector<typename T::value_type::tile_type>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						ii->tile = tile;
						return;
					}
				}
			}

			GFX newtile;
			newtile.x = x;
			newtile.tile = tile;

			for (typename T::iterator i = rows.begin(); i != rows.end(); ++i)
			{
				if (i->y == y)
				{
					i->tiles.push_back(newtile);
					return;
				}
			}

			GFX_Row newrow;
			newrow.y = y;
			newrow.tiles.push_back(newtile);

			rows.push_back(newrow);
		}

		void DelTileSpec(int x, int y)
		{
			for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						i->tiles.erase(ii);
						return;
					}
				}
			}

			for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						i->tiles.erase(ii);
						return;
					}
				}
			}

			for (std::vector<Sign>::iterator i = signs.begin(); i != signs.end(); ++i)
			{
				if (i->x == x && i->y == y)
                {
                    signs.erase(i);
                    return;
                }
			}
		}

		void SetTileSpec(Tile_Spec tile, int x, int y)
		{
			for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						ii->spec = tile;
						return;
					}
				}
			}

			Tile newtile;
			newtile.x = x;
			newtile.spec = tile;

			for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
			{
				if (i->y == y)
				{
					i->tiles.push_back(newtile);
					return;
				}
			}

			Tile_Row newrow;
			newrow.y = y;
			newrow.tiles.push_back(newtile);

			tilerows.push_back(newrow);
		}

        int GetTileSpec(int x, int y)
		{
			for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						return int(ii->spec);
					}
				}
			}
			return -1;
        }

		void SetTileWarp(unsigned short warp_map, unsigned char warp_x, unsigned char warp_y, unsigned char level, Door door, int x, int y)
		{
			for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						ii->warp_map = warp_map;
						ii->warp_x = warp_x;
						ii->warp_y = warp_y;
						ii->level = level;
						ii->door = door;
						return;
					}
				}
			}

			Warp newtile;
			newtile.x = x;
			newtile.warp_map = warp_map;
			newtile.warp_x = warp_x;
			newtile.warp_y = warp_y;
			newtile.level = level;
			newtile.door = door;

			for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
			{
				if (i->y == y)
				{
					i->tiles.push_back(newtile);
					return;
				}
			}

			Warp_Row newrow;
			newrow.y = y;
			newrow.tiles.push_back(newtile);

			warprows.push_back(newrow);
		}

		Warp *GetWarpTile(int x, int y)
		{
			for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						return &*ii;
					}
				}
			}

			return 0;
		}

        std::vector<Chest> GetChestSpawns(int x, int y)
		{
		    std::vector<Chest> ret;
            for (std::vector<Chest>::iterator i = chests.begin(); i != chests.end(); ++i)
			{
				if (i->x == x && i->y == y)
                {
					ret.push_back(*i);
				}
			}
			return ret;
		}

        Chest *GetChestSpawn(Chest spawn)
		{
            for (std::vector<Chest>::iterator i = chests.begin(); i != chests.end(); ++i)
			{
				if (i->x == spawn.x && i->y == spawn.y && i->key == spawn.key
				 && i->slot == spawn.slot && i->item == spawn.item && i->time == spawn.time
                 && i->amount == spawn.amount)
                {
                        return &*i;
                }
			}
			return 0;
		}

		bool DelChestSpawn(Chest spawn)
		{
            for (std::vector<Chest>::iterator i = chests.begin(); i != chests.end(); ++i)
			{
				if (i->x == spawn.x && i->y == spawn.y && i->key == spawn.key
				 && i->slot == spawn.slot && i->item == spawn.item && i->time == spawn.time
                 && i->amount == spawn.amount)
                {
                        chests.erase(i);
                        return true;
                }
			}
			return false;
		}

        std::vector<NPC> GetNPCSpawns(int x, int y)
		{
		    std::vector<NPC> ret;
            for (std::vector<NPC>::iterator i = npcs.begin(); i != npcs.end(); ++i)
			{
				if (i->x == x && i->y == y)
                {
					ret.push_back(*i);
				}
			}
			return ret;
		}

        NPC *GetNPCSpawn(NPC spawn)
		{
            for (std::vector<NPC>::iterator i = npcs.begin(); i != npcs.end(); ++i)
			{
				if (i->x == spawn.x && i->y == spawn.y && i->id == spawn.id
				 && i->spawn_type == spawn.spawn_type && i->spawn_time == spawn.spawn_time
                 && i->amount == spawn.amount)
                {
                        return &*i;
                }
			}
			return 0;
		}

		bool DelNPCSpawn(NPC spawn)
		{
            for (std::vector<NPC>::iterator i = npcs.begin(); i != npcs.end(); ++i)
			{
				if (i->x == spawn.x && i->y == spawn.y && i->id == spawn.id
				 && i->spawn_type == spawn.spawn_type && i->spawn_time == spawn.spawn_time
                 && i->amount == spawn.amount)
                {
                        npcs.erase(i);
                        return true;
                }
			}
			return false;
		}

        Sign *GetSign(int x, int y)
		{
            for (std::vector<Sign>::iterator i = signs.begin(); i != signs.end(); ++i)
			{
				if (i->x == x && i->y == y)
                {
                        return &*i;
                }
			}
			return 0;
		}

		void SetTileSign(std::string title, std::string message, int x, int y)
		{
			for (std::vector<Sign>::iterator i = signs.begin(); i != signs.end(); ++i)
			{
				if (i->x != x || i->y != y)
				{
					continue;
				}

                i->x = x;
                i->y = y;
                i->title = title;
                i->message = message;
				return;
			}

			Sign newsign;
			newsign.x = x;
			newsign.y= y;
			newsign.title = title;
			newsign.message = message;

			signs.push_back(newsign);
		}

        int GetObject(int x, int y)
        {
			for (std::vector<GFX_Row>::iterator i = gfxrows[1].begin(); i != gfxrows[1].end(); ++i)
			{
				if (i->y != y)
				{
					continue;
				}

				for (std::vector<GFX>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
				{
					if (ii->x == x)
					{
						return ii->tile;
					}
				}
			}
			return -1;
        }

        bool HasSomething(int x, int y)
        {
            for (std::vector<NPC>::iterator i = npcs.begin(); i != npcs.end(); ++i)
            {
                if (i->x == x && i->y == y) return true;
            }
            for (std::vector<Chest>::iterator i = chests.begin(); i != chests.end(); ++i)
            {
                if (i->x == x && i->y == y) return true;
            }
            for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
            {
                if (i->y != y) continue;
                for (std::vector<Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
                {
                    if (ii->x == x) return true;
                }
                break;
            }
            for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
            {
                if (i->y != y) continue;
                for (std::vector<Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
                {
                    if (ii->x == x) return true;
                }
                break;
            }

            for (std::size_t i = 0; i != 9; i++)
            {
                for (std::vector<GFX_Row>::iterator ii = gfxrows[i].begin(); ii != gfxrows[i].end(); ++ii)
                {
                    if (ii->y != y) continue;
                    for (std::vector<GFX>::iterator iii = ii->tiles.begin(); iii != ii->tiles.end(); ++iii)
                    {
                        if (iii->x == x) return true;
                    }
                    break;
                }
            }
            for (std::vector<Sign>::iterator i = signs.begin(); i != signs.end(); ++i)
            {
                if (i->x == x && i->y == y) return true;
            }
            return false;
        }

        void Cleanup();

		void Save(std::string filename);
};

#endif // MAP_HPP_INCLUDED
