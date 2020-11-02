#ifndef EOMAP_DIB_READER_HPP
#define EOMAP_DIB_READER_HPP

#include "common.hpp"

#include "cio.hpp"
#include "int_pack.hpp"

#include <cstdint>
#include <utility>

class dib_reader;

using read_fn_t = std::uint32_t (*)(const char* data, std::size_t offset) noexcept;
using extract_fn_t = void (*)(std::uint8_t imgbyte, std::uint8_t* entries_out) noexcept;
using read_line_fn_t = void (dib_reader::*)(char* outbuf, int row) noexcept;

class dib_reader
{
	//private:
	public:
		const char* data_ptr;
		std::size_t data_size;

		int           rs, gs, bs/*, as*/;
		std::uint32_t rm, gm, bm/*, am*/;

		int* rtable = nullptr;
		int* gtable = nullptr;
		int* btable = nullptr;
		//int* atable = nullptr;

		// for paletted images
		int pixels_per_byte = 0;
		std::uint8_t pixel_scan_mask = 0;

		std::uint32_t pal[256];

		union
		{
			read_fn_t read_fn;
			extract_fn_t extract_fn;
		};

		read_line_fn_t read_line_fn;

		std::uint8_t read_u8(size_t offset) const noexcept
		{
			char a = data_ptr[offset];

			return static_cast<std::uint8_t>(a);
		}

		std::uint16_t read_u16_le(size_t offset) const noexcept
		{
			char a = data_ptr[offset];
			char b = data_ptr[offset + 1];

			return int_pack_16_le(a, b);
		}

		std::uint32_t read_u32_le(size_t offset) const noexcept
		{
			char a = data_ptr[offset];
			char b = data_ptr[offset + 1];
			char c = data_ptr[offset + 2];
			char d = data_ptr[offset + 3];

			return int_pack_32_le(a, b, c, d);
		}

		void prepare_bitfields() noexcept;
		void prepare_palette() noexcept;

		void read_rgb_line(char* outbuf, int row) noexcept;
		void read_pal_line(char* outbuf, int row) noexcept;

	public:
		enum Compression
		{
			RGB = 0,
			RLE8 = 1,
			RLE4 = 2,
			BitFields = 3,
			JPEG = 4,
			PNG = 5
		};

		// The buffer pointed to by data_ptr must be at least 40 bytes
		dib_reader(const char* data_ptr, std::size_t data_size)
			: data_ptr(data_ptr)
			, data_size(data_size)
		{ }

		bool          v2_format()    const noexcept { return header_size() >= 52; }
		bool          v3_format()    const noexcept { return header_size() >= 56; }
		
		std::size_t   header_size()  const noexcept { return read_u32_le(0); }
		std::int32_t  width()        const noexcept { return read_u32_le(4); }
		std::int32_t  height()       const noexcept { return read_u32_le(8); }
		std::int16_t  color_planes() const noexcept { return 1; }
		std::int16_t  depth()        const noexcept { return read_u16_le(14); }
		Compression   compression()  const noexcept { return Compression(read_u32_le(16)); }
		std::uint32_t image_size()   const noexcept { return read_u32_le(20); }

		const char*   data()         const noexcept { return data_ptr + header_size() + palette_size(); }
		const char*   palette()      const noexcept { return data_ptr + header_size(); }
		const char*   raw_data()     const noexcept { return data_ptr; }
		
		std::int16_t  bpp()          const noexcept { return depth() >> 3; }
		std::int32_t  stride()       const noexcept { return width() * bpp() + ((4U - (width() * bpp())) & 3); }

		std::uint32_t red_mask()     const noexcept {
			return v2_format()
				? read_u32_le(40)
				: read_u32_le(header_size());
		}

		std::uint32_t green_mask()   const noexcept {
			return v2_format()
				? read_u32_le(44)
				: read_u32_le(header_size() + 4);
		}

		std::uint32_t blue_mask()    const noexcept {
			return v2_format()
				? read_u32_le(48)
				: read_u32_le(header_size() + 8);
		}

		std::uint32_t alpha_mask()   const noexcept {
			return v3_format()
				? read_u32_le(52)
				: 0;
		}

		bool is_paletted() const noexcept
		{
			return (compression() == RGB && depth() <= 8);
		}

		// Only meaningful if is_paletted()
		std::uint32_t num_colors() const noexcept
		{
			std::uint32_t num_colors = read_u32_le(32);

			if (num_colors == 0)
				num_colors = 1 << depth();

			return num_colors;
		}

		std::size_t palette_size() const noexcept
		{
			if (compression() == BitFields && !v2_format())
				return 12;
			else if (is_paletted())
				return num_colors() * 4;
			else
				return 0;
		}

		// Returns a pointer to a human readable string describing what's wrong with the file
		// Returns nullptr if the format is acceptable
		const char* check_format() const noexcept;

		// Attempts to adjust parameters to efficiently decode to the specified format
		// Returns the format in which the output bitmap should actually be locked
		ALLEGRO_PIXEL_FORMAT start(ALLEGRO_PIXEL_FORMAT fmt);

		// outbuf size must be at least width() * output bpp (4 bytes per pixel)
		void read_line(char* outbuf, int row) noexcept;
};

#endif // EOMAP_DIB_READER_HPP
