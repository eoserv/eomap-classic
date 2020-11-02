#include "GFX_Loader.hpp"
#include "cio.hpp"
#include "dib_reader.hpp"

extern std::string g_eo_install_path;

static void do_dib_copy(a5::Bitmap& bmp, dib_reader& reader, int rows)
{
	auto dpyfmt = al_get_display_format(al_get_current_display());

	if (dpyfmt == a5::Pixel_Format::ARGB_8888 || dpyfmt == a5::Pixel_Format::XRGB_8888)
	{
		auto lock = bmp.Lock(a5::Pixel_Format::ARGB_8888, a5::Bitmap::WriteOnly);
		char* start = reinterpret_cast<char*>(lock.Data());
		auto pitch = lock.Pitch();

		for (int i = 0; i < rows; ++i)
		{
			char* row = start + pitch * i;
			reader.read_line_argb(row, i);
		}
	}
	else
	{
		auto lock = bmp.Lock(a5::Pixel_Format::ABGR_8888, a5::Bitmap::WriteOnly);
		char* start = reinterpret_cast<char*>(lock.Data());
		auto pitch = lock.Pitch();

		for (int i = 0; i < rows; ++i)
		{
			char* row = start + pitch * i;
			reader.read_line_abgr(row, i);
		}
	}
}

std::unique_ptr<a5::Bitmap> GFX_Loader::Module::LoadBitmapUncached(int id)
{
	if (file_id == 3 && id == 100)
	{
		loader->blankbmp = al_load_bitmap("blank.bmp");
		al_convert_mask_to_alpha(loader->blankbmp, al_map_rgb(255, 0, 255));

		if (!loader->blankbmp_ptr)
			loader->blankbmp_ptr = std::make_unique<a5::Bitmap>(loader->blankbmp);

		return std::make_unique<a5::Bitmap>(loader->blankbmp, false);
	}

	auto bmp_table_it = bmp_table.find(id);

	if (bmp_table_it == bmp_table.end())
	{
		if (!loader->errbmp)
		{
			loader->errbmp = al_load_bitmap("error.bmp");
			al_convert_mask_to_alpha(loader->errbmp, al_map_rgb(255, 0, 255));
		}

		if (!loader->errbmp_ptr)
			loader->errbmp_ptr = std::make_unique<a5::Bitmap>(loader->errbmp);

		return std::make_unique<a5::Bitmap>(loader->errbmp, false);
	}

	auto&& info = bmp_table_it->second;

	auto buf = std::make_unique<char[]>(info.size);
	egf_reader.read_resource(buf.get(), info.start, info.size);

	dib_reader reader(buf.get(), info.size);
	reader.start();

	auto check_result = reader.check_format();

	if (check_result)
	{
		if (!loader->errbmp)
		{
			loader->errbmp = al_load_bitmap("error.bmp");
			al_convert_mask_to_alpha(loader->errbmp, al_map_rgb(255, 0, 255));
		}

		if (!loader->errbmp_ptr)
			loader->errbmp_ptr = std::make_unique<a5::Bitmap>(loader->errbmp);

		return std::make_unique<a5::Bitmap>(loader->errbmp, false);
	}

	auto bmp = std::make_unique<a5::Bitmap>(reader.width(), reader.height());

	do_dib_copy(*bmp, reader, info.height);

	return bmp;
}

GFX_Loader::Module& GFX_Loader::LoadModule(int file)
{
	auto cache_it = module_cache.find(file);

	if (cache_it != module_cache.end())
		return cache_it->second;

	char suffix[sizeof "/gfx/gfx.egf" + 3];
	snprintf(suffix, sizeof suffix, "/gfx/gfx%03i.egf", file);
	std::string filename = g_eo_install_path + suffix;

	cio::stream module_file(filename.c_str(), "rb");

	if (!module_file)
		EOMAP_ERROR("Failed to open: %s", filename.c_str());

	pe_reader module_reader(std::move(module_file));

	if (!module_reader.read_header())
		EOMAP_ERROR("Failed to load library: %s", filename.c_str());

	auto&& bmp_table = module_reader.read_bitmap_table();

	auto emplace_result = module_cache.emplace(
		file,
		Module{this, file, std::move(module_reader), std::move(bmp_table)}
	);

	return emplace_result.first->second;
}

void GFX_Loader::Prepare(int file)
{
	LoadModule(file);
}

int GFX_Loader::CountBitmaps(int file)
{
	GFX_Loader::Module& module = this->LoadModule(file);
	return module.bmp_table.size();
}

pe_reader::BitmapInfo GFX_Loader::Info(int file, int id)
{
	GFX_Loader::Module& module = this->LoadModule(file);

	auto info_it = module.bmp_table.find(100 + id);

	if (info_it == module.bmp_table.end())
		return {};

	return info_it->second;
}

a5::Bitmap& GFX_Loader::Load(int file, int id, int anim)
{
	auto info = Info(file, id);

	if ((file != 3 && file != 6) || info.width < 128)
		anim = 0;

	auto cache_it = anim_cache.find(BmpFrame{file, id, anim});

	if (cache_it != anim_cache.end())
		return *cache_it->second;

	if (frame_load_allocation-- < 0)
	{
		if (!blankbmp)
		{
			blankbmp = al_create_bitmap(0, 0);
		}

		if (!blankbmp_ptr)
			blankbmp_ptr = std::make_unique<a5::Bitmap>(blankbmp);

		return *blankbmp_ptr;
	}

	bool held = al_is_bitmap_drawing_held();

	if (held)
		al_hold_bitmap_drawing(false);

	GFX_Loader::Module& module = this->LoadModule(file);
	auto bmp_ptr = module.LoadBitmapUncached(100 + id);
	auto& bmp = *bmp_ptr;

	if (bmp == errbmp)
		return *errbmp_ptr;

	int bmpw = bmp.Width();
	int bmph = bmp.Height();

	auto anim_rect = [&]() -> a5::Rectangle
	{
		if (file == 3)
		{
			if (bmpw >= 120)
				return a5::Rectangle(anim * 64, 0, anim * 64 + 64, 32);
			else
				return a5::Rectangle(0, 0, 64, 32);
		}
		else if (file == 6 && bmpw >= 120)
		{
			auto frame_width = bmpw / 4;
			return a5::Rectangle(anim * frame_width, 0, (anim + 1) * frame_width, bmph);
		}
		else
		{
			return a5::Rectangle(0, 0, bmpw, bmph);
		}
	}();

	auto atlas_anim_bmp = atlas[anim]->Add(bmp, anim_rect);

	auto emplace_result = anim_cache.emplace(
		BmpFrame{file, id, anim},
		std::move(atlas_anim_bmp)
	);

	if (held)
		al_hold_bitmap_drawing(true);

	return *emplace_result.first->second;
}

a5::Bitmap& GFX_Loader::LoadRaw(std::string filename)
{
	auto cache_it = raw_bmp_cache.find(filename);

	if (cache_it != raw_bmp_cache.end())
		return *cache_it->second;

	auto graphic = std::make_unique<a5::Bitmap>(filename.c_str());
	al_convert_mask_to_alpha(*graphic, al_map_rgb(255, 0, 255));
	auto emplace_result = raw_bmp_cache.emplace(filename, std::move(graphic));
	return *emplace_result.first->second;
}

bool GFX_Loader::IsError(a5::Bitmap& bmp)
{
	return (ALLEGRO_BITMAP*)bmp == errbmp;
}

void GFX_Loader::Reset()
{
	anim_cache.clear();

	if (!atlas[0])
	{
		int max_size = al_get_display_option(al_get_current_display(), ALLEGRO_MAX_BITMAP_SIZE);

		if (max_size >= 2048)
			atlas[0] = std::make_unique<a5::Atlas>(2048, 2048, 32, 32);
		else
			atlas[0] = std::make_unique<a5::Atlas>(1024, 1024, 32, 32);

		atlas[1] = std::make_unique<a5::Atlas>(512, 512, 32, 32);
		atlas[2] = std::make_unique<a5::Atlas>(512, 512, 32, 32);
		atlas[3] = std::make_unique<a5::Atlas>(512, 512, 32, 32);
	}
	else
	{
		atlas[0]->Clear(true);
		atlas[1]->Clear(true);
		atlas[2]->Clear(true);
		atlas[3]->Clear(true);
	}
}
