#ifndef CIO_CIO_HPP
#define CIO_CIO_HPP

#include <cstdio>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace cio
{

enum class mode
{
	read         = 1 << 0,
	write        = 1 << 1,
	append       = 1 << 2,
	text         = 1 << 3,
	no_overwrite = 1 << 4
};

constexpr mode operator|(mode a, mode b) noexcept
{
	return mode(unsigned(a) | unsigned(b));
}

constexpr mode operator&(mode a, mode b) noexcept
{
	return mode(unsigned(a) & unsigned(b));
}

namespace impl
{
	std::string flags_to_modestr(cio::mode flags) noexcept;
}

struct no_ownership_t { };
static constexpr no_ownership_t no_ownership = {};

class stream
{
	private:
		std::FILE* fh;
		bool m_unowned = false;

	public:
		typedef std::FILE* element_type;

		stream(std::FILE* fh) noexcept
			: fh(fh)
		{ }

		stream(std::FILE* fh, no_ownership_t) noexcept
			: fh(fh)
			, m_unowned(true)
		{ }

		explicit stream(const char* filename, cio::mode flags = {}) noexcept
			: fh(nullptr)
		{
			open(filename, flags);
		}

		stream(const char* filename, const char* modestr) noexcept
			: fh(nullptr)
		{
			open(filename, modestr);
		}

		stream(stream&& other) noexcept
			: fh(other.fh)
			, m_unowned(other.m_unowned)
		{
			other.fh = nullptr;
			other.m_unowned = true;
		}

		stream(const stream&) = delete;
		stream& operator=(const stream&) = delete;

		bool open(const char* filename, cio::mode flags = {}) noexcept
		{
			if (fh && !m_unowned)
				std::fclose(fh);

			fh = std::fopen(filename, impl::flags_to_modestr(flags).c_str());
			m_unowned = false;

			return (fh != nullptr);
		}

		bool open(const char* filename, const char* modestr) noexcept
		{
			if (fh && !m_unowned)
				std::fclose(fh);

			fh = std::fopen(filename, modestr);
			m_unowned = false;

			return (fh != nullptr);
		}
		
		void close() noexcept
		{
			std::fclose(fh);
			fh = nullptr;
		}

		std::FILE* handle() noexcept
		{
			return fh;
		}

		const std::FILE* handle() const noexcept
		{
			return fh;
		}

		std::FILE* release() noexcept
		{
			std::FILE* temp_fh = fh;
			fh = nullptr;
			return temp_fh;
		}

		void reset(std::FILE* new_fh = nullptr) noexcept
		{
			if (fh && !m_unowned)
				std::fclose(fh);

			fh = new_fh;
			m_unowned = false;
		}

		void reset(std::FILE* new_fh, no_ownership_t) noexcept
		{
			if (fh && !m_unowned)
				std::fclose(fh);

			fh = new_fh;
			m_unowned = true;
		}

		void swap(stream& other) noexcept
		{
			std::swap(fh, other.fh);
			std::swap(m_unowned, other.m_unowned);
		}

		bool is_open() const noexcept
		{
			return fh != nullptr;
		}

		bool eof() const noexcept
		{
			return std::feof(fh);
		}

		bool error() const noexcept
		{
			return std::ferror(fh);
		}

		void clear() noexcept
		{
			std::clearerr(fh);
		}

		explicit operator bool() const noexcept
		{
			return is_open() && !error();
		}

		bool put(char c) noexcept
		{
			int result = std::fputc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool put(signed char c) noexcept
		{
			int result = std::fputc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool put(unsigned char c) noexcept
		{
			int result = std::fputc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool get(char& c) noexcept
		{
			int result = std::fgetc(fh);
			bool ok = (result != EOF);

			if (ok)
				c = static_cast<char>(result);

			return ok;
		}

		bool get(signed char& c) noexcept
		{
			int result = std::fgetc(fh);
			bool ok = (result != EOF);

			if (ok)
				c = static_cast<signed char>(result);

			return ok;
		}

		bool get(unsigned char& c) noexcept
		{
			int result = std::fgetc(fh);
			bool ok = (result != EOF);

			if (ok)
				c = static_cast<unsigned char>(result);

			return ok;
		}

		bool unget(char c) noexcept
		{
			int result = std::ungetc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool unget(signed char c) noexcept
		{
			int result = std::ungetc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool unget(unsigned char c) noexcept
		{
			int result = std::ungetc(static_cast<int>(c), fh);
			return (result != EOF);
		}

		bool write(const char* data) noexcept
		{
			int result = std::fputs(data, fh);
			return (result > 0);
		}

		std::size_t write(const char* data, std::size_t n) noexcept
		{
			return std::fwrite(data, 1, n, fh);
		}

		std::size_t read(char* data, std::size_t n) noexcept
		{
			return std::fread(data, 1, n, fh);
		}

		void rewind() noexcept
		{
			std::rewind(fh);
		}

		bool seek(long offset) noexcept
		{
			int result = std::fseek(fh, offset, SEEK_SET);
			return (result == 0);
		}

		bool skip(long offset) noexcept
		{
			int result = std::fseek(fh, offset, SEEK_CUR);
			return (result == 0);
		}

		bool seek_reverse(long offset) noexcept
		{
			int result = std::fseek(fh, offset, SEEK_END);
			return (result == 0);
		}

		long tell() noexcept
		{
			return std::ftell(fh);
		}

		void flush() noexcept
		{
			std::fflush(fh);
		}

		~stream() noexcept
		{
			if (!m_unowned && fh)
				close();
		}
};

struct io_flush_term_t { };
struct io_endl_term_t { };

constexpr io_flush_term_t flush;
constexpr io_endl_term_t endl;

inline stream& operator <<(stream& io, bool b) noexcept
{
	io.write(b ? "true" : "false");
	return io;
}

inline stream& operator <<(stream& io, void* p) noexcept
{
	std::fprintf(io.handle(), "%p", p);
	return io;
}

inline stream& operator <<(stream& io, char c) noexcept
{
	io.put(c);
	return io;
}

inline stream& operator <<(stream& io, signed char c) noexcept
{
	io.put(c);
	return io;
}

inline stream& operator <<(stream& io, unsigned char c) noexcept
{
	io.put(c);
	return io;
}

inline stream& operator <<(stream& io, int i) noexcept
{
	std::fprintf(io.handle(), "%i", i);
	return io;
}

inline stream& operator <<(stream& io, unsigned int i) noexcept
{
	std::fprintf(io.handle(), "%u", i);
	return io;
}

inline stream& operator <<(stream& io, short i) noexcept
{
	return (io << static_cast<int>(i));
}

inline stream& operator <<(stream& io, unsigned short i) noexcept
{
	return (io << static_cast<unsigned int>(i));
}

inline stream& operator <<(stream& io, long i) noexcept
{
	std::fprintf(io.handle(), "%li", i);
	return io;
}

inline stream& operator <<(stream& io, unsigned long i) noexcept
{
	std::fprintf(io.handle(), "%lu", i);
	return io;
}

inline stream& operator <<(stream& io, long long i) noexcept
{
	std::fprintf(io.handle(), "%lli", i);
	return io;
}

inline stream& operator <<(stream& io, unsigned long long i) noexcept
{
	std::fprintf(io.handle(), "%llu", i);
	return io;
}

inline stream& operator <<(stream& io, float f) noexcept
{
	std::fprintf(io.handle(), "%f", double(f));
	return io;
}

inline stream& operator <<(stream& io, double f) noexcept
{
	std::fprintf(io.handle(), "%f", f);
	return io;
}

inline stream& operator <<(stream& io, long double f) noexcept
{
	std::fprintf(io.handle(), "%Lf", f);
	return io;	
}

inline stream& operator <<(stream& io, const char* s) noexcept
{
	io.write(s);
	return io;
}

template <std::size_t Sz> stream& operator <<(stream& io, const char s[Sz]) noexcept
{
	io.write(s, Sz);
	return io;
}

inline stream& operator <<(stream& io, const std::string& s) noexcept
{
	io.write(s.data(), s.size());
	return io;
}

inline stream& operator <<(stream& io, const io_flush_term_t&) noexcept
{
	io.flush();
	return io;
}

inline stream& operator <<(stream& io, const io_endl_term_t&) noexcept
{
	io.put('\n');
	io.flush();
	return io;
}

inline stream& operator <<(stream& io, const std::string_view& sv) noexcept
{
	io.write(sv.data(), sv.size());
	return io;
}

inline void swap(stream& a, stream& b) noexcept
{
	a.swap(b);
}

extern stream in;
extern stream out;
extern stream err;

}

namespace std
{
	template <> struct hash<::cio::stream>
	{
		typedef ::cio::stream argument_type;
		typedef size_t result_type;
		
		size_t operator()(const ::cio::stream& io) const noexcept
		{
			return hash<const std::FILE*>()(io.handle());
		}
	};

	template <> struct hash<::cio::mode>
	{
		typedef ::cio::mode argument_type;
		typedef size_t result_type;
		
		size_t operator()(const ::cio::mode& mode) const noexcept
		{
			using UT = underlying_type<::cio::mode>::type;
			return hash<UT>()(UT(mode));
		}
	};
}

#endif // CIO_CIO_HPP
