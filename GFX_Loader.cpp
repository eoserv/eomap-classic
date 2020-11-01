#include "GFX_Loader.hpp"
#include "cio.hpp"
#include "dib_reader.hpp"

extern std::string g_eo_install_path;
static ALLEGRO_BITMAP* g_errbmp = nullptr;
static std::unique_ptr<a5::Bitmap> g_errbmp_ptr = nullptr;
/*
a5::Bitmap& GFX_Loader::Module::LoadBitmap(int id)
{
	auto cache_it = bmp_cache.find(id);

	if (cache_it != bmp_cache.end())
		return *cache_it->second;

	auto bmp = LoadBitmapUncached(id);

	auto emplace_result = bmp_cache.emplace(id, std::move(bmp));

	return *emplace_result.first->second;
}*/

std::unique_ptr<a5::Bitmap> GFX_Loader::Module::LoadBitmapUncached(int id)
{
	if (file_id == 3 && id == 100)
	{
		auto blankbmp = std::make_unique<a5::Bitmap>("blank.bmp");
		al_convert_mask_to_alpha(*blankbmp, al_map_rgb(255, 0, 255));
		return blankbmp;
	}

	auto bmp_table_it = bmp_table.find(id);

	if (bmp_table_it == bmp_table.end())
	{
		//EOMAP_ERROR("Failed to load bitmap: %d/%d", file_id, id);
		if (!g_errbmp)
		{
			g_errbmp = al_load_bitmap("error.bmp");
			al_convert_mask_to_alpha(g_errbmp, al_map_rgb(255, 0, 255));
		}

		if (!g_errbmp_ptr)
			g_errbmp_ptr = std::make_unique<a5::Bitmap>(g_errbmp);

		return std::make_unique<a5::Bitmap>(g_errbmp, false);
	}

	auto&& info = bmp_table_it->second;

	auto buf = std::make_unique<char[]>(info.size);
	egf_reader.read_resource(buf.get(), info.start, info.size);

	dib_reader reader(buf.get(), info.size);
	reader.start();

	auto check_result = reader.check_format();

	if (check_result)
	{
		//EOMAP_ERROR("Failed to load bitmap: %d/%d (DIB error: %s)", file_id, id, check_result);
		if (!g_errbmp)
		{
			g_errbmp = al_load_bitmap("error.bmp");
			al_convert_mask_to_alpha(g_errbmp, al_map_rgb(255, 0, 255));
		}

		if (!g_errbmp_ptr)
			g_errbmp_ptr = std::make_unique<a5::Bitmap>(g_errbmp);

		return std::make_unique<a5::Bitmap>(g_errbmp, false);
	}

	auto bmp = std::make_unique<a5::Bitmap>(reader.width(), reader.height());

	{
		auto lock = bmp->Lock(a5::Pixel_Format::ABGR_8888, a5::Bitmap::WriteOnly);

		for (int i = 0; i < info.height; ++i)
		{
			char* row = reinterpret_cast<char*>(lock.Data()) + lock.Pitch() * i;
			reader.read_line(row, i);
		}
	}

	al_convert_mask_to_alpha(*bmp, al_map_rgb(0, 0, 0));

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
		Module{file, std::move(module_reader), std::move(bmp_table)}
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

	bool held = al_is_bitmap_drawing_held();

	if (held)
		al_hold_bitmap_drawing(false);

	GFX_Loader::Module& module = this->LoadModule(file);
	auto bmp_ptr = module.LoadBitmapUncached(100 + id);
	auto& bmp = *bmp_ptr;

	if (bmp == g_errbmp)
		return *g_errbmp_ptr;

	auto anim_bmp = [&]() -> std::unique_ptr<a5::Bitmap>
	{
		if (file == 3)
		{
			if (bmp.Width() >= 128)
				return bmp.Sub(a5::Rectangle(anim * 64, 0, anim * 64 + 64, 32));
			else
				return bmp.Sub(a5::Rectangle(0, 0, 64, 32));
		}
		else if (file == 6 && bmp.Width() >= 128)
		{
			auto frame_width = bmp.Width() / 4;
			return bmp.Sub(a5::Rectangle(anim * frame_width, 0, (anim + 1) * frame_width, bmp.Height()));
		}
		else
		{
			return bmp.Sub(a5::Rectangle(0, 0, bmp.Width(), bmp.Height()));
		}
	}();

	auto atlas_anim_bmp = atlas[anim]->Add(*anim_bmp);

	auto emplace_result = anim_cache.emplace(
		BmpFrame{file, id, anim},
		std::move(atlas_anim_bmp ? atlas_anim_bmp : anim_bmp)
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
	return (ALLEGRO_BITMAP*)bmp == g_errbmp;
}

void GFX_Loader::Reset()
{
	anim_cache.clear();

	if (!atlas[0])
	{
		int max_size = al_get_display_option(al_get_current_display(), ALLEGRO_MAX_BITMAP_SIZE);

		if (max_size >= 2048)
			atlas[0] = std::make_unique<a5::Atlas>(2048, 2048, 32, 32);
		else if (max_size >= 1024)
			atlas[0] = std::make_unique<a5::Atlas>(1024, 1024, 32, 32);
		else
			atlas[0] = std::make_unique<a5::Atlas>(512, 512, 32, 32);

		atlas[1] = std::make_unique<a5::Atlas>(256, 256, 32, 32);
		atlas[2] = std::make_unique<a5::Atlas>(256, 256, 32, 32);
		atlas[3] = std::make_unique<a5::Atlas>(256, 256, 32, 32);
	}
	else
	{
		atlas[0]->Clear(true);
		atlas[1]->Clear(true);
		atlas[2]->Clear(true);
		atlas[3]->Clear(true);
	}
}
