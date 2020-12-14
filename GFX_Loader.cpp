#include "GFX_Loader.hpp"
#include "bmp_reader.hpp"
#include "cio.hpp"
#include "dib_reader.hpp"
#include "common.hpp"

extern std::string g_eo_install_path;

std::unique_ptr<a5::Bitmap> GFX_Loader::Module::LoadBitmapUncached(int id)
{
	auto bmp_table_it = bmp_table.find(id);

	if (bmp_table_it == bmp_table.end())
	{
		if (!loader->errbmp)
		{
			loader->errbmp = load_bmp("error.bmp").Release();
		}

		if (!loader->errbmp_ptr)
			loader->errbmp_ptr = std::make_unique<a5::Bitmap>(loader->errbmp);

		return std::make_unique<a5::Bitmap>(loader->errbmp, false);
	}

	auto&& info = bmp_table_it->second;

	auto buf = std::make_unique<char[]>(info.size);
	egf_reader.read_resource(buf.get(), info.start, info.size);

	dib_reader reader(buf.get(), info.size);

	auto check_result = reader.check_format();

	if (check_result)
	{
		fprintf(stderr, "Can't load BMP %d/%d: %s\n", file_id, id, check_result);
		fflush(stderr);

		if (!loader->errbmp)
		{
			loader->errbmp = load_bmp("error.bmp").Release();
		}

		if (!loader->errbmp_ptr)
			loader->errbmp_ptr = std::make_unique<a5::Bitmap>(loader->errbmp);

		return std::make_unique<a5::Bitmap>(loader->errbmp, false);
	}

	auto bmp = std::make_unique<a5::Bitmap>(reader.width(), reader.height());

	auto dpyfmt = static_cast<ALLEGRO_PIXEL_FORMAT>(
		al_get_bitmap_format(*bmp)
	);

	auto fmt = static_cast<a5::Pixel_Format::Format>(
		reader.start(dpyfmt)
	);

	auto lock = bmp->Lock(fmt, a5::Bitmap::WriteOnly);

	char* start = reinterpret_cast<char*>(lock.Data());
	auto pitch = lock.Pitch();
	int rows = bmp->Height();

	for (int i = 0; i < rows; ++i)
	{
		char* row = start + pitch * i;
		reader.read_line(row, i);
	}

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

void GFX_Loader::SetLoadTime(double secs)
{
	this->frame_load_until = al_get_time() + secs;
	this->dummy_frames_loaded = 0;
}

bool GFX_Loader::CanLoadFrames() const
{
	return (this->frame_load_until - al_get_time()) > 0.0;
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
	bool is_animation = (file == 3 || file == 6) && info.width >= 128;

	if (!is_animation)
		anim = 0;

	auto cache_it = anim_cache.find(BmpFrame{file, id, anim});

	if (cache_it != anim_cache.end())
		return *cache_it->second;

	bool is_partially_loaded_animation = is_animation &&
		std::find_if(
			anim_cache.begin(),
			anim_cache.end(),
			[file, id](auto& entry) { return entry.first.file == file && entry.first.id == id; }
		) != anim_cache.end();

	bool can_load = (CanLoadFrames() || is_partially_loaded_animation);

	if (!can_load)
		++this->dummy_frames_loaded;

	if (id == 0 || !can_load)
	{
		if (!nullbmp)
		{
			nullbmp = al_create_bitmap(1, 1);
		}

		if (!nullbmp_ptr)
			nullbmp_ptr = std::make_unique<a5::Bitmap>(nullbmp);

		return *nullbmp_ptr;
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
			if (bmpw >= 128)
				return a5::Rectangle(anim * 64, 0, anim * 64 + 64, 32);
			else
				return a5::Rectangle(0, 0, 64, 32);
		}
		else if (file == 6 && bmpw >= 128)
		{
			auto frame_width = bmpw / 4;
			return a5::Rectangle(anim * frame_width, 0, (anim + 1) * frame_width, bmph);
		}
		else
		{
			return a5::Rectangle(0, 0, bmpw, bmph);
		}
	}();

	// Animations larger than 512x512 can't be loaded without increasing the
	// anim texture atlas size

	if (anim > 0)
	{
		if (anim_rect.Width() > 512)
			anim_rect.x2 = anim_rect.x1 + 512;

		if (anim_rect.Height() > 512)
			anim_rect.y2 = anim_rect.y1 + 512;
	}

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

	auto graphic = std::make_unique<a5::Bitmap>(load_bmp(filename.c_str()));
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
