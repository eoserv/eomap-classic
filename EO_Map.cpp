
#include "EO_Map.hpp"

extern "C"
{
#include "crc32.h"
}

std::unique_ptr<char> EOEN(unsigned int number)
{
	unsigned char *bytes = new unsigned char[4];
	unsigned int onumber = number;

	if (onumber >= 16194277)
	{
		bytes[3] = number / 16194277 + 1;
		number = number % 16194277;
	}
	else
	{
		bytes[3] = 254;
	}

	if (onumber >= 64009)
	{
		bytes[2] = number / 64009 + 1;
		number = number % 64009;
	}
	else
	{
		bytes[2] = 254;
	}

	if (onumber >= 253)
	{
		bytes[1] = number / 253 + 1;
		number = number % 253;
	}
	else
	{
		bytes[1] = 254;
	}

	bytes[0] = number + 1;

	return std::unique_ptr<char>(reinterpret_cast<char*>(bytes));
}

unsigned char EON(unsigned char b1)
{
	if (b1 == 0 || b1 == 254) b1 = 1;

	--b1;

	return b1;
}

unsigned short EON(unsigned char b1, unsigned char b2)
{
	if (b1 == 0 || b1 == 254) b1 = 1;
	if (b2 == 0 || b2 == 254) b2 = 1;

	--b1;
	--b2;

	return (b2*253 + b1);
}

unsigned int EON(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4)
{
	if (b1 == 0 || b1 == 254) b1 = 1;
	if (b2 == 0 || b2 == 254) b2 = 1;
	if (b3 == 0 || b3 == 254) b3 = 1;
	if (b4 == 0 || b4 == 254) b4 = 1;

	--b1;
	--b2;
	--b3;
	--b4;

	return (b4*16194277 + b3*64009 + b2*253 + b1);
}

static const char *safe_fail_filename;

static void safe_fail(int line)
{
	EOMAP_ERROR("Invalid file / failed read/seek: %s -- %i", safe_fail_filename, line);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); safe_fail(__LINE__); }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<std::size_t>(count)) {  std::fclose(fh); safe_fail(__LINE__); }
#define ATTEMPT_READ(buf, size, count, fh, success) if (std::fread(buf, size, count, fh) != static_cast<std::size_t>(count)) {  success = false; } else { success = true; }

void EO_Map::Load(std::string filename)
{
	safe_fail_filename = filename.c_str();

	FILE *fh = fopen(filename.c_str(), "rb");
	unsigned char buf[65535];
	int outersize;
	int innersize;
	int p;

	if (!fh)
	{
		EOMAP_ERROR("Failed to load this: %s", filename.c_str());
	}

	SAFE_READ(buf, sizeof(char), 0x2E, fh);

	if (buf[0x0] != 'E' || buf[0x1] != 'M' || buf[0x2] != 'F')
	{
		std::fclose(fh);
		EOMAP_ERROR("Not an EMF file: %s", filename.c_str());
	}

	this->revision = EON(buf[0x3], buf[0x4], buf[0x5], buf[0x6]);
	this->name = util::DecodeEMFString(std::string((char *)buf + 0x7, 24));

	std::size_t idx = this->name.find_first_of(char(0xFF));
	if (idx != std::string::npos)
		this->name.resize(idx);

	this->type = static_cast<EO_Map::Type>(EON(buf[0x1F]));
	this->effect = static_cast<EO_Map::Effect>(EON(buf[0x20]));
	this->music = EON(buf[0x21]);
	this->music_extra = EON(buf[0x22]);
	this->ambient_noise = EON(buf[0x23], buf[0x24]);
	this->width = EON(buf[0x25]);
	this->height = EON(buf[0x26]);
	this->fill_tile = EON(buf[0x27], buf[0x28]);
	this->map_available = EON(buf[0x29]);
	this->can_scroll = EON(buf[0x2A]);
	this->relog_x = EON(buf[0x2B]);
	this->relog_y = EON(buf[0x2C]);
	this->unknown = EON(buf[0x2D]);

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = EON(buf[0]);
	this->npcs.resize(outersize);
	SAFE_READ(buf, sizeof(char), outersize * 8, fh);
	p = 0;
	for (int i = 0; i < outersize; ++i)
	{
		this->npcs[i].x = EON(buf[p]);
		this->npcs[i].y = EON(buf[p + 1]);
		this->npcs[i].id = EON(buf[p + 2], buf[p + 3]);
		this->npcs[i].spawn_type = EON(buf[p + 4]);
		this->npcs[i].spawn_time = EON(buf[p + 5], buf[p + 6]);
		this->npcs[i].amount = EON(buf[p + 7]);
		p += 8;
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = EON(buf[0]);
	this->unknown1s.resize(outersize);
	SAFE_READ(buf, sizeof(char), outersize * 4, fh);
	p = 0;
	for (int i = 0; i < outersize; ++i)
	{
		for (int ii = 0; ii < 4; ++ii)
		{
			this->unknown1s[i].data[ii] = EON(buf[p + ii]);
		}
		p += 4;
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = EON(buf[0]);
	this->chests.resize(outersize);
	SAFE_READ(buf, sizeof(char), outersize * 12, fh);
	p = 0;
	for (int i = 0; i < outersize; ++i)
	{
		this->chests[i].x = EON(buf[p]);
		this->chests[i].y = EON(buf[p + 1]);
		this->chests[i].key = EON(buf[p + 2], buf[p + 3]);
		this->chests[i].slot = EON(buf[p + 4]);
		this->chests[i].item = EON(buf[p + 5], buf[p + 6]);
		this->chests[i].time = EON(buf[p + 7], buf[p + 8]);
		this->chests[i].amount = EON(buf[p + 9], buf[p + 10], buf[p + 11]);
		p += 12;
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = EON(buf[0]);
	this->tilerows.resize(outersize);
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 2, fh);
		this->tilerows[i].y = EON(buf[0]);
		innersize = EON(buf[1]);
		this->tilerows[i].tiles.resize(innersize);
		SAFE_READ(buf, sizeof(char), innersize * 2, fh);
		p = 0;
		for (int ii = 0; ii < innersize; ++ii)
		{
			this->tilerows[i].tiles[ii].x = EON(buf[p]);
			this->tilerows[i].tiles[ii].spec = static_cast<EO_Map::Tile_Spec>(EON(buf[p + 1]));
			p += 2;
		}
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = EON(buf[0]);
	this->warprows.resize(outersize);
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 2, fh);
		this->warprows[i].y = EON(buf[0]);
		innersize = EON(buf[1]);
		this->warprows[i].tiles.resize(innersize);
		SAFE_READ(buf, sizeof(char), innersize * 8, fh);
		p = 0;
		for (int ii = 0; ii < innersize; ++ii)
		{
			this->warprows[i].tiles[ii].x = EON(buf[p]);
			this->warprows[i].tiles[ii].warp_map = EON(buf[p + 1], buf[p + 2]);
			this->warprows[i].tiles[ii].warp_x = EON(buf[p + 3]);
			this->warprows[i].tiles[ii].warp_y = EON(buf[p + 4]);
			this->warprows[i].tiles[ii].level = EON(buf[p + 5]);
			this->warprows[i].tiles[ii].door = static_cast<EO_Map::Door>(EON(buf[p + 6], buf[p + 7]));
			p += 8;
		}
	}

    bool old_eomap = false;
	for (int layer = 0; layer < 9; ++layer)
	{
	    if (layer == 8)
	    {
	        ATTEMPT_READ(buf, sizeof(char), 1, fh, old_eomap);
	    }
        else
        {
            SAFE_READ(buf, sizeof(char), 1, fh);
        }
        if (layer == 8 && old_eomap) outersize = 0;
		outersize = EON(buf[0]);
		this->gfxrows[layer].resize(outersize);
		for (int i = 0; i < outersize; ++i)
		{
			SAFE_READ(buf, sizeof(char), 2, fh);
			this->gfxrows[layer][i].y = EON(buf[0]);
			innersize = EON(buf[1]);
			this->gfxrows[layer][i].tiles.resize(innersize);
			SAFE_READ(buf, sizeof(char), innersize * 3, fh);
			p = 0;
			for (int ii = 0; ii < innersize; ++ii)
			{
				this->gfxrows[layer][i].tiles[ii].x = EON(buf[p]);
				this->gfxrows[layer][i].tiles[ii].tile = EON(buf[p + 1], buf[p + 2]);
				p += 3;
			}
		}
	}

	ATTEMPT_READ(buf, sizeof(char), 1, fh, old_eomap);
	if (old_eomap)
	{
        outersize = EON(buf[0]);
        //printf("Total signs = %i \n", outersize);
        this->signs.resize(outersize);
	}
	else
        outersize = 0;
    p = 0;
    for (int i = 0; i < outersize; ++i)
    {
        SAFE_READ(buf, sizeof(char), 4, fh);
        this->signs[i].x = EON(buf[p]);
        this->signs[i].y = EON(buf[p + 1]);
        int msglen = EON(buf[p + 2], buf[p + 3]);// - 1;

        std::string data;
        char *databuf = new char[msglen];
        SAFE_READ(databuf, sizeof(unsigned char), msglen - 1, fh);

        data.assign(databuf, msglen - 1);
        if (!data.length())
            continue;
        data = util::DecodeEMFString(data).c_str();
        SAFE_READ(buf, sizeof(char), 1, fh);
        int ttllen = EON(buf[0]);

        this->signs[i].title = data.substr(0, ttllen);
        this->signs[i].message = data.substr(ttllen);
    }

	std::fclose(fh);

	this->loaded = true;

	//printf("%s\n", this->name);
}

#define WRITE(data, size, count, fh) do { crc = crc32(crc, reinterpret_cast<const u8 *>(&data), size); std::fwrite(reinterpret_cast<const char *>(data), size, count, fh); } while(0)
#define ENC_WRITE(number, size, count, fh) do { std::unique_ptr<char> bytes = EOEN(number); const char *p = bytes.get(); std::fwrite(p, size, count, fh); crc = crc32(crc, reinterpret_cast<const u8 *>(p), size); } while(0)

void EO_Map::Save(std::string filename)
{
	FILE *fh = std::fopen(filename.c_str(), "wb");
	u32 crc = 0;
	char single_buf = 0;

	std::fwrite("EMF", sizeof(char), 3, fh);
	single_buf = (crc >> 24) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf = (crc >> 16) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf = (crc >>  8) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf =  crc        & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	std::string namebuf_s = this->name;
	namebuf_s.resize(24, char(0xFF));
	namebuf_s = util::EncodeEMFString(namebuf_s);
	const char *namebuf = namebuf_s.data();
	WRITE(namebuf, sizeof(char), 24, fh);
	ENC_WRITE(unsigned(this->type), sizeof(char), 1, fh);
	ENC_WRITE(unsigned(this->effect), sizeof(char), 1, fh);
	ENC_WRITE(this->music, sizeof(char), 1, fh);
	ENC_WRITE(this->music_extra, sizeof(char), 1, fh);
	ENC_WRITE(this->ambient_noise, sizeof(char), 2, fh);
	ENC_WRITE(this->width, sizeof(char), 1, fh);
	ENC_WRITE(this->height, sizeof(char), 1, fh);
	ENC_WRITE(this->fill_tile, sizeof(char), 2, fh);
	ENC_WRITE(this->map_available, sizeof(char), 1, fh);
	ENC_WRITE(this->can_scroll, sizeof(char), 1, fh);
	ENC_WRITE(this->relog_x, sizeof(char), 1, fh);
	ENC_WRITE(this->relog_y, sizeof(char), 1, fh);
	ENC_WRITE(this->unknown, sizeof(char), 1, fh);

	ENC_WRITE(this->npcs.size(), sizeof(char), 1, fh);
	for (std::vector<EO_Map::NPC>::iterator i = this->npcs.begin(); i != this->npcs.end(); ++i)
	{
		ENC_WRITE(i->x, sizeof(char), 1, fh);
		ENC_WRITE(i->y, sizeof(char), 1, fh);
		ENC_WRITE(i->id, sizeof(char), 2, fh);
		ENC_WRITE(i->spawn_type, sizeof(char), 1, fh);
		ENC_WRITE(i->spawn_time, sizeof(char), 2, fh);
		ENC_WRITE(i->amount, sizeof(char), 1, fh);
	}

	ENC_WRITE(this->unknown1s.size(), sizeof(char), 1, fh);
	for (std::vector<EO_Map::Unknown_1>::iterator i = this->unknown1s.begin(); i != this->unknown1s.end(); ++i)
	{
		for (int ii = 0; ii < 4; ++ii)
		{
			ENC_WRITE(i->data[ii], sizeof(char), 1, fh);
		}
	}

	ENC_WRITE(this->chests.size(), sizeof(char), 1, fh);
	for (std::vector<EO_Map::Chest>::iterator i = this->chests.begin(); i != this->chests.end(); ++i)
	{
		ENC_WRITE(i->x, sizeof(char), 1, fh);
		ENC_WRITE(i->y, sizeof(char), 1, fh);
		ENC_WRITE(i->key, sizeof(char), 2, fh);
		ENC_WRITE(i->slot, sizeof(char), 1, fh);
		ENC_WRITE(i->item, sizeof(char), 2, fh);
		ENC_WRITE(i->time, sizeof(char), 2, fh);
		ENC_WRITE(i->amount, sizeof(char), 3, fh);
	}

	ENC_WRITE(this->tilerows.size(), sizeof(char), 1, fh);
	for (std::vector<EO_Map::Tile_Row>::iterator i = this->tilerows.begin(); i != this->tilerows.end(); ++i)
	{
		ENC_WRITE(i->y, sizeof(char), 1, fh);
		ENC_WRITE(i->tiles.size(), sizeof(char), 1, fh);
		for (std::vector<EO_Map::Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
		{
			ENC_WRITE(ii->x, sizeof(char), 1, fh);
			ENC_WRITE(unsigned(ii->spec), sizeof(char), 1, fh);
		}
	}

	ENC_WRITE(this->warprows.size(), sizeof(char), 1, fh);
	for (std::vector<EO_Map::Warp_Row>::iterator i = this->warprows.begin(); i != this->warprows.end(); ++i)
	{
		ENC_WRITE(i->y, sizeof(char), 1, fh);
		ENC_WRITE(i->tiles.size(), sizeof(char), 1, fh);
		for (std::vector<EO_Map::Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
		{
			ENC_WRITE(ii->x, sizeof(char), 1, fh);
			ENC_WRITE(ii->warp_map, sizeof(char), 2, fh);
			ENC_WRITE(ii->warp_x, sizeof(char), 1, fh);
			ENC_WRITE(ii->warp_y, sizeof(char), 1, fh);
			ENC_WRITE(ii->level, sizeof(char), 1, fh);
			ENC_WRITE(ii->door, sizeof(char), 2, fh);
		}
	}

	for (int layer = 0; layer < 9; ++layer)
	{
		ENC_WRITE(this->gfxrows[layer].size(), sizeof(char), 1, fh);
		for (std::vector<EO_Map::GFX_Row>::iterator i = this->gfxrows[layer].begin(); i != this->gfxrows[layer].end(); ++i)
		{
			ENC_WRITE(i->y, sizeof(char), 1, fh);
			ENC_WRITE(i->tiles.size(), sizeof(char), 1, fh);
			for (std::vector<EO_Map::GFX>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
			{
				ENC_WRITE(ii->x, sizeof(char), 1, fh);
				ENC_WRITE(ii->tile, sizeof(char), 2, fh);
			}
		}
	}

    ENC_WRITE(this->signs.size(), sizeof(char), 1, fh);

    for (std::vector<EO_Map::Sign>::iterator i = this->signs.begin(); i != this->signs.end(); ++i)
	{
        ENC_WRITE(i->x, sizeof(char), 1, fh);
        ENC_WRITE(i->y, sizeof(char), 1, fh);

        int msglen = util::EncodeEMFString(i->title + i->message).length() + 1;
        ENC_WRITE(msglen, sizeof(char), 2, fh);

        char *data = new char[msglen];
        data = (char *)util::EncodeEMFString(i->title + i->message).c_str();
        WRITE(data, sizeof(unsigned char), msglen - 1, fh);

        ENC_WRITE(i->title.length(), sizeof(char), 1, fh);
    }

	std::fseek(fh, 0x03, SEEK_SET);
	crc = crc | 0x01010101;
	single_buf = (crc >> 24) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf = (crc >> 16) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf = (crc >>  8) & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);
	single_buf =  crc        & 0xFF; std::fwrite(&single_buf, sizeof(char), 1, fh);

	std::fclose(fh);
}

void EO_Map::Cleanup()
{
	for (std::vector<NPC>::iterator i = npcs.begin(); i != npcs.end(); ++i)
	{
		while (i->x > width || i->y > height)
		{
			i = npcs.erase(i);

			if (i == npcs.end())
				goto end_npcs;
		}
	}
	end_npcs:

	for (std::vector<Chest>::iterator i = chests.begin(); i != chests.end(); ++i)
	{
		while (i->x > width || i->y > height)
		{
			i = chests.erase(i);

			if (i == chests.end())
				goto end_chests;
		}
	}
	end_chests:

	for (std::vector<Tile_Row>::iterator i = tilerows.begin(); i != tilerows.end(); ++i)
	{
		while (i->y > height)
		{
			i = tilerows.erase(i);

			if (i == tilerows.end())
				goto end_tilerows;
		}

		for (std::vector<Tile>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
		{
			while (ii->x > width)
			{
				ii = i->tiles.erase(ii);

				if (ii == i->tiles.end())
					goto end_tiles;
			}
		}
		end_tiles: ;
	}
	end_tilerows:

	for (std::vector<Warp_Row>::iterator i = warprows.begin(); i != warprows.end(); ++i)
	{
		while (i->y > height)
		{
			i = warprows.erase(i);

			if (i == warprows.end())
				goto end_warprows;
		}

		for (std::vector<Warp>::iterator ii = i->tiles.begin(); ii != i->tiles.end(); ++ii)
		{
			while (ii->x > width)
			{
				ii = i->tiles.erase(ii);

				if (ii == i->tiles.end())
					goto end_warps;
			}
		}
		end_warps: ;
	}
	end_warprows:

	for (std::size_t i = 0; i != 9; i++)
	{
		for (std::vector<GFX_Row>::iterator ii = gfxrows[i].begin(); ii != gfxrows[i].end(); ++ii)
		{
			while (ii->y > height)
			{
				ii = gfxrows[i].erase(ii);

				if (ii == gfxrows[i].end())
					goto end_warprows;
			}

			for (std::vector<GFX>::iterator iii = ii->tiles.begin(); iii != ii->tiles.end(); ++iii)
			{
				while (iii->x > width)
				{
					iii = ii->tiles.erase(iii);

					if (iii == ii->tiles.end())
						goto end_gfx;
				}
			}
			end_gfx: ;
		}
		end_gfxrows: ;
	}

	for (std::vector<Sign>::iterator i = signs.begin(); i != signs.end(); ++i)
	{
		while (i->x > width || i->y > height)
		{
			i = signs.erase(i);

			if (i == signs.end())
				goto end_signs;
		}
	}
	end_signs: ;
}
