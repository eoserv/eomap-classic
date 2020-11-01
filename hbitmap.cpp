
#include "hbitmap.hpp"

std::unique_ptr<a5::Bitmap> convert_hbitmap_to_bitmap(HBITMAP bitmap, a5::Pixel_Format format)
{
	BITMAP bm;

	if (!GetObject(bitmap, sizeof(bm), reinterpret_cast<LPSTR>(&bm)))
	{
		return 0;
	}

	std::unique_ptr<a5::Bitmap> bmp(new a5::Bitmap(bm.bmWidth, bm.bmHeight));

	std::unique_ptr<a5::Bitmap_Lock> bmp_lock(bmp->Lock(format));
	BYTE *data = reinterpret_cast<BYTE *>(bmp_lock->Data());

	if (!bitmap)
	{
		return 0;
	}

	if (!GetObject(bitmap, sizeof(bm), reinterpret_cast<LPSTR>(&bm)))
	{
		return 0;
	}

	int pitch = bm.bmWidth * bmp_lock->Format().ByteSize();
	pitch = (pitch + 3) & ~3;

	std::unique_ptr<BYTE[]> pixels(new BYTE[bm.bmHeight * pitch]);

	BITMAPINFOHEADER bi;
	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biBitCount = bmp_lock->Format().BitSize();
	bi.biPlanes = 1;
	bi.biWidth = bm.bmWidth;
	bi.biHeight = -abs(bm.bmHeight);
	bi.biClrUsed = 256;
	bi.biCompression = BI_RGB;

	BITMAPINFO *binfo = static_cast<BITMAPINFO *>(std::malloc(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256));
	binfo->bmiHeader = bi;

	HDC hdc = GetDC(NULL);

	GetDIBits(hdc, bitmap, 0, bm.bmHeight, pixels.get(), binfo, DIB_RGB_COLORS);

	std::free(binfo);

	ReleaseDC(NULL, hdc);

	int height = std::abs(std::min(int(bm.bmHeight), int(bmp->Height())));
	int width = std::abs(std::min(pitch, int(bmp_lock->Pitch())));

	for (int y = 0; y < height; ++y)
	{
		std::memcpy(data + bmp_lock->Pitch() * y, &pixels[pitch * y], width);
	}

	return bmp;
}
