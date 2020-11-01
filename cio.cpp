#include "cio.hpp"

namespace cio
{

namespace impl
{
	std::string flags_to_modestr(cio::mode flags) noexcept
	{
		std::string modestr;

		bool read = (flags & mode::read) == mode::read;
		bool write = (flags & mode::write) == mode::write;
		bool append = (flags & mode::append) == mode::append;
		bool text = (flags & mode::text) == mode::text;
		bool no_overwrite = (flags & mode::no_overwrite) == mode::no_overwrite;

		modestr += append ? 'a' : (write ? 'w' : 'r');

		if (read && (write || append))
			modestr += '+';

		if (!text)
			modestr += 'b';

		if (no_overwrite)
			modestr += 'x';

		return modestr;
	}
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif
stream in(stdin, no_ownership);
stream out(stdout, no_ownership);
stream err(stderr, no_ownership);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

}
