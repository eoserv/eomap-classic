
/* crc32.h
 * Copyright 2009 the EOSERV development team (http://eoserv.net/devs)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef CRC32_H_INCLUDED
#define CRC32_H_INCLUDED

#include <limits.h>

#if UINT_MAX >= 0xFFFFFFFF
#define u32 unsigned int
#define u32_max 0xFFFFFFFFU
#else
#define u32 unsigned long
#define u32_max 0xFFFFFFFFLU
#endif
#define u16 unsigned short
#define u8 unsigned char

#define u16_max 0xFFFFU
#define u8_max 0xFFU

u32 crc32(u32 hash, const u8 *input, unsigned long length);

#endif /* CRC32_H_INCLUDED */
