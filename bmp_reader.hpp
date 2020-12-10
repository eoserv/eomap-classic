#ifndef EOMAP_BMP_READER_HPP
#define EOMAP_BMP_READER_HPP

#include "common.hpp"

a5::Bitmap load_bmp(const char* filename, std::uint32_t mask_color = 0xFF00FF);

#endif // EOMAP_BMP_READER_HPP
