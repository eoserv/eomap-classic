
#include "dib_reader.hpp"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

// For ALLEGRO_LITTLE_ENDIAN / ALLEGRO_BIG_ENDIAN
#include <allegro5/platform/alplatf.h>

#define NUM_CONVERT_TABLES 8

static bool convert_table_init = false;
static int convert_table[(1 << (NUM_CONVERT_TABLES + 1)) - 1];

static int* get_convert_table(int bit)
{
	return &convert_table[(1 << bit) - 1];
}

static void decode_bitfield(std::uint32_t m, int& shift_out, std::uint32_t& mask_out)
{
	int shift = 0;

	if (m == 0)
	{
		shift_out = 0;
		mask_out = 0;
		return;
	}

#ifdef __GNUC__
	shift = __builtin_ctz(m);
	m >>= shift;
#else
	while ((m & 1) == 0)
	{
		m >>= 1;
		++shift;
	}
#endif

	shift_out = shift;
	mask_out = m;
}

static void generate_scale_table(int* table, int entries)
{
	int tblsize = entries - 1;

	if (tblsize == 0)
	{
		table[0] = 0;
		return;
	}

	for (int i = 0; i < entries; ++i)
		table[i] = (i * 510 + tblsize) / tblsize / 2;
}

static read_fn_t read_fn_for_bpp(int bpp) noexcept
{
	switch (bpp)
	{
		case 2: // depth = 16
			return [](const char* data, std::size_t offset) noexcept
			{
				return std::uint32_t(int_pack_16_le(
					data[offset], data[offset + 1]
				));
			};

		case 3: // depth = 24
			return [](const char* data, std::size_t offset) noexcept
			{
				return int_pack_32_le(
					data[offset], data[offset + 1],
					data[offset + 2], 0
				);
			};

		case 4: // depth = 32
			return [](const char* data, std::size_t offset) noexcept
			{
				return int_pack_32_le(
					data[offset], data[offset + 1],
					data[offset + 2], data[offset + 3]
				);
			};
	}

	return nullptr;
}

static extract_fn_t extract_fn_for_depth(int depth) noexcept
{
	switch (depth)
	{
		case 1:
			return [](std::uint8_t imgbyte, std::uint8_t* entries_out) noexcept
			{
				for (int i = 0; i < 8; ++i)
					entries_out[7 - i] = (imgbyte & (1 << i)) >> i;
			};

		case 2:
			return [](std::uint8_t imgbyte, std::uint8_t* entries_out) noexcept
			{
				entries_out[0] = (imgbyte & 0xC0) >> 6;
				entries_out[1] = (imgbyte & 0x30) >> 4;
				entries_out[2] = (imgbyte & 0x0C) >> 2;
				entries_out[3] = (imgbyte & 0x03);
			};

		case 4:
			return [](std::uint8_t imgbyte, std::uint8_t* entries_out) noexcept
			{
				entries_out[0] = (imgbyte & 0xF0) >> 4;
				entries_out[1] = (imgbyte & 0x0F);
			};

		case 8:
			return [](std::uint8_t imgbyte, std::uint8_t* entries_out) noexcept
			{
				entries_out[0] = imgbyte;
			};
	}

	return nullptr;
}

void dib_reader::prepare_bitfields() noexcept
{
	if (compression() == BitFields)
	{
		decode_bitfield(red_mask(),   rs, rm);
		decode_bitfield(green_mask(), gs, gm);
		decode_bitfield(blue_mask(),  bs, bm);
		//decode_bitfield(alpha_mask(), as, am);
	}
	else
	{
		//as = am = 0;

		switch (depth())
		{
			case 16:
				decode_bitfield(0x00007C00U, rs, rm);
				decode_bitfield(0x000003E0U, gs, gm);
				decode_bitfield(0x0000001FU, bs, bm);
				break;

			// paletted images (1/2/4/8-bit) use 32-bit ARGB palette entries
			case 1:
			case 2:
			case 4:
			case 8:
			case 32:
				//decode_bitfield(0xFF000000U, as, am);
			case 24:
				decode_bitfield(0x00FF0000U, rs, rm);
				decode_bitfield(0x0000FF00U, gs, gm);
				decode_bitfield(0x000000FFU, bs, bm);
				break;
		}
	}

	if (!convert_table_init)
		convert_table[0] = 0;

	for (int i = 0; i <= NUM_CONVERT_TABLES; ++i)
	{
		std::uint32_t mask = ~(0xFFFFFFFFU << i) & 0xFFFFFFFFU;

		int* table = get_convert_table(i);
		int entries = (1 << i);

		if (!convert_table_init)
			generate_scale_table(table, entries);

		if (rm == mask) rtable = table;
		if (gm == mask) gtable = table;
		if (bm == mask) btable = table;
		// (am == mask) atable = table;
	}

	convert_table_init = true;
}

void dib_reader::prepare_palette() noexcept
{
	if (header_size() + palette_size() < data_size)
	{
		auto bytes = palette_size();

		if (bytes > sizeof pal)
			bytes = sizeof pal;

		std::memcpy(pal, palette(), bytes);

		if (bytes < sizeof pal)
		{
			char* zero_pal = reinterpret_cast<char*>(pal) + palette_size();
			std::memset(zero_pal, 0, sizeof pal - palette_size());
		}
	}
	else
	{
		std::memset(pal, 0, sizeof pal);
	}
}

const char* dib_reader::check_format() const noexcept
{
	if (data_size < 40 || data_size < std::size_t(header_size()))
		return "Truncated header";

	if (width() < 0)
		return "Image width less than zero";

	if (width() >= 0x40000000 || height() <= -0x40000000 || height() >= 0x40000000)
		return "Image dimensions out of range";

	if (depth() == 1 || depth() == 2 || depth() == 4 || depth() == 8)
	{
		if (compression() != RGB)
			return "Unsupported compression";
	}
	else if (depth() == 16 || depth() == 24 || depth() == 32)
	{
		if (compression() != RGB && compression() != BitFields)
			return "Unsupported compression";
	}
	else
	{
		return "Unsupported bit depth";
	}

	return nullptr;
}

ALLEGRO_PIXEL_FORMAT dib_reader::start(ALLEGRO_PIXEL_FORMAT fmt)
{
	if (is_paletted())
	{
		switch (depth())
		{
			case 1: pixels_per_byte = 8; break;
			case 2: pixels_per_byte = 4; break;
			case 4: pixels_per_byte = 2; break;
			case 8: pixels_per_byte = 1; break;
		}

		prepare_palette();

		this->extract_fn = extract_fn_for_depth(depth());
		this->read_line_fn = &dib_reader::read_pal_line;
	}
	else
	{
		this->read_fn = read_fn_for_bpp(bpp());
		this->read_line_fn = &dib_reader::read_rgb_line;
	}

	// Bitfield masks are also utilized for palette entry channel swaps
	prepare_bitfields();

	// Fixme: throwing an exception isn't so polite
	if (!rtable || !gtable || !btable/* || !atable*/)
		throw std::runtime_error("Bitfield mask too large");

#define SWAP_CHANNEL(x, y) \
	std::swap(x ## m, y ## m); \
	std::swap(x ## s, y ## s); \
	std::swap(x ## table, y ## table);

#ifdef ALLEGRO_BIG_ENDIAN
	SWAP_CHANNEL(r, b);
	return ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE;
#else // ALLEGRO_BIG_ENDIAN
	switch (fmt)
	{
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888:
		case ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE:
			SWAP_CHANNEL(r, b);
			return ALLEGRO_PIXEL_FORMAT_ABGR_8888;

		case ALLEGRO_PIXEL_FORMAT_XBGR_8888:
			SWAP_CHANNEL(r, b);
			return ALLEGRO_PIXEL_FORMAT_XBGR_8888;

		case ALLEGRO_PIXEL_FORMAT_XRGB_8888:
			return ALLEGRO_PIXEL_FORMAT_XRGB_8888;

		default:
			return ALLEGRO_PIXEL_FORMAT_ARGB_8888;
	}
#endif // ALLEGRO_BIG_ENDIAN
}

void dib_reader::read_rgb_line(char* outbuf, int row) noexcept
{
	int line = ((height() < 0) ? row : height() - 1 - row);
	const char* linebuf = data() + stride() * line;
	auto read_fn = read_fn_for_bpp(bpp());

	assert(read_fn);

	for (int i = 0; i < width(); i++)
	{
		std::size_t pixel_offset = linebuf - data_ptr;
		std::uint32_t pixel;

		if (pixel_offset + bpp() <= data_size)
			pixel = read_fn(data_ptr, pixel_offset);
		else
			pixel = 0;

		std::uint8_t r = rtable[((pixel >> rs) & rm)];
		std::uint8_t g = gtable[((pixel >> gs) & gm)];
		std::uint8_t b = btable[((pixel >> bs) & bm)];

		// ignore alpha bits -- use black as a mask instead
		std::uint8_t a = ((r|g|b) != 0) * 0xFF;

		outbuf[0] = b;
		outbuf[1] = g;
		outbuf[2] = r;
		outbuf[3] = a;

		linebuf += bpp();
		outbuf += 4;
	}
}

void dib_reader::read_pal_line(char* outbuf, int row) noexcept
{
	int line = ((height() < 0) ? row : height() - 1 - row);
	const char* linebuf = data() + stride() * line;
	auto extract_fn = extract_fn_for_depth(depth());

	assert(extract_fn);

	int bytes = (width() + (pixels_per_byte - 1)) / pixels_per_byte;
	int x = 0;

	std::uint8_t entries[8] = {};

	for (int i = 0; i < bytes; i++)
	{
		std::size_t pixel_offset = linebuf - data_ptr;
		std::uint8_t imgbyte;

		if (pixel_offset < data_size)
			imgbyte = read_u8(pixel_offset);
		else
			imgbyte = 0;

		extract_fn(imgbyte, entries);

		for (int ii = 0; ii < pixels_per_byte; ++ii)
		{
			std::uint32_t pixel = pal[entries[ii]];

			if (++x > width())
				break;

			std::uint8_t r = rtable[((pixel >> rs) & 0xFF)];
			std::uint8_t g = gtable[((pixel >> gs) & 0xFF)];
			std::uint8_t b = btable[((pixel >> bs) & 0xFF)];

			// ignore alpha bits -- use black as a mask instead
			std::uint8_t a = ((r|g|b) != 0) * 0xFF;

			outbuf[0] = b;
			outbuf[1] = g;
			outbuf[2] = r;
			outbuf[3] = a;

			outbuf += 4;
		}

		++linebuf;
	}
}

void dib_reader::read_line(char* outbuf, int row) noexcept
{
	auto fn = this->read_line_fn;
	(*this.*fn)(outbuf, row);
}
