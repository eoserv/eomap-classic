#ifndef CIO_PHYSIO_HPP
#define CIO_PHYSIO_HPP

#include "cio.hpp"
#include "physfs.h"

namespace cio
{

class physfs_stream
{
	private:
		PHYSFS_File* fh;
		bool m_unowned = false;
		PHYSFS_ErrorCode m_error = PHYSFS_ERR_OK;

	public:
		typedef PHYSFS_File* element_type;

		physfs_stream(PHYSFS_File* fh) noexcept
			: fh(fh)
		{ }

		physfs_stream(PHYSFS_File* fh, no_ownership_t) noexcept
			: fh(fh)
			, m_unowned(true)
		{ }

		explicit physfs_stream(const char* filename) noexcept
			: fh(nullptr)
		{
			open(filename);
		}

		physfs_stream(physfs_stream&& other) noexcept
			: fh(other.fh)
			, m_unowned(other.m_unowned)
			, m_error(other.m_error)
		{
			other.fh = nullptr;
			other.m_unowned = true;
		}

		physfs_stream(const physfs_stream&) = delete;
		physfs_stream& operator=(const physfs_stream&) = delete;

		bool open(const char* filename) noexcept
		{
			if (fh && !m_unowned)
				close();

			fh = PHYSFS_openRead(filename);
			m_unowned = false;

			return (fh != nullptr);
		}

		void close() noexcept
		{
			PHYSFS_close(fh);
			fh = nullptr;
			m_error = PHYSFS_ERR_OK;
		}

		PHYSFS_File* handle() noexcept
		{
			return fh;
		}

		const PHYSFS_File* handle() const noexcept
		{
			return fh;
		}

		PHYSFS_File* release() noexcept
		{
			PHYSFS_File* temp_fh = fh;
			fh = nullptr;
			return temp_fh;
		}

		void reset(PHYSFS_File* new_fh = nullptr) noexcept
		{
			if (fh && !m_unowned)
				close();

			fh = new_fh;
			m_unowned = false;
		}

		void reset(PHYSFS_File* new_fh, no_ownership_t) noexcept
		{
			if (fh && !m_unowned)
				close();

			fh = new_fh;
			m_unowned = true;
		}

		void swap(physfs_stream& other) noexcept
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
			return PHYSFS_eof(fh);
		}

		bool error() const noexcept
		{
			return eof() || m_error != PHYSFS_ERR_OK;
		}

		void clear() noexcept
		{
			m_error = PHYSFS_ERR_OK;
		}

		explicit operator bool() const noexcept
		{
			return is_open() && !error();
		}

		bool get(char& c) noexcept
		{
			std::size_t bytes_read = read(&c, 1);
			return (bytes_read == 1);
		}

		bool get(signed char& c) noexcept
		{
			std::size_t bytes_read = read(reinterpret_cast<char*>(&c), 1);
			return (bytes_read == 1);
		}

		bool get(unsigned char& c) noexcept
		{
			std::size_t bytes_read = read(reinterpret_cast<char*>(&c), 1);
			return (bytes_read == 1);
		}

		std::size_t read(char* data, std::size_t n) noexcept
		{
			PHYSFS_sint64 result = PHYSFS_readBytes(fh, data, n);

			if (result == -1 || result == 0)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			return static_cast<std::size_t>(result);
		}

		void rewind() noexcept
		{
			seek(0);
		}

		bool seek(long offset) noexcept
		{
			if (offset < 0)
			{
				m_error = PHYSFS_ERR_PAST_EOF;
				return false;
			}

			int result = PHYSFS_seek(fh, static_cast<PHYSFS_uint64>(offset));

			if (result != 0)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			return true;
		}

		bool skip(long offset) noexcept
		{
			PHYSFS_sint64 pos = PHYSFS_tell(fh);

			if (pos == -1)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			pos += static_cast<PHYSFS_sint64>(offset);

			if (pos < 0)
			{
				m_error = PHYSFS_ERR_PAST_EOF;
				return false;
			}

			int result = PHYSFS_seek(fh, static_cast<PHYSFS_uint64>(pos));

			if (result != 0)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			return true;
		}

		bool seek_reverse(long offset) noexcept
		{
			PHYSFS_sint64 file_length = PHYSFS_fileLength(fh);

			if (file_length == -1)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			PHYSFS_sint64 pos = file_length - static_cast<PHYSFS_sint64>(offset);

			if (pos < 0)
			{
				m_error = PHYSFS_ERR_PAST_EOF;
				return false;
			}

			int result = PHYSFS_seek(fh, static_cast<PHYSFS_uint64>(pos));

			if (result != 0)
			{
				m_error = PHYSFS_getLastErrorCode();
				return false;
			}

			return true;
		}

		long tell() noexcept
		{
			PHYSFS_sint64 result = PHYSFS_tell(fh);

			if (result == -1)
				m_error = PHYSFS_getLastErrorCode();

			return static_cast<long>(result);
		}

		void flush() noexcept
		{
			PHYSFS_flush(fh);
		}

		PHYSFS_ErrorCode errcode() const noexcept
		{
			return m_error;
		}

		const char* errstr() const noexcept
		{
			return PHYSFS_getErrorByCode(m_error);
		}

		~physfs_stream() noexcept
		{
			if (!m_unowned && fh)
				close();
		}
};

inline void swap(physfs_stream& a, physfs_stream& b) noexcept
{
	a.swap(b);
}

}

namespace std
{
	template <> struct hash<::cio::physfs_stream>
	{
		typedef ::cio::physfs_stream argument_type;
		typedef size_t result_type;

		size_t operator()(const ::cio::physfs_stream& io) const noexcept
		{
			return hash<const PHYSFS_File*>()(io.handle());
		}
	};
}

#endif // CIO_PHYSFS_HPP
