#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <algorithm>
#include <list>
#include <optional>
#include <string>
#include <map>
#include <vector>

#include <exception>
#include <memory>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#define A5SES_INT_COORD
#define A5SES_IMAGE_ADDON
#define A5SES_FONT_ADDON
#include <a5ses/a5ses.hpp>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

#ifdef WIN32
#include <allegro5/allegro_windows.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef LoadBitmap
#undef LoadBitmap
#endif // LoadBitmap

#ifdef RGB
#undef RGB
#endif // RGB

#ifdef None
#undef None
#endif // None
#endif // WIN32

#include "util.hpp"

class EOMap_Exception : std::exception
{
	private:
		const char *message_;

	public:
		EOMap_Exception(const char *message__) : message_(message__) {}

		virtual const char *what()
		{
			return "a5::EOMap_Exception";
		}

		const char *message()
		{
			return this->message_;
		}

		~EOMap_Exception() throw()
		{
			delete[] this->message_;
		}
};

#define EOMAP_ERROR(format, ...) do { char *_err_buf = new char[1024]; snprintf(_err_buf, 1024, format, __VA_ARGS__); _err_buf[1023] = '\0'; throw EOMap_Exception(_err_buf); } while(0)

#endif // COMMON_HPP_INCLUDED
