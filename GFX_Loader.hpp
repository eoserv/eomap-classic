#ifndef GFX_LOADER_INCLUDED
#define GFX_LOADER_INCLUDED

#include "common.hpp"
#include "a5ses/Atlas.hpp"

#include "pe_reader.hpp"

class GFX_Loader
{
	protected:
		struct Module
		{
			GFX_Loader* loader;
			int file_id;
			pe_reader egf_reader;
			std::map<int, pe_reader::BitmapInfo> bmp_table;
			//std::map<int, std::unique_ptr<a5::Bitmap>> bmp_cache;

			//a5::Bitmap& LoadBitmap(int id);
			std::unique_ptr<a5::Bitmap> LoadBitmapUncached(int id);
		};

		struct BmpFrame
		{
			int file;
			int id;
			int frame;

			bool operator<(const BmpFrame& other) const noexcept
			{
				return (file == other.file)
					? (id == other.id)
						? (frame < other.frame)
						: (id < other.id)
					: (file < other.file);
			}
		};

		std::map<BmpFrame, std::unique_ptr<a5::Bitmap>> anim_cache;
		std::map<int, Module> module_cache;
		std::map<std::string, std::unique_ptr<a5::Bitmap>> raw_bmp_cache;

		std::unique_ptr<a5::Atlas> atlas[4]{};

		Module& LoadModule(int file);

		ALLEGRO_BITMAP* nullbmp = nullptr;
		std::unique_ptr<a5::Bitmap> nullbmp_ptr = nullptr;

		ALLEGRO_BITMAP* errbmp = nullptr;
		std::unique_ptr<a5::Bitmap> errbmp_ptr = nullptr;

	public:
		int frame_load_allocation = 0;

		GFX_Loader() { Reset(); }

		void Prepare(int file);
		int CountBitmaps(int file);
		pe_reader::BitmapInfo Info(int file, int id);
		a5::Bitmap& Load(int file, int id, int anim = 0);
		a5::Bitmap& LoadRaw(std::string filename);

		bool IsError(a5::Bitmap&);

		void Reset();
};

#endif // GFX_LOADER_INCLUDED
