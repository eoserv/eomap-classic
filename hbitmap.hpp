#ifndef HBITMAP_HPP_INCLUDED
#define HBITMAP_HPP_INCLUDED

#include "common.hpp"

std::unique_ptr<a5::Bitmap> convert_hbitmap_to_bitmap(HBITMAP bitmap, a5::Pixel_Format format);

#endif // HBITMAP_HPP_INCLUDED
