#include "bmp_reader.hpp"

#include "cio_physfs.hpp"
#include "dib_reader.hpp"
#include "int_pack.hpp"

a5::Bitmap load_bmp(const char* filename, std::uint32_t mask_color)
{
	cio::physfs_stream file(filename);

	if (!file.is_open())
		EOMAP_ERROR("Could not open file %s:\n%s", filename, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));

	char bm[14];

	if (file.read(bm, 14) != 14 || bm[0] != 'B' || bm[1] != 'M')
		EOMAP_ERROR("%s is not a valid BMP file", filename);

	std::uint32_t bmsize = int_pack_32_le(bm + 2);

	if (bmsize < 54)
		EOMAP_ERROR("%s is not a valid BMP file", filename);

	std::uint32_t dibsize = bmsize - 14;

	auto buf = std::make_unique<char[]>(dibsize);
	std::size_t total_read = 0;

	while (total_read < bmsize - 14)
	{
		std::size_t bytes_read = file.read(buf.get() + total_read, dibsize - total_read);

		if (bytes_read == 0)
		{
			if (file.error())
				EOMAP_ERROR("IO error reading file %s:\n%s", filename, file.errstr());

			break;
		}

		total_read += bytes_read;
	}

	dib_reader reader(buf.get(), dibsize);

	auto check_result = reader.check_format();

	if (check_result != nullptr)
		EOMAP_ERROR("Can't load %s: %s", filename, check_result);

	int w = reader.width();
	int h = reader.height();

	a5::Bitmap bmp(w, h);

	auto dpyfmt = static_cast<ALLEGRO_PIXEL_FORMAT>(
		al_get_bitmap_format(bmp)
	);

	reader.mask_color = mask_color;

	auto fmt = static_cast<a5::Pixel_Format::Format>(
		reader.start(dpyfmt)
	);

	auto lock = bmp.Lock(fmt, a5::Bitmap::WriteOnly);

	char* start = reinterpret_cast<char*>(lock.Data());
	auto pitch = lock.Pitch();
	int rows = bmp.Height();

	for (int i = 0; i < rows; ++i)
	{
		char* row = start + pitch * i;
		reader.read_line(row, i);
	}

	return bmp;
}
